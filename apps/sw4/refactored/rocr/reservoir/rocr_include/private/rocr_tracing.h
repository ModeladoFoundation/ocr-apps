/* Copyright (C) 2016 Reservoir Labs, Inc. All rights reserved. */

#ifndef ROCR_TRACING_H
#define ROCR_TRACING_H

#include "rocr_config.h"
#include "rocr_shared_context.h"

#ifdef ROCR_TRACE

/**
 * Registers a link between an EDT and a DB.
 *
 * @param ctx the current shared context
 * @param edtTp type identifier of the EDT
 * @param edtId identifier of the EDT
 * @param dbTp type identifier of the DB
 * @param dbId identifier of the DB
 * @param written whether the DB is accessed with write permissions
 */
void traceEdtDB(RocrSharedCtx *ctx, unsigned int edtTp, unsigned long edtId,
    unsigned int dbTp, unsigned long dbId, unsigned int written);

/**
 * Registers a link between an EDT and another EDT.
 *
 * @param ctx the current shared context
 * @param srcTp type identifier of the source EDT
 * @param srcId identifier of the source EDT
 * @param dstTp type identifier of the destination EDT
 * @param dstId identifier of the destination EDT
 */
void traceEdtEdt(RocrSharedCtx *ctx, unsigned int srcTp, unsigned long srcId,
    unsigned int dstTp, unsigned long dstId);

/**
 * Reports that an EDT touches a distant DB at creation
 *
 * @param ctx the current shared context
 * @param edtTp type identifier of the source EDT
 * @param edtId identifier of the source EDT
 * @param dbTp type identifier of the DB
 * @param dbId identifier of the DB
 */
void traceEdtDistDB(RocrSharedCtx *ctx, unsigned int edtTp, unsigned long edtId,
    unsigned int dbTp, unsigned long dbId);

/**
 * Registers the beginning of an EDT.
 *
 * @param ctx the current shared context
 * @param edtTp type identifier of the EDT
 * @param edtId identifier of the EDT
 */
void traceEdtStart(RocrSharedCtx *ctx, unsigned int edtTp, unsigned long edtId);

/**
 * Registers the ending of an EDT.
 *
 * @param ctx the current shared context
 * @param edtTp type identifier of the EDT
 * @param edtId identifier of the EDT
 */
void traceEdtEnd(RocrSharedCtx *ctx, unsigned int edtTp, unsigned long edtId);

/**
 * Registers the beginning of the execution of a new task graph.
 */
void traceGraphStart();

#endif

#endif

