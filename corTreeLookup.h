#ifndef CORTREE_LOOKUP_H_
#define CORTREE_LOOKUP_H_

//
// FILE            corTreeLookup.h
//
// AUTHOR          Ken Zangelin
//
// Copyright 2019 Ken Zangelin
//
// SPDX-License-Identifier: Apache-2.0
//
#include "corTree/CorNode.h"               // CorNode



// -----------------------------------------------------------------------------
//
// corTreeLookup - look up a node named 'name' in container 'container'
//
// For containers of type CorObject only, the name of the nodes (children of the object)
// are compared tothe parameter 'name' and if equal, a pointer to the node is returned.
//
extern CorNode* corTreeLookup(CorNode* container, const char* name);
extern CorNode* corTreeLookupWithStrcmp(CorNode* container, char* name);


#endif  // CORTREE_LOOKUP_H_
