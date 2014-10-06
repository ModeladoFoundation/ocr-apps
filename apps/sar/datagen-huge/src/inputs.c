#include <assert.h>
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
		fprintf(stderr,"Fs\n");
		return 2;
	}
	fscanf(pFile, "%s %f", param, &fc);
	if(strcmp(param, "Fc") != 0) {
		fprintf(stderr,"Fc\n");
		return 2;
	}
	fscanf(pFile, "%s %f", param, &prf);
	if(strcmp(param, "PRF") != 0) {
		fprintf(stderr,"PRF\n");
		return 2;
	}
	fscanf(pFile, "%s %f", param, &r0);
	if(strcmp(param, "SlantRange") != 0) {
		fprintf(stderr,"SlantRange\n");
		return 2;
	}
	fscanf(pFile, "%s %d", param, &numImages);
	if(strcmp(param, "NumberImages") != 0) {
		fprintf(stderr,"NumberImages\n");
		return 2;
	}
	fscanf(pFile, "%s %f", param, &cr);
	if(strcmp(param, "CrossRangeRes") != 0) {
		fprintf(stderr,"CrossRangeRes\n");
		return 2;
	}
	fscanf(pFile, "%s %f", param, &theta_el);
	if(strcmp(param, "ElevationAngle") != 0) {
		fprintf(stderr,"ElevationAngle\n");
		return 2;
	}
	fscanf(pFile, "%s %f", param, &edgeSize);
	if(strcmp(param, "GroundEdgeSize") != 0) {
		fprintf(stderr,"GroundEdgeSize\n");
		return 2;
	}
	fscanf(pFile, "%s %f", param, &v);
	if(strcmp(param, "PlatformVelocity") != 0) {
		fprintf(stderr,"PlatformVelocity\n");
		return 2;
	}
	fscanf(pFile, "%s %d", param, &rngBinDev);
	if(strcmp(param, "RangeBinDeviation") != 0) {
		fprintf(stderr,"RangeBinDeviation\n");
		return 2;
	}
	fscanf(pFile, "%s %lf", param, &backgroundReflSpacing);
	if(strcmp(param, "BackgroundReflectorSpacingMeters") != 0) {
		fprintf(stderr,"BackgroundReflectionSpacingMeters\n");
		return 2;
	}

	fclose(pFile);

	return 0;
}

int ReadTargetBoundingBoxes(
    const char *targets_filename,
    TargetBoundingBox **target_bboxes)
{
    FILE *fp = fopen(targets_filename, "rb");
    char *line = NULL;
    size_t length = 0;
    TargetBoundingBox bbox;
    int num_targets = 0, i, n;

    assert(target_bboxes != NULL);

    if (fp == NULL)
    {
        fprintf(stderr, "Error: Unable to open targets file %s.", targets_filename);
        exit(-1);
    }

    while (getline(&line, &length, fp) > 0)
    {
        if (strlen(line) == 1 && line[0] == '\n') { continue; }
        n = sscanf(line, "[%lf, %lf, %lf, %lf, %lf, %lf] %d %d\n",
            &bbox.tlhc_x, &bbox.tlhc_y, &bbox.width, &bbox.height, &bbox.spacing,
            &bbox.reflectivity, &bbox.firstImage, &bbox.lastImage);
        if (n != 8)
        {
            fprintf(stderr, "Error: unexpected target format on following line:\n");
            fprintf(stderr, "\t%s\n", line);
            exit(-1);
        }
        ++num_targets;
    }

    rewind(fp);

    *target_bboxes = (TargetBoundingBox *) malloc(sizeof(TargetBoundingBox) * num_targets);
    if (*target_bboxes == NULL)
    {
        printf("Error allocating memory for target bounding boxes.\n");
        exit(-1);
    }

    i = 0;
    while (getline(&line, &length, fp) > 0)
    {
        if (strlen(line) == 1 && line[0] == '\n') { continue; }
        // Testing for invalid target formats is above
        n = sscanf(line, "[%lf, %lf, %lf, %lf, %lf, %lf] %d %d\n",
            &bbox.tlhc_x, &bbox.tlhc_y, &bbox.width, &bbox.height, &bbox.spacing,
            &bbox.reflectivity, &bbox.firstImage, &bbox.lastImage);
        (*target_bboxes)[i++] = bbox;
    }

    fclose(fp);
    free(line);

    return num_targets;
}


#if 0
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
#endif
