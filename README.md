# corTree — the Tree, and Nothing Else

The in-memory tree every other part of the coraine stack works on: the node, and
the functions that build, edit, search, clone and sort a tree of them. It has no
parser and no renderer. JSON in and out is **corJson**; any other format — the
binary one the `cor://` protocol and corDB persistence will use, XML, YAML — is
a sibling of corJson that produces or consumes the same tree.

- **Version:** 0.1.0
- **Language:** C
- **License:** [Apache License 2.0](LICENSE)

The only dependencies are **corAlloc, corLog and corBase**.

## Where it comes from

corTree and corJson are the two halves of **kjson**, which was one library that
held both the representation and the JSON parser. That was the design mistake
the split corrects: with the tree on its own, every format is just another
producer or consumer of it. kjson itself is untouched and keeps serving its own
users; this is a copy of its implementation under new names, not a fork.

The one change of substance is the **allocator**. kjson's builders took the
parser's handle, `Kjson*`, and used nothing of it but the allocator inside —
which made the tree depend on the parser. corTree's builders take the allocator
itself:

```c
CorNode* corTreeString(CorAlloc* kaP, const char* name, const char* value);
```

`NULL` means the tree heap (`corTreeHeap.h`): the trees that live on, freed node by node with
`corTreeFree`. A node, its name and a string value are one block there, as with `malloc`, but
8-byte aligned and without `malloc`'s header - blocks of one size per 16 KiB page, and a free finds
the size by the page, not in the node. `COR_TREE_HEAP=malloc`, and a build with AddressSanitizer,
use `malloc` instead (valgrind and ASan see a block's bounds only in `malloc`'s).

## The node

```c
typedef struct CorNode
{
  char*            name;       // NULL/"" for an array item
  struct CorNode*  next;       // next sibling
  CorValueType     type;       // CorString, CorInt, CorFloat, CorBoolean, CorNull, CorObject, CorArray
  unsigned char    flags;      // for the users of the library - opaque to corTree
  unsigned char    kind;       // what an object is, for the users of the library - opaque to corTree
  uint16_t         termId;     // for the users of the library (NGSI-LD: which core term, 0 = none) - opaque to corTree
  CorValue         value;      // b, i, f, s, or - for a container - head and tail. LAST
} CorNode;
```

40 bytes on a 64-bit machine - `flags`, `kind` and `termId` share 8 bytes with `type`, and
`CorNode.h` asserts the size and the offsets. Every builder zeroes them (and sets `next` to NULL), and `corTreeClone`
copies them. Any other code that builds nodes (a parser, a database reader) must do the same.

A prefix layout: `value` is last, and its last 8 bytes - `value.tail` - are a container's only.
`corTreeClone(NULL, ...)` - the clone into the tree heap - allocates a node that is not a container
without them: 32 bytes. Such a node is never made a container in place, and never copied whole
(`*a = *b`) - `corTreeNodeCopy` copies the bytes it has. Every other builder allocates 40.

The layout has a number, `corNodeLayout` (2), which every object built with `CorNode.h` carries;
it also requires `corNodeLayout2`, defined by this corTree. A plugin built against another layout
than its broker is refused either way, instead of reading every node at the wrong offsets - see
coraine's doc/plugin-architecture.md, "The CorNode layout".

## API

| | |
|---|---|
| build | `corTreeObject`, `corTreeArray`, `corTreeString`, `corTreeInteger`, `corTreeFloat`, `corTreeBoolean`, `corTreeNull` |
| edit | `corTreeChildAdd`, `corTreeChildAddSorted`, `corTreeChildAddSortedReverse`, `corTreeChildPrepend`, `corTreeChildRemove`, `corTreeChildMove`, `corTreeChildReplace`, `corTreeChildAddOrReplace`, `corTreeNodeDecouple` |
| find | `corTreeLookup`, `corTreeLookupWithStrcmp`, `corTreeNavigate`, `corTreeNavigate2`, `corTreeStringSiblingFind`, `corTreeStringValueLookupInArray`, `corTreeChildCount` |
| copy, free | `corTreeClone`, `corTreeCloneMarked`, `corTreeNodeCopy`, `corTreeFree` |
| sort | `corTreeSort`, `corTreeArraySort`, `corTreeNodeCompare`, `corTreeStringArraySort`, `corTreeStringArraySortedInsert` |
| tree heap | `corTreeHeapAlloc`, `corTreeHeapFree`, `corTreeHeapBytes` |
| binary form | `corTreeBinTablesInit`, `corTreeBinTablesPreload`, `corTreeBinTablesRelease`, `corTreeBinEncode`, `corTreeBinDecode` |
| describe | `corTreeValueType`, `corTreeValue` |

`corTreeChildAdd` sets the child's `next` to NULL: a child still in another list is removed from
it first (`corTreeChildRemove`), or that list ends at the child. `corTreeChildAddOrReplace` puts
the new node in the old child's place - the old node is not overwritten - and returns the old one,
out of the tree, for the caller to free (NULL if the node was added).

## Build

A sibling repo of the rest of the stack: `-I..` and `../corTree/libcorTree.a`.

```sh
make di
```
