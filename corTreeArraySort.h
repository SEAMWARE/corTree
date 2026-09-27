#ifndef CORTREE_ARRAY_SORT_H_
#define CORTREE_ARRAY_SORT_H_

//
// FILE            corTreeArraySort.h
//
// AUTHOR          Ken Zangelin
//
// Copyright 2026 Ken Zangelin
//
// SPDX-License-Identifier: Apache-2.0
//
#include "corTree/CorNode.h"               // CorNode



// -----------------------------------------------------------------------------
//
// corTreeNodeCompare - compare two KjNodes, recursively
//
// The order is: type, then name, then value. For containers, the children are
// compared pairwise, in list order, and if all pairs are equal, the container
// with fewer children comes first.
//
// Returns < 0 if aP < bP, 0 if they are equal, > 0 if aP > bP.
//
extern int corTreeNodeCompare(CorNode* aP, CorNode* bP);



// -----------------------------------------------------------------------------
//
// corTreeArraySort - order the elements of a CorNode Array
//
// Elements of ANY type are ordered, using corTreeNodeCompare. Arrays found deeper
// down (inside the elements) are ordered as well, so one call makes the entire
// sub-tree canonical.
//
// ⚠️ It calls corTreeSort on the way in, so the MEMBERS of every Object in the
// sub-tree are ordered as well. That is not a bonus, it is a precondition: the
// elements are compared member by member in list order, so two elements with
// identical content but built by two different pieces of code have to look
// identical before they can compare equal.
//
// Contrast corTreeSort, that orders the members of an Object and never touches the
// order of an Array, and corTreeStringArraySort, that orders an Array of Strings.
//
// ⚠️ The order of an Array is significant in most JSON-based data models - it is
// part of the value. Only ever call this on an Array whose order carries no
// meaning (a result list, a set of errors, ...), and never on data on its way in
// or out.
//
extern void corTreeArraySort(CorNode* arrayP);

#endif  // CORTREE_ARRAY_SORT_H_
