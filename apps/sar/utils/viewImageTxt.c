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

#define MAX_Nd (10000)
	int Nd;

FILE    *images_debug;
typedef uint32_t Pixel_t;
typedef float complex FreqDomain_t;
	int wantImage = 0;

	if (argc >= 2)
		wantImage = atoi(argv[1]);

	if (argc != 2) {
		fprintf(stderr,"Usage: ./viewImage <image #>\nwhere 1 >= cur, 2=> ref, 3 => all-in-one\nBMP output written to stdout\n");fflush(stderr);
		exit(0);
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
	image_params.P2 = image_params.P1/image_params.TF;
	image_params.S2 = image_params.S1/image_params.TF;

fprintf(stderr,"image <Iy,Ix> == <%3d,%3d> dr = %f\n",image_params.Iy,image_params.Ix,image_params.dr);fflush(stderr);

#if 0
	if ( (images_debug = fopen("./images_debug.bin","rb")) == NULL ) {
		fprintf(stderr,"Error opening ./images_debug.bin\n");
		exit(1);
	}
#else
	if ( (images_debug = fopen("./images_debug.txt","r")) == NULL ) {
		fprintf(stderr,"Error opening ./images_debug.txt\n");
		exit(1);
	}
#endif

	// TF > 1 implies digital spotlighting, TF = 1 implies no digital spotlighting

	Pixel_t **PixelArray;
	FreqDomain_t **FreqDomain;

fprintf(stderr,"malloc PixalArray\n");fflush(stderr);
	PixelArray = (Pixel_t **)malloc(image_params.Iy*sizeof(Pixel_t*));
	if(!PixelArray) {perror("malloc of PixelArray[]");exit(1);}
	for(int iy=0;iy<image_params.Iy;iy++) {
		PixelArray[iy] = (Pixel_t *)calloc(image_params.Ix,sizeof(Pixel_t));
		if(!PixelArray[iy]) {perror("calloc of PixelArray[][]");exit(1);}
	}

fprintf(stderr,"malloc FreqDomain\n");fflush(stderr);
	FreqDomain = (FreqDomain_t **)malloc(image_params.Iy*sizeof(FreqDomain_t *));
	if(!FreqDomain) {perror("malloc of FreqDomain[]");exit(1);}
	for(int iy=0;iy<image_params.Iy;iy++) {
		FreqDomain[iy] = (FreqDomain_t *)malloc(image_params.Ix*sizeof(FreqDomain_t));
		if(!FreqDomain[iy]) {perror("malloc of FreqDomain[][]");exit(1);}
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

	Y = malloc(MAX_Nd*sizeof(Y[0]));
	if(!Y) {perror("malloc of Y[]");exit(1);}

fprintf(stderr,"read Y\n");fflush(stderr);
	Nd = 0;
	while( !feof(stdin)  && (Nd < MAX_Nd) ) {
		fscanf(stdin,"x=   %f m,  y=   %f m, p=%f\n",&Y[Nd].x,&Y[Nd].y,&Y[Nd].p);
//fprintf(stderr,"<%f,%f> %f\n",Y[Nd].x,Y[Nd].x,Y[Nd].p);fflush(stderr);
		Nd++;
	}
fprintf(stderr,"Nd = %3d\n",Nd);fflush(stderr);

fprintf(stderr,"mark Y\n");fflush(stderr);
	for(int i=0;i<Nd;i++) {
		int iy = (int)floorf(Y[i].y/image_params.dr+(float)image_params.Iy/2.0f);
		int ix = (int)floorf(Y[i].x/image_params.dr+(float)image_params.Ix/2.0f);
//fprintf(stderr,"<iy=%3d,ix=%3d> p=%f>\n",iy,ix,Y[i].p);fflush(stderr);
		if (wantImage == 3) {
			int ip = (int)fminf(255.0f,fmaxf(0.0f,(Y[i].p*512.0f)));
			PixelArray[iy][ix] |= RGB(    0,    0,  ip);
		}
	}

    for(int image=1;image<=image_params.numImages;image++) {
fprintf(stderr,"process image number %d\n",image);fflush(stderr);

fprintf(stderr,"read FreqDomain\n");fflush(stderr);
	for(int iy=0;iy<image_params.Iy;iy++) {
#if 0
		fread(&FreqDomain[iy][0],sizeof(FreqDomain_t),image_params.Ix,images_debug);
#else
		for(int ix=0;ix<image_params.Ix;ix++) {
			uint32_t xe_num;
			float t[2];
			if(image == 1) {
				uint32_t junk1, junk2;
				if( (3+2) != fscanf(images_debug,"CONSOLE: XE%d >>> @ (%d; %d) cur %x %x\n",
					&junk1, &junk2,
					&xe_num,(uint32_t *)&t[0],(uint32_t *)&t[1])) {
					perror("fscanf cur");
					exit(1);
				}
		    	} else if(image == 2) {
				uint32_t junk1, junk2;
				if( (3+2) != fscanf(images_debug,"CONSOLE: XE%d >>> @ (%d; %d) ref %x %x\n",
					&junk1, &junk2,
					&xe_num,(uint32_t *)&t[0],(uint32_t *)&t[1])) {
					perror("fscanf ref");
					exit(1);
				}
			}
			FreqDomain[iy][ix] = t[0] + t[1] * I;
		}
#endif
	}

fprintf(stderr,"detect FreqDomain\n");fflush(stderr);
	for(int iy=0;iy<image_params.Iy;iy++) {
		for(int ix=0;ix<image_params.Ix;ix++) {
			const float re = crealf(FreqDomain[iy][ix]);
			const float im = cimagf(FreqDomain[iy][ix]);
			const float mag = sqrtf(re*re+im*im);
			FreqDomain[iy][ix] = mag + I*0.0f;
		}
	}

	if( !(image & wantImage))continue;

fprintf(stderr,"max FreqDomain\n");fflush(stderr);
	float max_mag =       0.0f;
	float min_mag = 1000000.0f;
	for(int iy=0;iy<image_params.Iy;iy++) {
		for(int ix=0;ix<image_params.Ix;ix++) {
			max_mag = fmaxf(max_mag,crealf(FreqDomain[iy][ix]));
			min_mag = fminf(min_mag,crealf(FreqDomain[iy][ix]));
		}
	}
fprintf(stderr,"max is %f\nmin is %f\n",max_mag,min_mag);fflush(stderr);

	uint64_t hist[256];
	for(int i=0;i<256;i++) hist[i] = 0;

fprintf(stderr,"scale FreqDomain\n");fflush(stderr);
	for(int iy=0;iy<image_params.Iy;iy++) {
		for(int ix=0;ix<image_params.Ix;ix++) {
			const float re = crealf(FreqDomain[iy][ix]);
			const uint8_t umag = fminf(5.0f*(re/max_mag)*255.0f,255.0f);
			if( (image & wantImage) == 1 )
				PixelArray[iy][ix] |= RGB(    0, umag,    0);
			else if( (image & wantImage) == 2)
				PixelArray[iy][ix] |= RGB( umag,    0,    0);
			hist[umag]++;
		}
	}

//for(int i=0;i<256;i++)fprintf(stderr,"hist[%3d] = %8lld\n",i,hist[i]);fflush(stderr);
	if( image == wantImage)break;

    } // for images

fprintf(stderr,"write bmp file\n");fflush(stderr);
	fwrite(&bmp_magic,       sizeof(bmp_magic), 1, stdout);
	fwrite(&bmp_header,      sizeof(bmp_header), 1, stdout);
	fwrite(&bmp_info_header, sizeof(bmp_info_header), 1, stdout);
	for(int iy=0;iy<image_params.Iy;iy++) {
		fwrite(&PixelArray[iy][0], sizeof(Pixel_t), image_params.Ix, stdout);
	}

fprintf(stderr,"free FreqDomain\n");fflush(stderr);
	for(int iy=0;iy<image_params.Iy;iy++) {
		free(FreqDomain[iy]);
	}
	free(FreqDomain);

fprintf(stderr,"free PixelArray\n");fflush(stderr);
	for(int iy=0;iy<image_params.Iy;iy++) {
		free(PixelArray[iy]);
	}
	free(PixelArray);

fprintf(stderr,"done\n");fflush(stderr);
	exit(0);
}
