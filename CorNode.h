#ifndef CORTREE_NODE_H_
#define CORTREE_NODE_H_

//
// FILE            CorNode.h
//
// AUTHOR          Ken Zangelin
//
// Copyright 2019 Ken Zangelin
//
// SPDX-License-Identifier: Apache-2.0
//
//
// This file defines the CorNode structure, that contains everything necessary for
// the nodes in the JSON tree that is created as output of the parse step.
//
#include <stdbool.h>                         // bool



// -----------------------------------------------------------------------------
//
// CorValueType -
//
typedef enum CorValueType
{
  CorNone,
  CorString,
  CorInt,
  CorFloat,
  CorBoolean,
  CorNull,
  CorObject,
  CorArray
} CorValueType;



struct CorNode;
// -----------------------------------------------------------------------------
//
// CorValue - value of a node in the JSON tree
//
// 16 bytes: a scalar uses the first 8, a container's head/tail use all 16.
//
typedef union CorValue
{
  bool             b;
  long long        i;
  double           f;
  char*            s;
  struct                         // Object and Array: the children, a singly linked list via 'next'
  {
    struct CorNode*  head;       // First child, NULL for an empty container
    struct CorNode*  tail;       // Last child - only a container has one, so it lives here and not in every node
  };
} CorValue;



// -----------------------------------------------------------------------------
//
// CorNode - struct describing a node in a JSON tree
//
typedef struct CorNode
{
  char*           name;        // The name of the node, "" if father is an Array
  CorValueType     type;        // The type of the node. Number, String, Object, Array, ...
  unsigned char   flags;       // Bit flags for users of the lib; lands in the type->value alignment padding (CorNode is 40 bytes), and kalloc zeroes every allocation so it is born 0.
  CorValue         value;       // The value of the node - see CorValue
  struct CorNode*  next;        // Pointer to the next Sibling
} CorNode;



// -----------------------------------------------------------------------------
//
// corTreeValueType -
//
extern const char* corTreeValueType(CorValueType vt);



// -----------------------------------------------------------------------------
//
// corTreeValue -
//
extern char* corTreeValue(CorNode* nodeP, char* buf, int bufLen);

#endif  // CORTREE_NODE_H_
