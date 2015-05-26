/* Copyright (C) 2012 Reservoir Labs, Inc. All rights reserved. */

// SBIR DATA RIGHTS
// Contract No. HR0011-12-C-0123
// Contractor Name: Reservoir Labs, Inc.
// Contractor Address: 632 Broadway, Suite 803, New York, NY 10012
//
// Expiration of SBIR Data Rights Period: 8/31/2019, subject to Section 8 of the
// SBA SBIR Policy Directive of August 6, 2012.
//
// The Government's rights to use, modify, reproduce, release, perform, display, or
// disclose technical data or computer software marked with this legend are
// restricted during the period shown as provided in paragraph (b)(4) of the Rights
// in Noncommercial Technical Data and Computer Software: Small Business Innovative
// Research (SBIR) Program clause contained in the above identified contract. No
// restrictions apply after the expiration date shown above. Any reproduction of
// technical data, computer software, or portions thereof marked with this legend
// must also reproduce the markings.
//
// Distribution Statement “A” (Approved for Public Release, Distribution Unlimited).
//
// Copyright (C) 2003-2013 Reservoir Labs, Inc. All rights not granted to the
// Government are retained by the Contractor, and as Confidential and Proprietary.

/**
 * @file rstream_ocr.h
 * R-Stream runtime layer for OCR.
 * @copyright Copyright (C) Reservoir Labs, Inc. All rights reserved.
 */

#ifndef __RSTREAM_OCR_H__
#define __RSTREAM_OCR_H__

#ifdef SOCR
    #include <rstream_socr.h>
    #define rocrAlloc(...) rsocrAlloc(__VA_ARGS__)
    #define rocrDeclareType(...) rsocrDeclareType(__VA_ARGS__)
    #define rocrCreateTask(...) rsocrCreateTask(__VA_ARGS__)
    #define rocrInit(...) rsocrInit(__VA_ARGS__)
    #define rocrArgs(...) rsocrArgs(__VA_ARGS__)
    #define rocrAsyncFree(...) rsocrAsyncFree(__VA_ARGS__)
    #define rocrAsyncDbDestroy(...) rsocrAsyncDbDestroy(__VA_ARGS__)
    #define rocrFreeInputArgs(...) rsocrFreeInputArgs(__VA_ARGS__)
    #define rocrAutoDec(...) rsocrAutoDec(__VA_ARGS__)
    #define rocrParseInputArgs(...) rsocrParseInputArgs(__VA_ARGS__)

#ifdef ROCR_ASYNCHRONOUS
    #define rocrExecute(...) rsocrExecute(false, __VA_ARGS__)
    #define rocrExit(...)
#else
    #define rocrExecute(...) rsocrExecute(true, __VA_ARGS__)
    #define rocrExit(...) rsocrExit(__VA_ARGS__)
#endif

#else
    #include <rstream_docr.h>
    #define rocrAlloc(...) rdocrAlloc(__VA_ARGS__)
    #define rocrDeclareType(...) rdocrDeclareType(__VA_ARGS__)
    #define rocrDeclareTask(...) rdocrDeclareTask(__VA_ARGS__)
    #define rocrExecute(...) rdocrExecute(__VA_ARGS__)
    #define rocrDeclareDependence(...) rdocrDeclareDependence(__VA_ARGS__)
    #define rocrScheduleAll(...) rdocrScheduleAll(__VA_ARGS__)
    #define rocrInit(...) rdocrInit(__VA_ARGS__)
    #define rocrExit(...) rdocrExit(__VA_ARGS__)
    #define rocrArgs(...) rdocrArgs(__VA_ARGS__)
    #define rocrAsyncFree(...) rdocrAsyncFree(__VA_ARGS__)
    #define rocrAsyncDbDestroy(...) rdocrAsyncDbDestroy(__VA_ARGS__)
    #define rocrFreeInputArgs(...) rsocrFreeInputArgs(__VA_ARGS__)
    #define rocrParseInputArgs(...)
#endif // SOCR

#endif // __RSTREAM_OCR_H__
