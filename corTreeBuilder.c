//
// FILE            corTreeBuilder.c - build a JSON tree
//
// AUTHOR          Ken Zangelin
//
// Copyright 2019 Ken Zangelin
//
// SPDX-License-Identifier: Apache-2.0
//
#include <stdio.h>                           // NULL
#include <string.h>                          // strcmp
#include <stdlib.h>                          // malloc

#include <stdbool.h>                              // bool
#include "corLog/corLog.h"                // COR_E, COR_RE, COR_T
#include "kalloc/kaAlloc.h"                  // kaAlloc

#include "corTree/corTreeTraceLevels.h"             // KJ Log Trace Levels
#include "kalloc/KAlloc.h"                  // KAlloc
#include "corTree/CorNode.h"                    // CorNode
#include "corTree/corTreeBuilder.h"                 // Own Interface





// -----------------------------------------------------------------------------
//
// corTreeObject - create a kjson object
//
// PARAMETERS
//   kaP   - the allocator; NULL means malloc
//   name  - the name of the object
//
// RETURN VALUE
//   `corTreeObject` returns a CorNode of `CorObject` type.
//
CorNode* corTreeObject(KAlloc* kaP, const char* name)
{
  int     nameLen = (name == NULL)? 0 : strlen(name);
  int     bufLen  = sizeof(CorNode) + nameLen + 1;
  char*   buf     = (kaP != NULL)? kaAlloc(kaP, bufLen) : malloc(bufLen);
  CorNode* nodeP   = (CorNode*) buf;

  if (nodeP == NULL)
    COR_RE(NULL, "malloc failed to allocate %d bytes", bufLen);

  if (name != NULL)
  {
    nodeP->name = &buf[sizeof(CorNode)];
    memcpy(nodeP->name, name, nameLen + 1);  // Safe: buffer sized for nameLen + null terminator
  }
  else
    nodeP->name = NULL;

  nodeP->type              = CorObject;
  nodeP->next              = NULL;
  nodeP->value.head = NULL;
  nodeP->value.tail        = NULL;

  return nodeP;
}



// -----------------------------------------------------------------------------
//
// corTreeArray - create a kjson array
//
// PARAMETERS
//   kaP   - the allocator; NULL means malloc
//   name  - the name of the array
//
// RETURN VALUE
//   `corTreeArray` returns a CorNode of `CorArray` type.
//
CorNode* corTreeArray(KAlloc* kaP, const char* name)
{
  int     nameLen = (name == NULL)? 0 : strlen(name);
  int     bufLen  = sizeof(CorNode) + nameLen + 1;
  char*   buf     = (kaP != NULL)? kaAlloc(kaP, bufLen) : malloc(bufLen);
  CorNode* nodeP   = (CorNode*) buf;

  if (nodeP == NULL)
    COR_RE(NULL, "malloc failed to allocate %d bytes", bufLen);

  if (name != NULL)
  {
    nodeP->name = &buf[sizeof(CorNode)];
    memcpy(nodeP->name, name, nameLen + 1);  // Safe: buffer sized for nameLen + null terminator
  }
  else
    nodeP->name = NULL;

  nodeP->type              = CorArray;
  nodeP->next              = NULL;
  nodeP->value.head = NULL;
  nodeP->value.tail        = NULL;

  return nodeP;
}



