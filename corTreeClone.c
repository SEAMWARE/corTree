//
// FILE            corTreeClone.c - clone a CorNode
//
// AUTHOR          Ken Zangelin
//
// Copyright 2019 Ken Zangelin
//
// SPDX-License-Identifier: Apache-2.0
//
#include <stdio.h>                      // NULL
#include <string.h>                     // strcmp

#include "corLog/corLog.h"                // COR_E, COR_RE, COR_T
#include "kbase/kBasicLog.h"            // K BasicLog macros
#include "kalloc/KAlloc.h"                 // KAlloc
#include "corTree/CorNode.h"               // CorNode
#include "corTree/corTreeBuilder.h"            // corTreeString, corTreeArray, corTreeFloat, ...
#include "corTree/corTreeClone.h"              // Own Interface



// -----------------------------------------------------------------------------
//
// corTreeClone - clone a node
//
CorNode* corTreeClone(KAlloc* kaP, CorNode* nodeP)
{
  CorNode* newNodeP = NULL;

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


  return newNodeP;
}
