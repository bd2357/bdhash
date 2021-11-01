#ifndef BDHASH_NODE_H
#define BDHASH_NODE_H

#include <stdint.h>
#include <stddef.h>
#include "bdhash.h"
#include "bdhash_key.h"
#include "bdhash_value.h"

typedef struct bdhash_node_t
{
    struct bdhash_node_t *next; // next in chain
    bdkey_t key;              // actual key or pointer to key
    bdval_t value;            // actual value or pointer to item
} bdhash_node_t;

// storage for actual key/values or their references
typedef struct bdhash_extend_t
{
    struct bdhash_extend_t *next_extended;
    bdhash_node_t store[BD_HASH_STORAGE];
} bdhash_extend_t;

typedef struct bdhash_t
{
    bdhash_node_t *hash_table[BD_HASH_SIZE]; // the first level table
    bdhash_extend_t storage;                 // the first storage area
    bdhash_extend_t *last_storage;           // pnt to last added storage
    bdhash_node_t *free_list;  // pointer to next known free storage node
    uint32_t items;   // count of stored items
    uint32_t extends; // number of bdhash_extend_t blocks allocated
    uint8_t do_not_extend; // prevent malloc call
} bdhash_t;
// hash is full when either
#define HASH_FULL(hashP) \
  ((hashP)->free_list == NULL || \
   (((hashP)->extends+1)*BD_HASH_STORAGE)==(hashP)->items)


#endif // BDHASH_NODE_H