// -----------------------------------------------------------------------------
//
// corTreeString -
//
// PARAMETERS
//   kaP   - the allocator; NULL means malloc
//   name  - the name of the string field
//   value - the content of the string
//
// RETURN VALUE
//   `corTreeString` returns a CorNode of `CorString` type with the contents from the third parameter `value`
//
//
// FIXME: Include the 'value' in the malloc of 'buf' as well
//
CorNode* corTreeString(KAlloc* kaP, const char* name, const char* value)
{
  int     nameLen  = (name  != NULL)?  strlen(name) : 0;
  int     valueLen = (value != NULL)?  strlen(value) : 0;
  int     bufLen   = sizeof(CorNode) + nameLen + 1 + valueLen + 1;
  char*   buf      = (kaP != NULL)? kaAlloc(kaP, bufLen) : malloc(bufLen);
  CorNode* nodeP    = (CorNode*) buf;

  if (nodeP == NULL)
  {
    //
    // Assuming the error is due to too long string
    // This also prevents an infinite recursion as in the second call, bufLen is quite small
    //
    if (bufLen > 16 * 1024)
    {
      return corTreeString(kaP, name, "too big string");
    }
    else
      COR_RE(NULL, "malloc failed to allocate %d bytes", bufLen);
  }

  if (name != NULL)
  {
    nodeP->name = &buf[sizeof(CorNode)];
    memcpy(nodeP->name, name, nameLen + 1);  // Safe: buffer sized for nameLen + null terminator
  }
  else
    nodeP->name = NULL;

  nodeP->type        = CorString;
  nodeP->next        = NULL;

  if (value != NULL)
  {
    nodeP->value.s = &buf[sizeof(CorNode) + nameLen + 1];
    memcpy(nodeP->value.s, value, valueLen + 1);  // Safe: buffer sized for valueLen + null terminator
  }
  else
  {
    nodeP->value.s = NULL;
  }

  return nodeP;
}



// -----------------------------------------------------------------------------
//
// corTreeInteger -
//
// PARAMETERS
//   kaP   - the allocator; NULL means malloc
//   name  - the name of the integer field
//   value - the value of the integer field
//
// RETURN VALUE
//   `corTreeInteger` returns a CorNode of `CorInt` type with the value of the third parameter `value`
//
CorNode* corTreeInteger(KAlloc* kaP, const char* name, long long value)
{
  int     nameLen = (name == NULL)? 0 : strlen(name);
  int     bufLen  = sizeof(CorNode) + nameLen + 1;
  char*   buf     = (kaP != NULL)? kaAlloc(kaP, bufLen) : malloc(bufLen);
  CorNode* nodeP   = (CorNode*) buf;

  if (nodeP == NULL)
    COR_RE(NULL, "malloc failed to allocate %d bytes", bufLen);

  if (name != NULL)
  {
    nodeP->name = &buf[sizeof(CorNode)];
    memcpy(nodeP->name, name, nameLen + 1);  // Safe: buffer sized for nameLen + null terminator
  }
  else
    nodeP->name = NULL;

  nodeP->type     = CorInt;
  nodeP->value.i  = value;

  return nodeP;
}



// -----------------------------------------------------------------------------
//
// corTreeFloat -
//
// PARAMETERS
//   kaP   - the allocator; NULL means malloc
//   name  - the name of the field
//   value - the value of the field
//
// RETURN VALUE
//   `corTreeFloat` returns a CorNode of `CorFloat` type with the value of the third parameter `value`
//
CorNode* corTreeFloat(KAlloc* kaP, const char* name, double value)
{
  int     nameLen = (name == NULL)? 0 : strlen(name);
  int     bufLen  = sizeof(CorNode) + nameLen + 1;
  char*   buf     = (kaP != NULL)? kaAlloc(kaP, bufLen) : malloc(bufLen);
  CorNode* nodeP   = (CorNode*) buf;

  if (nodeP == NULL)
    COR_RE(NULL, "malloc failed to allocate %d bytes", bufLen);

  if (name != NULL)
  {
    nodeP->name = &buf[sizeof(CorNode)];
    memcpy(nodeP->name, name, nameLen + 1);  // Safe: buffer sized for nameLen + null terminator
  }
  else
    nodeP->name = NULL;

  nodeP->type     = CorFloat;
  nodeP->value.f  = value;

  return nodeP;
}



