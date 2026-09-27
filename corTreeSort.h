#ifndef CORTREE_SORT_H_
#define CORTREE_SORT_H_

//
// FILE            corTreeSort.h
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
// corTreeSort - sort a CorNode Object
//
// Objects are sorted alphabetically by key.
// Arrays are not sorted - use corTreeStringArraySort for that
//
extern void corTreeSort(CorNode* nodeP);

#endif  // CORTREE_SORT_H_
