#ifndef CORTREE_STRING_VALUE_LOOKUP_IN_ARRAY_H_
#define CORTREE_STRING_VALUE_LOOKUP_IN_ARRAY_H_

//
// FILE            corTreeStringValueLookupInArray.h
//
// AUTHOR          Ken Zangelin
//
// Copyright 2025 Ken Zangelin
//
// SPDX-License-Identifier: Apache-2.0
//
#include "corTree/CorNode.h"                                        // CorNode



// -----------------------------------------------------------------------------
//
// corTreeStringValueLookupInArray -
//
// NOTE
//   This lookup function works on string items in an array.
//   The caller needs to be sure that the 'stringArray' is really an array and that ALL its items are Strings
//
extern CorNode* corTreeStringValueLookupInArray(CorNode* stringArray, const char* value);

#endif  // CORTREE_STRING_VALUE_LOOKUP_IN_ARRAY_H_
