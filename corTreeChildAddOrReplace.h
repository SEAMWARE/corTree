#ifndef CORTREE_CHILD_ADD_OR_REPLACE_H_
#define CORTREE_CHILD_ADD_OR_REPLACE_H_

//
// FILE            corTreeChildAddOrReplace.h
//
// AUTHOR          Ken Zangelin
//
// Copyright 2025 Ken Zangelin
//
// SPDX-License-Identifier: Apache-2.0
//
#include "corTree/CorNode.h"                                      // CorNode



// -----------------------------------------------------------------------------
//
// corTreeChildAddOrReplace - replace the child called 'itemName' by 'replacementP' (in its place), or add it
//
// Returns the child replaced, out of the tree, or NULL if 'replacementP' was added.
//
extern CorNode* corTreeChildAddOrReplace(CorNode* container, const char* itemName, CorNode* replacementP);

#endif  // CORTREE_CHILD_ADD_OR_REPLACE_H_
