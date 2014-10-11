#include "common.h"

int ReadParams(struct RadarParams *radar_params, struct ImageParams *image_params, struct RegistrationParams *registration_params, struct CfarParams *cfar_params)
{
	char param[80];
	FILE *pFile;
    int n;

	pFile = fopen("Parameters.txt", "r");

	if(pFile == NULL) {
		return 1;
	}

	n = fscanf(pFile, "%s %f", param, &radar_params->fs);
	if(n != 2 || strcmp(param, "Fs") != 0) {
#ifdef RAG
		fprintf(stderr,"Fs\n");
#endif
		return 2;
	}
	n = fscanf(pFile, "%s %f", param, &radar_params->fc);
	if(n != 2 || strcmp(param, "Fc") != 0) {
#ifdef RAG
		fprintf(stderr,"Fc\n");
#endif
		return 2;
	}
	n = fscanf(pFile, "%s %f", param, &radar_params->PRF);
	if(n != 2 || strcmp(param, "PRF") != 0) {
#ifdef RAG
		fprintf(stderr,"PRF\n");
#endif
		return 2;
	}
	n = fscanf(pFile, "%s %d", param, &image_params->F);
	if(n != 2 || strcmp(param, "F") != 0) {
#ifdef RAG
		fprintf(stderr,"F\n");
#endif
		return 2;
	}
	n = fscanf(pFile, "%s %d", param, &image_params->Ix);
	if(n != 2 || strcmp(param, "Ix") != 0) {
#ifdef RAG
		fprintf(stderr,"Ix\n");
#endif
		return 2;
	}
	n = fscanf(pFile, "%s %d", param, &image_params->Iy);
	if(n != 2 || strcmp(param, "Iy") != 0) {
#ifdef RAG
		fprintf(stderr,"Iy\n");
#endif
		return 2;
	}
	n = fscanf(pFile, "%s %d", param, &image_params->Sx);
	if(n != 2 || strcmp(param, "Sx") != 0) {
#ifdef RAG
		fprintf(stderr,"Sx\n");
#endif
		return 2;
	}
	n = fscanf(pFile, "%s %d", param, &image_params->Sy);
	if(n != 2 || strcmp(param, "Sy") != 0) {
#ifdef RAG
		fprintf(stderr,"Sy\n");
#endif
		return 2;
	}
	n = fscanf(pFile, "%s %d", param, &image_params->Fbp);
	if(n != 2 || strcmp(param, "Fbp") != 0) {
#ifdef RAG
		fprintf(stderr,"Fbp\n");
#endif
		return 2;
	}
	n = fscanf(pFile, "%s %d", param, &image_params->P1);
	if(n != 2 || strcmp(param, "P1") != 0) {
#ifdef RAG
		fprintf(stderr,"P1 %d (n=%d)\n",image_params->P1,n);
#endif
		return 2;
	}
	n = fscanf(pFile, "%s %d", param, &image_params->S1);
	if(n != 2 || strcmp(param, "S1") != 0) {
#ifdef RAG
		fprintf(stderr,"S1\n");
#endif
		return 2;
	}
	n = fscanf(pFile, "%s %f", param, &radar_params->r0);
	if(n != 2 || strcmp(param, "r0") != 0) {
#ifdef RAG
		fprintf(stderr,"r0\n");
#endif
		return 2;
	}
	n = fscanf(pFile, "%s %f", param, &radar_params->R0);
	if(n != 2 || strcmp(param, "R0") != 0) {
#ifdef RAG
		fprintf(stderr,"R0\n");
#endif
		return 2;
	}
	n = fscanf(pFile, "%s %d", param, &registration_params->Nc);
	if(n != 2 || strcmp(param, "Nc") != 0) {
#ifdef RAG
		fprintf(stderr,"Nc\n");
#endif
		return 2;
	}
	n = fscanf(pFile, "%s %d", param, &registration_params->Rc);
	if(n != 2 || strcmp(param, "Rc") != 0) {
#ifdef RAG
		fprintf(stderr,"Rc\n");
#endif
		return 2;
	}
	n = fscanf(pFile, "%s %d", param, &registration_params->Sc);
	if(n != 2 || strcmp(param, "Sc") != 0) {
#ifdef RAG
		fprintf(stderr,"Sc\n");
#endif
		return 2;
	}
	n = fscanf(pFile, "%s %f", param, &registration_params->Tc);
	if(n != 2 || strcmp(param, "Tc") != 0) {
#ifdef RAG
		fprintf(stderr,"Tc\n");
#endif
		return 2;
	}
	n = fscanf(pFile, "%s %d", param, &image_params->Ncor);
	if(n != 2 || strcmp(param, "Ncor") != 0) {
#ifdef RAG
		fprintf(stderr,"Ncor\n");
#endif
		return 2;
	}
	n = fscanf(pFile, "%s %d", param, &cfar_params->Ncfar);
	if(n != 2 || strcmp(param, "Ncfar") != 0) {
#ifdef RAG
		fprintf(stderr,"Ncfar\n");
#endif
		return 2;
	}
	n = fscanf(pFile, "%s %f", param, &cfar_params->Tcfar);
	if(n != 2 || strcmp(param, "Tcfar") != 0) {
#ifdef RAG
		fprintf(stderr,"Tcfar\n");
#endif
		return 2;
	}
	n = fscanf(pFile, "%s %d", param, &cfar_params->Nguard);
	if(n != 2 || strcmp(param, "Nguard") != 0) {
#ifdef RAG
		fprintf(stderr,"Nguard\n");
#endif
		return 2;
	}
	n = fscanf(pFile, "%s %d", param, &image_params->numImages);
	if(n != 2 || strcmp(param, "NumberImages") != 0) {
#ifdef RAG
		fprintf(stderr,"NumberImages\n");
#endif
		return 2;
	}

	fclose(pFile);

	return 0;
}

void ReadData(FILE *pFile1, FILE *pFile2, FILE *pFile3, struct Inputs *in, struct ImageParams *image_params)
{
	int m;
    size_t n;

	for(m=0; m<image_params->P1; m++) {
		n = fread(&in->X[m][0], sizeof(struct complexData), image_params->S1, pFile1);	// Read complex SAR data
        if (n != image_params->S1)
        {
            fprintf(stderr, "Error - incorrect number of entries read from data file (%lu instead of %d)\n",
                n, image_params->S1);
            exit(-1);
        }
		n = fread(&in->Pt[m][0], sizeof(float), 3, pFile2);	// Read platform positions
        if (n != 3)
        {
            fprintf(stderr, "Error - incorrect number of entries read from platform position file (%lu instead of %d)\n",
                n, 3);
            exit(-1);
        }
	}

	n = fread(in->Tp, sizeof(float), image_params->P1, pFile3);	// Read pulse transmission timestamps
    if (n != image_params->P1)
    {
        fprintf(stderr, "Error - incorrect number of entries read from pulse transmission timestamps file (%lu instead of %d)\n",
            n, image_params->P1);
        exit(-1);
    }
}
