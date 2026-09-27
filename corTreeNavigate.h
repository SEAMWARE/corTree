#ifndef CORTREE_NAVIGATE_H_
#define CORTREE_NAVIGATE_H_

//
// FILE            corTreeNavigate.h
//
// AUTHOR          Ken Zangelin
//
// Copyright 2025 Ken Zangelin
//
// SPDX-License-Identifier: Apache-2.0
//
#include "corTree/CorNode.h"                                        // CorNode
#include <stdbool.h>                                                // bool



// -----------------------------------------------------------------------------
//
// corTreeNavigate -
//
extern CorNode* corTreeNavigate(CorNode* treeP, const char** pathCompV, CorNode** parentPP, bool* onlyLastMissingP);



// -----------------------------------------------------------------------------
//
// corTreeNavigate2 - true KJ-Tree navigation
//
extern CorNode* corTreeNavigate2(CorNode* treeP, char** compV);

#endif  // CORTREE_NAVIGATE_H_