// -----------------------------------------------------------------------------
//
// corTreeNull -
//
// PARAMETERS
//   kaP   - the allocator; NULL means malloc
//   name  - the name of the field
//
// RETURN VALUE
//   `corTreeNull` returns a CorNode of `CorNull` type.
//
CorNode* corTreeNull(KAlloc* kaP, const char* name)
{
  int     nameLen = (name == NULL)? 0 : strlen(name);
  int     bufLen  = sizeof(CorNode) + nameLen + 1;
  char*   buf     = (kaP != NULL)? kaAlloc(kaP, bufLen) : malloc(bufLen);
  CorNode* nodeP   = (CorNode*) buf;

  if (nodeP == NULL)
    COR_RE(NULL, "malloc failed to allocate %d bytes", bufLen);

  if (name != NULL)
  {
    nodeP->name = &buf[sizeof(CorNode)];
    memcpy(nodeP->name, name, nameLen + 1);  // Safe: buffer sized for nameLen + null terminator
  }
  else
    nodeP->name = NULL;

  nodeP->type = CorNull;

  return nodeP;
}



// -----------------------------------------------------------------------------
//
// corTreeBoolean -
//
// PARAMETERS
//   kaP   - the allocator; NULL means malloc
//   name  - the name of the boolean field
//   value - the value of the boolean field
//
// RETURN VALUE
//   `corTreeBoolean` returns a CorNode of `CorBoolean` type with the value of the third parameter `value`
//
CorNode* corTreeBoolean(KAlloc* kaP, const char* name, bool value)
{
  int     nameLen = (name == NULL)? 0 : strlen(name);
  int     bufLen  = sizeof(CorNode) + nameLen + 1;
  char*   buf     = (kaP != NULL)? kaAlloc(kaP, bufLen) : malloc(bufLen);
  CorNode* nodeP   = (CorNode*) buf;

  if (nodeP == NULL)
    COR_RE(NULL, "malloc failed to allocate %d bytes", bufLen);

  if (name != NULL)
  {
    nodeP->name = &buf[sizeof(CorNode)];
    memcpy(nodeP->name, name, nameLen + 1);  // Safe: buffer sized for nameLen + null terminator
  }
  else
    nodeP->name = NULL;

  nodeP->type     = CorBoolean;
  nodeP->value.b  = value;

  return nodeP;
}



// -----------------------------------------------------------------------------
//
// corTreeChildAdd - add a node to a container
//
// PARAMETERS
//   container - pointer to the father (container) of the child to be added
//   child     - pointer to the child to be added
//
// NOTE
//   The child is appended to the list of children.
//   If 'child' is already part of a linked list, it will be removed from that list.
//
void corTreeChildAdd(CorNode* container, CorNode* child)
{
  // First child?
  if (container->value.head != NULL)
    container->value.tail->next = child;
  else
    container->value.head         = child;

  container->value.tail = child;

  child->next = NULL;
}



// -----------------------------------------------------------------------------
//
// corTreeChildRemove - remove a node from a container
//
// PARAMETERS
//   container - pointer to the father (container) of the child to be removed
//   child     - pointer to the child to be removed
//
// RETURN VALUE
//   'child' if it was found in 'container' and unlinked, NULL if it was not there
//
CorNode* corTreeChildRemove(CorNode* container, CorNode* child)
{
  CorNode* prevP = NULL;
  CorNode* nodeP = container->value.head;

  while (nodeP != child)
  {
    if (nodeP == NULL)
      return NULL;  // Not found

    prevP = nodeP;
    nodeP = nodeP->next;
  }

  if (nodeP == container->value.head)         // Hit in beginning of list
  {
    container->value.head = nodeP->next;

    if (container->value.tail == nodeP) // Only ONE item in list?
      container->value.tail = NULL;
  }
  else if (nodeP == container->value.tail) // Hit in end of list
  {
    container->value.tail = prevP;
    prevP->next          = NULL;
  }
  else  // Hit in the middle of the list
  {
    prevP->next = nodeP->next;
  }

  nodeP->next = NULL;

  return nodeP;
}



