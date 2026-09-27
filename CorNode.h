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
typedef union CorValue
{
  bool           b;
  long long       i;
  double          f;
  char*           s;
  struct CorNode*  firstChildP;
} CorValue;



// -----------------------------------------------------------------------------
//
// CorNode - struct describing a node in a JSON tree
//
typedef struct CorNode
{
  char*           name;        // The name of the node, "" if father is an Array
  CorValueType     type;        // The type of the node. Number, String, Object, Array, ...
  unsigned char   flags;       // Bit flags for users of the lib; lands in the type->value alignment padding (CorNode stays 40 bytes), and kalloc zeroes every allocation so it is born 0.
  CorValue         value;       // The value of the node - see CorValue
  struct CorNode*  next;        // Pointer to the next Sibling
  struct CorNode*  lastChild;   // Pointer to the last child of this node - FIXME: to be removed (move to struct KjContainer?)


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
