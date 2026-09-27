
//
// FILE            corTreeTraceLevels.h
//
// AUTHOR          Ken Zangelin
//
// Copyright 2019 Ken Zangelin
//
// SPDX-License-Identifier: Apache-2.0
//
#ifndef CORTREE_TRACELEVELS_H_
#define CORTREE_TRACELEVELS_H_

//
// Trace levels for the corTree library.
//
// The trace-level space is numeric and shared: every library takes a slice and
// the owner switches levels on by number (coraine: --traceLevels / -t). The
// JSON tree and parser took 50-66 together when they were one library (kjson),
// and the numbers did not move when they were split: corTree keeps 55 and
// 64-66, corJson the rest.
// The slices in use elsewhere are corRest 100-116, corJsonld 150-156 and
// corNgsild 200-240.
//
#define CorTreeTlMalloc          55   // Memory allocation
#define CorTreeTlBuilder         64   // Building a tree
#define CorTreeTlFree            65   // Freeing nodes
#define CorTreeTlLookup          66   // Lookups

#endif  // CORTREE_TRACELEVELS_H_