// -----------------------------------------------------------------------------
//
// corTreeChildAddSorted - sorted insert of a node in a container
//
// PARAMETERS
//   container - pointer to the father (container) of the child to be added
//   child     - pointer to the child to be added
//
// NOTE
//   The `name` of the children in the container is used as criteria for the sort.
//
void corTreeChildAddSorted(CorNode* container, CorNode* child)
{
  //
  // First child to be added?
  //
  if (container->value.head == NULL)
  {
    container->value.head = child;
    container->value.tail        = child;
    child->next                  = NULL;

    return;
  }

  // Find where to insert the new node
  CorNode* current     = container->value.head;
  CorNode* prev        = NULL;
  char    childName0  = child->name[0];

  while (current != NULL)
  {
    char currentName0 = current->name[0];

    if (childName0 > currentName0)
    {
      // Still not there - must continue
      prev    = current;
      current = current->next;
      continue;
    }

    if (childName0 < currentName0)
    {
      // Got it - insert child between prev and current
      break;
    }

    if (strcmp(child->name, current->name) > 0)  // child->name GT current->name - must continue
    {
      // Still not there - must continue
      prev    = current;
      current = current->next;
      continue;
    }

    //
    // child->name LE current->name => insert child between prev and current
    //
    break;
  }

  //
  // Three cases:
  //
  //   01. If prev == NULL, then child is LT the containers first child and should be prepended
  //       to the list
  //
  //   02. If current == NULL, then child is to be appended to the list
  //
  //   03. Else, both prev and current points to nodes. child to be inserted between prev and current
  //
  //   Note that 'prev == NULL' and 'current == NULL' is not possible as the empty list has been
  //   taken care of already
  //
  if (prev == NULL)
  {
    child->next                  = current;
    container->value.head = child;
  }
  else if (current == NULL)
  {
    container->value.tail->next = child;
    container->value.tail      = child;
    child->next                = NULL;
  }
  else
  {
    child->next = current;
    prev->next  = child;
  }
}



// -----------------------------------------------------------------------------
//
// corTreeChildAddSortedReverse - reverse sorted insert of a node in a container
//
// PARAMETERS
//   container - pointer to the father (container) of the child to be added
//   child     - pointer to the child to be added
//
// NOTE
//   The `name` of the children in the container is used as criteria for the sort.
//
void corTreeChildAddSortedReverse(CorNode* container, CorNode* child)
{
  //
  // First child to be added?
  //
  if (container->value.head == NULL)
  {
    container->value.head          = child;
    container->value.tail          = child;
    child->next                    = NULL;

    return;
  }

  // Find where to insert the new node
  CorNode* current     = container->value.head;
  CorNode* prev        = NULL;
  char    childName0  = child->name[0];

  while (current != NULL)
  {
    char currentName0 = current->name[0];

    if (childName0 < currentName0)
    {
      // Still not there - must continue
      prev    = current;
      current = current->next;
      continue;
    }

    if (childName0 > currentName0)
    {
      // Got it - insert child between prev and current
      break;
    }

    if (strcmp(child->name, current->name) < 0)  // child->name LT current->name - must continue
    {
      // Still not there - must continue
      prev    = current;
      current = current->next;
      continue;
    }

    //
    // child->name LE current->name => insert child between prev and current
    //
    break;
  }

  //
  // Three cases:
  //
  //   01. If prev == NULL, then child is GT the containers first child and should be prepended
  //       to the list
  //
  //   02. If current == NULL, then child is to be appended to the list
  //
  //   03. Else, both prev and current points to nodes. child to be inserted between prev and current
  //
  //   Note that 'prev == NULL' and 'current == NULL' is not possible as the empty list has been
  //   taken care of already
  //
  if (prev == NULL)
  {
    child->next                  = current;
    container->value.head = child;
  }
  else if (current == NULL)
  {
    container->value.tail->next = child;
    container->value.tail      = child;
    child->next                = NULL;
  }
  else
  {
    child->next = current;
    prev->next  = child;
  }
}
