//
// FILE            corTreeClone.c - clone a CorNode
//
// AUTHOR          Ken Zangelin
//
// Copyright 2019 Ken Zangelin
//
// SPDX-License-Identifier: Apache-2.0
//
#include <stdbool.h>                    // bool
#include <stddef.h>                     // offsetof
#include <stdio.h>                      // NULL
#include <string.h>                     // strlen, memcpy

#include "corLog/corLog.h"                // COR_E, COR_RE, COR_T
#include "corAlloc/CorAlloc.h"             // CorAlloc
#include "corTree/CorNode.h"               // CorNode
#include "corTree/corTreeBuilder.h"            // corTreeString, corTreeArray, corTreeFloat, ...
#include "corTree/corTreeHeap.h"               // corTreeHeapAlloc
#include "corTree/corTreeClone.h"              // Own Interface



// -----------------------------------------------------------------------------
//
// heapClone - corTreeClone into the tree heap: the trees that live on (a store's entities, the caches).
//
// A node that is not a container is allocated without the container's tail (CorNode.h): 32 bytes, not 40.
// Its name and a string value follow it, in the same block.
//
static CorNode* heapClone(CorNode* nodeP)
{
  bool     container = (nodeP->type == CorObject) || (nodeP->type == CorArray);
  int      nodeBytes = container? (int) sizeof(CorNode) : (int) offsetof(CorNode, value.tail);
  int      nameLen   = (nodeP->name != NULL)? strlen(nodeP->name) + 1 : 0;
  int      valueLen  = ((nodeP->type == CorString) && (nodeP->value.s != NULL))? strlen(nodeP->value.s) + 1 : 0;
  char*    buf       = corTreeHeapAlloc(nodeBytes + nameLen + valueLen);
  CorNode* copyP     = (CorNode*) buf;

  if (buf == NULL)
  {
    COR_E("Error cloning CorNode '%s' - out of memory?", nodeP->name);
    return NULL;
  }

  memcpy(copyP, nodeP, nodeBytes);   // type, flags, kind, termId and a scalar's value
  copyP->next = NULL;
  copyP->name = (nameLen != 0)? memcpy(&buf[nodeBytes], nodeP->name, nameLen) : NULL;

  if (valueLen != 0)
    copyP->value.s = memcpy(&buf[nodeBytes + nameLen], nodeP->value.s, valueLen);

  if (container)
  {
    copyP->value.head = NULL;
    copyP->value.tail = NULL;

    for (CorNode* itemP = nodeP->value.head; itemP != NULL; itemP = itemP->next)
    {
      CorNode* cloneP = heapClone(itemP);

      if (cloneP == NULL)
        return NULL;

      corTreeChildAdd(copyP, cloneP);
    }
  }

  return copyP;
}



// -----------------------------------------------------------------------------
//
// corTreeClone - clone a node
//
CorNode* corTreeClone(CorAlloc* kaP, CorNode* nodeP)
{
  CorNode* newNodeP = NULL;

  if (kaP == NULL)
    return heapClone(nodeP);

  switch (nodeP->type)
  {
  case CorString:
    newNodeP = corTreeString(kaP, nodeP->name, nodeP->value.s);
    break;

  case CorInt:
    newNodeP = corTreeInteger(kaP, nodeP->name, nodeP->value.i);
    break;

  case CorFloat:
    newNodeP = corTreeFloat(kaP, nodeP->name, nodeP->value.f);
    break;

  case CorBoolean:
    newNodeP = corTreeBoolean(kaP, nodeP->name, nodeP->value.b);
    break;

  case CorNull:
    newNodeP = corTreeNull(kaP, nodeP->name);
    break;

  case CorArray:
    newNodeP = corTreeArray(kaP, nodeP->name);

    if (newNodeP != NULL)
    {
      CorNode* arrItemP;

      for (arrItemP = nodeP->value.head; arrItemP != NULL; arrItemP = arrItemP->next)
      {
        CorNode* cloneP = corTreeClone(kaP, arrItemP);

        if (cloneP == NULL)
        {
          COR_E("Error cloning array item - out of memory?");
          return NULL;
        }

        corTreeChildAdd(newNodeP, cloneP);
      }
    }
    break;

  case CorObject:
    newNodeP = corTreeObject(kaP, nodeP->name);

    if (newNodeP != NULL)
    {
      CorNode* itemP;

      for (itemP = nodeP->value.head; itemP != NULL; itemP = itemP->next)
      {
        CorNode* cloneP = corTreeClone(kaP, itemP);

        if (cloneP == NULL)
        {
          COR_E("Error cloning CorNode '%s' - out of memory?", itemP->name);
          return NULL;
        }

        corTreeChildAdd(newNodeP, cloneP);
      }
    }
    break;

  case CorNone:
  default:
    COR_E("Invalid KJSON Value Type for node '%s'", nodeP->name);
    return NULL;
  }

  if (newNodeP == NULL)
    COR_E("Error cloning CorNode '%s' - out of memory?", nodeP->name);
  else
  {
    newNodeP->flags  = nodeP->flags;
    newNodeP->kind   = nodeP->kind;
    newNodeP->termId = nodeP->termId;
  }

  return newNodeP;
}



// -----------------------------------------------------------------------------
//
// corTreeCloneMarked -
//
CorNode* corTreeCloneMarked(CorAlloc* kaP, CorNode* nodeP, unsigned char mask, CorTreeMarkFn fn, void* ctx)
{
  if ((nodeP->type != CorObject) && (nodeP->type != CorArray))
    return corTreeClone(kaP, nodeP);

  CorNode* newNodeP = (nodeP->type == CorObject) ? corTreeObject(kaP, nodeP->name) : corTreeArray(kaP, nodeP->name);

  if (newNodeP == NULL)
  {
    COR_E("Error cloning CorNode '%s' - out of memory?", nodeP->name);
    return NULL;
  }

  for (CorNode* itemP = nodeP->value.head; itemP != NULL; itemP = itemP->next)
  {
    CorNode* cloneP = corTreeCloneMarked(kaP, itemP, mask, fn, ctx);

    if (cloneP == NULL)
      return NULL;

    corTreeChildAdd(newNodeP, cloneP);
  }

  newNodeP->flags  = nodeP->flags & ~mask;
  newNodeP->kind   = 0;
  newNodeP->termId = nodeP->termId;

  // A kind is a mark too: what the original keeps in it, the clone gets back as the user puts it
  if ((((nodeP->flags & mask) != 0) || (nodeP->kind != 0)) && (fn != NULL))
    fn(kaP, newNodeP, nodeP, ctx);

  return newNodeP;
}
