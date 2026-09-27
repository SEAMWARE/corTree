#ifndef CORTREE_STRING_ARRAY_SORT_H_
#define CORTREE_STRING_ARRAY_SORT_H_

//
// FILE            corTreeStringArraySort.h
//
// AUTHOR          Ken Zangelin
//
// Copyright 2025 Ken Zangelin
//
// SPDX-License-Identifier: Apache-2.0
//
#include "corTree/CorNode.h"               // CorNode



// -----------------------------------------------------------------------------
//
// corTreeStringArraySort - sort a CorNode Array
//
// Elements (CorString) are sorted alphabetically by value.
//
extern void corTreeStringArraySort(CorNode* arrayP);

#endif  // CORTREE_STRING_ARRAY_SORT_H_
