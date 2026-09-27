#ifndef CORTREE_BUILDER_H_
#define CORTREE_BUILDER_H_

//
// FILE            corTreeBuilder.h
//
// AUTHOR          Ken Zangelin
//
// Copyright 2019 Ken Zangelin
//
// SPDX-License-Identifier: Apache-2.0
//
#include <stdbool.h>                         // bool

#include "corAlloc/CorAlloc.h"               // CorAlloc
#include "corTree/CorNode.h"                 // CorNode



// -----------------------------------------------------------------------------
//
// corTree builder functions -
//
extern CorNode* corTreeObject(CorAlloc* kaP, const char* name);
extern CorNode* corTreeArray(CorAlloc* kaP, const char* name);
extern CorNode* corTreeString(CorAlloc* kaP, const char* name, const char* value);
extern CorNode* corTreeInteger(CorAlloc* kaP, const char* name, long long value);
extern CorNode* corTreeFloat(CorAlloc* kaP, const char* name, double value);
extern CorNode* corTreeNull(CorAlloc* kaP, const char* name);
extern CorNode* corTreeBoolean(CorAlloc* kaP, const char* name, bool value);
extern CorNode* corTreeChildRemove(CorNode* container, CorNode* child);
extern void     corTreeChildAdd(CorNode* container, CorNode* child);
extern void     corTreeChildAddSorted(CorNode* container, CorNode* child);
extern void     corTreeChildAddSortedReverse(CorNode* container, CorNode* child);

#endif  // CORTREE_BUILDER_H_
