#include "common.h"

int ReadParams()
{
	char param[80];
	FILE *pFile;

	pFile = fopen("Parameters.txt", "r");

	if(pFile == NULL) {
		return 1;
	}

	fscanf(pFile, "%s %f", param, &fs);
	if(strcmp(param, "Fs") != 0) {
		return 2;
	}
	fscanf(pFile, "%s %f", param, &fc);
	if(strcmp(param, "Fc") != 0) {
		return 2;
	}
	fscanf(pFile, "%s %f", param, &prf);
	if(strcmp(param, "PRF") != 0) {
		return 2;
	}
	fscanf(pFile, "%s %f", param, &r0);
	if(strcmp(param, "SlantRange") != 0) {
		return 2;
	}
	fscanf(pFile, "%s %d", param, &numImages);
	if(strcmp(param, "NumberImages") != 0) {
		return 2;
	}
	fscanf(pFile, "%s %f", param, &cr);
	if(strcmp(param, "CrossRangeRes") != 0) {
		return 2;
	}
	fscanf(pFile, "%s %f", param, &theta_el);
	if(strcmp(param, "ElevationAngle") != 0) {
		return 2;
	}
	fscanf(pFile, "%s %f", param, &edgeSize);
	if(strcmp(param, "GroundEdgeSize") != 0) {
		return 2;
	}
	fscanf(pFile, "%s %f", param, &v);
	if(strcmp(param, "PlatformVelocity") != 0) {
		return 2;
	}
	fscanf(pFile, "%s %d", param, &rngBinDev);
	if(strcmp(param, "RangeBinDeviation") != 0) {
		return 2;
	}

	fclose(pFile);

	return 0;
}

int ReadTargets()
{
	int i = 0;
	char chr;
	FILE *pFile;

	pFile = fopen("Targets.txt", "r");

	if(pFile == NULL) {
		return 1;
	}

	numTargets = 0;
	while( (chr = fgetc(pFile)) != EOF) {
		if(chr == '\n') {
			numTargets++;
		}
	}

	rewind(pFile);

	target_x = (float*)malloc(numTargets*sizeof(float));
	target_y = (float*)malloc(numTargets*sizeof(float));
	target_z = (float*)malloc(numTargets*sizeof(float));
	target_first = (int*)malloc(numTargets*sizeof(int));
	target_last = (int*)malloc(numTargets*sizeof(int));
	target_b = (float*)malloc(numTargets*sizeof(float));

	for(i=0; i<numTargets; i++) {
		fscanf(pFile, "%f %f %f %d %d %f", &target_x[i], &target_y[i], &target_z[i], &target_first[i], &target_last[i], &target_b[i]);
	}

	fclose(pFile);

	return 0;
}
