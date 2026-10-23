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
#include <stddef.h>                          // offsetof
#include <stdint.h>                          // uint16_t



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
  struct CorNode*  next;        // Pointer to the next Sibling
  CorValueType     type;        // The type of the node. Number, String, Object, Array, ...
  unsigned char   flags;       // Bit flags for users of the lib - opaque to corTree
  unsigned char   kind;        // What an object IS for users of the lib (NGSI-LD: an attribute's type, 0 = none) - opaque to corTree
  uint16_t        termId;      // Term id for users of the lib (NGSI-LD: which core term, 0 = none) - opaque to corTree
  CorValue         value;       // The value of the node - see CorValue. LAST: only a container has a tail
} CorNode;

//
// flags, kind and termId share 8 bytes with type: the node stays 40 bytes. Every builder zeroes all
// three and corTreeClone copies all three - a tree builder that does neither (a parser, a DB reader)
// must do the same, or the users' classification of a node depends on which code path created it.
//
// kind is cor://'s object kind (cor-protocol-details § 4.2) held in the node: a user that knows what
// an object is - an NGSI-LD store, an attribute's type - can keep it here instead of in a member, and
// corTreeBin writes it as the object's kind with nothing to fold. 0: a plain object.
//
// A prefix layout: the value is last, and its last 8 bytes - value.tail - are a container's only. A
// node that is not a container can be allocated without them, 32 bytes instead of 40, and the trees
// that live on are: corTreeClone(NULL, ...) - the clone into the tree heap - does it. Such a node is
// never made a container in place, and never copied whole (*a = *b) into a 40-byte node's place; a
// change of size is a new node, put where the old one was.
//
#ifdef __cplusplus
static_assert(sizeof(CorNode) == 40,                   "CorNode must stay 40 bytes");
static_assert(offsetof(CorNode, value) == 24,          "CorNode.value must be last");
static_assert(offsetof(CorNode, value.tail) == 32,     "CorNode.value.tail must be the node's last 8 bytes");
#else
_Static_assert(sizeof(CorNode) == 40,                  "CorNode must stay 40 bytes");
_Static_assert(offsetof(CorNode, value) == 24,         "CorNode.value must be last");
_Static_assert(offsetof(CorNode, value.tail) == 32,    "CorNode.value.tail must be the node's last 8 bytes");
#endif



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
