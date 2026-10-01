#ifndef CORTREE_BIN_H_
#define CORTREE_BIN_H_

//
// FILE            corTreeBin.h
//
// AUTHOR          Ken Zangelin
//
// Copyright 2026 Seamware
// SPDX-License-Identifier: Apache-2.0
//
// The cor format - a CorNode tree, serialised. See coraine's doc/cor-protocol.md.
//
// The codec knows nothing about NGSI-LD. What it cannot know - which names and values are core
// terms, which objects are attributes - comes through CorBinCodec's callbacks, all optional: with
// none at all it serialises any JSON tree, losslessly.
//
// A node on the wire, little-endian throughout:
//
//   tag     1 byte    bits 0-2: value type (CorValueType)
//                     bits 3-4: name mode - none (array element), core term id, new string, string-table index
//                     bits 5-7: per value type - an integer 0-6 itself, a float's width, a boolean's value,
//                               a string value that is a core-term id, an object's kind 0-6
//   [kind]  1 byte    objects of kind 7-15 only (0 = a plain object), from CorBinCodec.objectKind
//   name              per name mode
//   value             per value type; an object or array: its byte length (varint), then its children
//
// Strings keep their NUL, so a decoded tree's names and values point into the buffer it was decoded
// from: the buffer must outlive the tree. Only an IRI split into namespace + local name is joined
// into a new string.
//
#include <stdbool.h>                              // bool
#include <stdint.h>                               // uint8_t, uint16_t

#include "corAlloc/CorAlloc.h"                    // CorAlloc
#include "corTree/CorNode.h"                      // CorNode



// -----------------------------------------------------------------------------
//
// CorBinCodec - what the codec cannot know: core terms, and kinds of object
//
// Every member may be NULL.
//
typedef struct CorBinCodec
{
  //
  // nameTermId - the core-term id of a node's NAME, 0 when it is not one.
  // NULL: names travel as strings (node->termId is still used when it is set).
  //
  uint16_t     (*nameTermId)(CorNode* nodeP);

  //
  // valueTermId - the core-term id of a string VALUE, 0 when it is not one
  //
  uint16_t     (*valueTermId)(const char* value);

  //
  // termName - the name of a core-term id, on decode; NULL for an id it does not know
  //
  const char*  (*termName)(uint16_t termId);

  //
  // objectKind - the kind of an object (1-15; 0 = a plain object), on encode.
  // *foldedP gets the child the kind replaces - written nowhere, it is the kind - or NULL.
  //
  uint8_t      (*objectKind)(CorNode* objectP, CorNode** foldedP);

  //
  // objectKindExpand - give a decoded object back what its kind replaced (e.g. its 'type' member)
  //
  void         (*objectKindExpand)(CorNode* objectP, uint8_t kind, CorAlloc* kaP);
} CorBinCodec;



// -----------------------------------------------------------------------------
//
// CorBinTable - strings numbered by first appearance, for one direction of one stream
//
// Two per stream: namespaces (the part of an IRI up to its last '/', '#' or ':') and names. On a
// cor:// connection they live as long as the connection; in a file, as long as a block. The writer
// and the reader each keep one, and they stay identical because both add an entry at the same point
// of the stream. The strings are the table's own copies.
//
typedef struct CorBinTable
{
  char**    stringV;      // by index
  int       count;
  int       max;          // the cap - when reached, nothing more is added and strings go inline
  int*      hashV;        // open addressing: index + 1, 0 = empty - the writer's lookup
  int       hashSize;
} CorBinTable;

typedef struct CorBinTables
{
  CorBinTable  namespaces;
  CorBinTable  names;
} CorBinTables;



// -----------------------------------------------------------------------------
//
// CorBinBuffer - an encoder's output, grown with realloc
//
typedef struct CorBinBuffer
{
  char*  buf;
  int    len;
  int    size;
} CorBinBuffer;



// -----------------------------------------------------------------------------
//
// corTreeBinTablesInit - empty tables, with their caps; false if out of memory
//
// The fixed NGSI-LD namespaces are NOT added here - the codec knows nothing about NGSI-LD. A user
// adds them with corTreeBinTablesPreload, before the first message, on both sides.
//
extern bool corTreeBinTablesInit(CorBinTables* tablesP, int namespacesMax, int namesMax);



// -----------------------------------------------------------------------------
//
// corTreeBinTablesPreload - fixed first entries of the namespace table (both sides, same order)
//
extern bool corTreeBinTablesPreload(CorBinTables* tablesP, const char** namespaceV, int count);



// -----------------------------------------------------------------------------
//
// corTreeBinTablesRelease - free the tables' strings and arrays
//
extern void corTreeBinTablesRelease(CorBinTables* tablesP);



// -----------------------------------------------------------------------------
//
// corTreeBinEncode - append a tree to a buffer
//
// tablesP: NULL for no tables at all (every string inline). Returns false only when out of memory.
//
extern bool corTreeBinEncode(CorNode* treeP, const CorBinCodec* codecP, CorBinTables* tablesP, CorBinBuffer* outP);



// -----------------------------------------------------------------------------
//
// corTreeBinDecode - a tree, out of a buffer
//
// Allocates the nodes from kaP (NULL: malloc); names and values point into buf, which must outlive
// the tree. tablesP must be the reader's mirror of the tables the writer used (or NULL if it used
// none). NULL on any malformed input, with *errorP saying what - the decoder never reads past
// buf + len.
//
extern CorNode* corTreeBinDecode(const char* buf, int len, const CorBinCodec* codecP, CorBinTables* tablesP, CorAlloc* kaP, const char** errorP);

#endif  // CORTREE_BIN_H_
