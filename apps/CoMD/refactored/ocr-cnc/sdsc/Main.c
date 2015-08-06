#include "comd.h"
#include <string.h>

void processArgs(comdCtx *ctx, u32 argc, char *argv[]);

int cncMain(int argc, char *argv[]) {

  // Create a new graph contex
  comdCtx *ctx = comd_create();

  processArgs(ctx, argc, argv);

  // Exit when the graph execution completes
  CNC_SHUTDOWN_ON_FINISH(ctx);

  // Launch the graph for execution
  comd_launch(NULL, ctx);
  return 0;
}

void processArgs(comdCtx *ctx, u32 argc, char *argv[]) {

/*  strcpy(ctx->potDir,  "pots\0");
  strcpy(ctx->potName, "\0"); // default depends on potType
  strcpy(ctx->potType, "funcfl");
*/
  ctx->doeam = 0;
  ctx->nx = 20;
  ctx->ny = 20;
  ctx->nz = 20;
  ctx->xproc = 1;
  ctx->yproc = 1;
  ctx->zproc = 1;
  ctx->nSteps = 100;
  ctx->printRate = 10;
  ctx->dt = 1.0;
  ctx->lat = -1.0;
  ctx->temperature = 600.0;
  ctx->initialDelta = 0.0;

  int i;
  for (i = 0; i < argc; i++) {

    if (!strcmp(argv[i],"-e")) {
       i++; ctx->doeam = atoi(argv[i]);
    } else if (!strcmp(argv[i],"-x")) {
       i++; ctx->nx = atoi(argv[i]);
    } else if (!strcmp(argv[i],"-y")) {
       i++; ctx->ny = atoi(argv[i]);
    } else if (!strcmp(argv[i],"-z")) {
       i++; ctx->nz = atoi(argv[i]);
    } else if (!strcmp(argv[i],"-i")) {
       i++; ctx->xproc = atoi(argv[i]);
    } else if (!strcmp(argv[i],"-j")) {
       i++; ctx->yproc = atoi(argv[i]);
    } else if (!strcmp(argv[i],"-k")) {
       i++; ctx->zproc = atoi(argv[i]);
    } else if (!strcmp(argv[i],"-D")) {
       i++; ctx->dt = atof(argv[i]);
    } else if (!strcmp(argv[i],"-l")) {
       i++; ctx->lat = atof(argv[i]);
    } else if (!strcmp(argv[i],"-T")) {
       i++; ctx->temperature = atof(argv[i]);
    } else if (!strcmp(argv[i],"-r")) {
       i++; ctx->initialDelta = atof(argv[i]);
    } else if (!strcmp(argv[i],"-n")) {
       i++; ctx->printRate = atoi(argv[i]);
    } else if (!strcmp(argv[i],"-N")) {
       i++; ctx->nSteps = atoi(argv[i]);
    }

  }

  printf("doeam = %d, nx = %d, ny = %d, nz = %d, xproc = %d, yproc = %d, zproc = %d\n", ctx->doeam, ctx->nx, ctx->ny, ctx->nz, ctx->xproc, ctx->yproc, ctx->zproc);
  printf("nSteps = %d, printRate = %d, dt = %f, lat = %f, temperature = %f, initialDelta = %f\n",  ctx->nSteps,  ctx->printRate,  ctx->dt, ctx->lat,  ctx->temperature,  ctx->initialDelta);

}

