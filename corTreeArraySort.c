//
// FILE            corTreeArraySort.c
//
// AUTHOR          Ken Zangelin
//
// Copyright 2026 Ken Zangelin
//
// SPDX-License-Identifier: Apache-2.0
//
#include <string.h>                             // strcmp, memset
#include <unistd.h>                             // NULL

#include "corTree/CorNode.h"                       // CorNode
#include "corTree/corTreeBuilder.h"                    // corTreeChildRemove, corTreeChildAdd
#include "corTree/corTreeSort.h"                       // corTreeSort
#include "corTree/corTreeArraySort.h"                  // Own interface



// -----------------------------------------------------------------------------
//
// stringCompare - strcmp that accepts NULL, treating it as the empty string
//
// The name of an Array element is "" when the tree comes from corJsonParse, but a
// hand-built node may well have a NULL name.
//
static int stringCompare(const char* a, const char* b)
{
  if (a == NULL)  a = "";
  if (b == NULL)  b = "";

  return strcmp(a, b);
}



// -----------------------------------------------------------------------------
//
// corTreeNodeCompare - compare two KjNodes, recursively
//
// Nothing is rendered and nothing is allocated - the two trees are walked in
// parallel and the comparison stops at the first difference.
//
int corTreeNodeCompare(CorNode* aP, CorNode* bP)
{
  int diff;

  if (aP->type != bP->type)
    return (int) aP->type - (int) bP->type;

  if ((diff = stringCompare(aP->name, bP->name)) != 0)
    return diff;

  switch (aP->type)
  {
  case CorString:
    return stringCompare(aP->value.s, bP->value.s);

  case CorInt:
    if (aP->value.i < bP->value.i)   return -1;
    if (aP->value.i > bP->value.i)   return 1;
    return 0;

  case CorFloat:
    if (aP->value.f < bP->value.f)   return -1;
    if (aP->value.f > bP->value.f)   return 1;
    return 0;

  case CorBoolean:
    return (int) aP->value.b - (int) bP->value.b;

  case CorObject:
  case CorArray:
  {
    CorNode* aChildP = aP->value.head;
    CorNode* bChildP = bP->value.head;

    while ((aChildP != NULL) && (bChildP != NULL))
    {
      if ((diff = corTreeNodeCompare(aChildP, bChildP)) != 0)
        return diff;

      aChildP = aChildP->next;
      bChildP = bChildP->next;
    }

    // All pairs equal - the shorter container comes first
    if (aChildP != NULL)   return 1;
    if (bChildP != NULL)   return -1;
    return 0;
  }

  case CorNone:
  case CorNull:
    return 0;  // No value to compare - the type is all there is
  }

  return 0;
}



// -----------------------------------------------------------------------------
//
// elementsOrder - order the children of an Array, using corTreeNodeCompare
//
// Selection sort: pull every child out into a local container and move them
// back, smallest first. Same shape as corTreeStringArraySort.
//
static void elementsOrder(CorNode* arrayP)
{
  CorNode  raw;
  CorNode* rawP = &raw;

  memset(&raw, 0, sizeof(raw));
  rawP->type = CorArray;

  //
  // Put all items in rawP, leaving arrayP empty
  //
  rawP->value.head          = arrayP->value.head;
  rawP->value.tail          = arrayP->value.tail;
  arrayP->value.head = NULL;
  arrayP->value.tail        = NULL;

  //
  // Loop over rawP, find smallest, then just move it to arrayP
  //
  while (rawP->value.head != NULL)
  {
    CorNode* minP = rawP->value.head;

    for (CorNode* tmpP = minP->next; tmpP != NULL; tmpP = tmpP->next)
    {
      if (corTreeNodeCompare(minP, tmpP) > 0)
        minP = tmpP;
    }

    corTreeChildRemove(rawP, minP);
    corTreeChildAdd(arrayP, minP);
  }
}



// -----------------------------------------------------------------------------
//
// containerOrder - order every Array in the sub-tree, deepest first
//
// The children are done before the container itself, so that when the elements
// of an Array are compared, everything below them is already canonical.
//
static void containerOrder(CorNode* nodeP)
{
  for (CorNode* childP = nodeP->value.head; childP != NULL; childP = childP->next)
  {
    if ((childP->type == CorObject) || (childP->type == CorArray))
      containerOrder(childP);
  }

  if (nodeP->type == CorArray)
    elementsOrder(nodeP);
}



// -----------------------------------------------------------------------------
//
// corTreeArraySort - order the elements of a CorNode Array
//
void corTreeArraySort(CorNode* arrayP)
{
  if (arrayP->type != CorArray)
    return;

  if (arrayP->value.head == NULL)
    return;

  //
  // The elements are compared member by member, in list order, so the members must be in a
  // canonical order first - otherwise two elements with identical content but built by two
  // different pieces of code compare as different, and where they end up depends on which
  // piece of code built them.
  //
  // corTreeSort does exactly that and nothing else - it orders the members of every Object in the
  // sub-tree and leaves the order of every Array alone.
  //
  corTreeSort(arrayP);

  containerOrder(arrayP);
}
