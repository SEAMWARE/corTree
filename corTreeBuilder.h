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

#include "kalloc/KAlloc.h"                   // KAlloc
#include "corTree/CorNode.h"                 // CorNode



// -----------------------------------------------------------------------------
//
// corTree builder functions -
//
extern CorNode* corTreeObject(KAlloc* kaP, const char* name);
extern CorNode* corTreeArray(KAlloc* kaP, const char* name);
extern CorNode* corTreeString(KAlloc* kaP, const char* name, const char* value);
extern CorNode* corTreeInteger(KAlloc* kaP, const char* name, long long value);
extern CorNode* corTreeFloat(KAlloc* kaP, const char* name, double value);
extern CorNode* corTreeNull(KAlloc* kaP, const char* name);
extern CorNode* corTreeBoolean(KAlloc* kaP, const char* name, bool value);
extern CorNode* corTreeChildRemove(CorNode* container, CorNode* child);
extern void     corTreeChildAdd(CorNode* container, CorNode* child);
extern void     corTreeChildAddSorted(CorNode* container, CorNode* child);
extern void     corTreeChildAddSortedReverse(CorNode* container, CorNode* child);

#endif  // CORTREE_BUILDER_H_
