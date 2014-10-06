#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <math.h>
#include <complex.h>

#include "ocr.h"
#include "common.h"

struct Inputs in;					// Inputs
struct detects *Y;					// Detects list
struct point **corr_map;			// Correlation map
struct DigSpotVars dig_spot;		// Digital spotlight variables
struct complexData **curImage;		// Current image
struct complexData **refImage;		// Reference (previous) image
struct CfarParams cfar_params;		// CFAR parameters
struct RadarParams radar_params;	// Radar parameters
struct ImageParams image_params;	// Image parameters
struct AffineParams affine_params;	// Affine registration parameters

#define RGB(r,g,b) ( ( (r) << 16 ) | ( (g) << 8 ) | ( (b) << 0 ) )

int main(int argc, char **argv) {
	struct bmpfile_magic {
		unsigned char magic[2];
	} bmp_magic;

	struct bmpfile_header {
		uint32_t filesz;
		uint16_t creator1;
		uint16_t creator2;
		uint32_t bmp_offset;
	} bmp_header;

	struct bmpfile_info_header {
		uint32_t header_sz;
		int32_t  width;
		int32_t  height;
		uint16_t nplanes;
		uint16_t bitspp;
		uint32_t compress_type;
		uint32_t bmp_bytesz;
		int32_t  hres;
		uint32_t vres;
		uint32_t ncolors;
		uint32_t nimpcolors;
	} bmp_info_header;

FILE    *corr_debug;
typedef uint32_t Pixel_t;

	if (argc == 2) {
		if(strcmp(argv[1],"-h") == 0) {
			fprintf(stderr,"Usage: ./viewCorr \nBMP output written to stdout\n");fflush(stderr);
			exit(0);
		} else {
			fprintf(stderr,"Unsupported swith try: ./viewCorr -h\n");fflush(stderr);
			exit(0);
		}
	}

	switch(ReadParams(&radar_params, &image_params, &affine_params, &cfar_params)) {
	  case 1:
		  printf("Unable to open Parameters.txt\n");
		  exit(1);
	  case 2:
		  printf("Parameters.txt does not adhere to expected format.\n");
		  exit(1);
	}

	// Ensure all window sizes are odd
	if( !(affine_params.Sc % 2) ) {
		printf("Sc must be odd. Exiting.\n");
		exit(1);
	}

	if( !(image_params.Ncor % 2) ) {
		printf("Ncor must be odd. Exiting.\n");
		exit(1);
	}

	if( !(cfar_params.Ncfar % 2) ) {
		printf("Ncfar must be odd. Exiting.\n");
		exit(1);
	}

	if( !(cfar_params.Nguard % 2) ) {
		printf("Nguard must be odd. Exiting.\n");
		exit(1);
	}

	// Calculate dependent variables
	image_params.TF = image_params.Ix/image_params.Sx;
	image_params.dr = c_mks_mps/radar_params.fs/2/image_params.F;
	int Cy = image_params.Iy-image_params.Ncor+1;
	int Cx = image_params.Ix-image_params.Ncor+1;

fprintf(stderr,"image <Iy,Ix> <%3d,%3d> <Cy,Cx> <%3d,%3d>\n",image_params.Iy,image_params.Ix,Cy,Cx);fflush(stderr);

#if 0
	if ( (corr_debug = fopen("./corr_debug.bin","rb")) == NULL ) {
		fprintf(stderr,"Error opening ./corr_debug.bin\n");
		exit(1);
	}
#else
	if ( (corr_debug = fopen("./corr_debug.txt","r")) == NULL ) {
		fprintf(stderr,"Error opening ./corr_debug.txt\n");
		exit(1);
	}
#endif

	Pixel_t **PixelArray;

fprintf(stderr,"malloc PixalArray\n");fflush(stderr);
	PixelArray = (Pixel_t **)malloc(image_params.Iy*sizeof(Pixel_t*));
	if(!PixelArray) {perror("malloc of PixelArray[]");exit(1);}
	for(int iy=0;iy<image_params.Iy;iy++) {
		PixelArray[iy] = (Pixel_t *)calloc(image_params.Ix,sizeof(Pixel_t));
		if(!PixelArray[iy]) {perror("calloc of PixelArray[][]");exit(1);}
	}

fprintf(stderr,"malloc corr_map\n");fflush(stderr);
	corr_map = (struct point **)malloc(Cy*sizeof(struct point *));
	if(!corr_map) {perror("malloc of corr_map[]");exit(1);}
	for(int iy=0;iy<Cy;iy++) {
		corr_map[iy] = (struct point *)malloc(Cx*sizeof(struct point));
		if(!corr_map[iy]) {perror("malloc of corr_map[][]");exit(1);}
	}

fprintf(stderr,"init bmp header\n");fflush(stderr);
	bmp_magic.magic[0] = 'B';
	bmp_magic.magic[1] = 'M';

	bmp_header.filesz     = sizeof(bmp_magic)
                              + sizeof(bmp_header)
                              + sizeof(bmp_info_header)
                              + image_params.Iy*image_params.Ix*4;
	bmp_header.creator1   = -1;
	bmp_header.creator2   = -1;
	bmp_header.bmp_offset = sizeof(bmp_magic)
                              + sizeof(bmp_header)
                              + sizeof(bmp_info_header);

	bmp_info_header.header_sz     = sizeof(bmp_info_header);
	bmp_info_header.width         = image_params.Ix;
	bmp_info_header.height        = image_params.Iy;
	bmp_info_header.nplanes       = 1;
	bmp_info_header.bitspp        = 32;
	bmp_info_header.compress_type = 0;
	bmp_info_header.bmp_bytesz    = 0;
	bmp_info_header.hres          = 1;
	bmp_info_header.vres          = 1;
	bmp_info_header.ncolors       = 0;
	bmp_info_header.nimpcolors    = 0;

fprintf(stderr,"read corr_map\n");fflush(stderr);
	for(int iy=0;iy<Cy;iy++) {
#if 0
		fread(&corr_map[iy][0],sizeof(struct point),Cx,corr_debug);
#else
		for(int ix=0;ix<Cx;ix++) {
			uint32_t xe_num;
			uint32_t junk1, junk2;
			if( (4+2) != fscanf(corr_debug,"CONSOLE: XE%d >>> @ (%d; %d) corr %x %x %x\n",
				&xe_num,
				(uint32_t *)&junk1,
				(uint32_t *)&junk2,
				(uint32_t *)&corr_map[iy][ix].x,
				(uint32_t *)&corr_map[iy][ix].y,
				(uint32_t *)&corr_map[iy][ix].p)) {
				perror("fscanf");
				exit(1);
			}
		}
#endif
	}

fprintf(stderr,"scale corr_map\n");fflush(stderr);
	for(int iy=0;iy<Cy;iy++) {
		for(int ix=0;ix<Cx;ix++) {
			const int iiy = corr_map[iy][ix].y;
			const int iix = corr_map[iy][ix].x;
			const uint8_t umag = fminf(255.0f,fmaxf(0.0f,corr_map[iy][ix].p*255.0f));
			PixelArray[iiy][iix] |= RGB( umag, umag, umag);
		}
	}

fprintf(stderr,"write bmp file\n");fflush(stderr);
	fwrite(&bmp_magic,       sizeof(bmp_magic), 1, stdout);
	fwrite(&bmp_header,      sizeof(bmp_header), 1, stdout);
	fwrite(&bmp_info_header, sizeof(bmp_info_header), 1, stdout);
	for(int iy=0;iy<image_params.Iy;iy++) {
		fwrite(&PixelArray[iy][0], sizeof(Pixel_t), image_params.Ix, stdout);
	}

fprintf(stderr,"free corr_map\n");fflush(stderr);
	for(int iy=0;iy<Cy;iy++) {
		free(corr_map[iy]);
	}
	free(corr_map);

fprintf(stderr,"free PixelArray\n");fflush(stderr);
	for(int iy=0;iy<image_params.Iy;iy++) {
		free(PixelArray[iy]);
	}
	free(PixelArray);

fprintf(stderr,"done\n");fflush(stderr);
	exit(0);
}
