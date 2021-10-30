#include "bdhash.h"
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "bdhash_key.h"
#include <stdio.h>


// add all unallocated to free list, does not update other meta
void add_extend_to_free(jthash_t *hash, jthash_extend_t *store)
{
    jthash_node_t *next = store->store;
    for(int i = 0; i < (sizeof store->store/sizeof store->store[0]); i++)
    {
        if(!next->next) // un allocated
        {
            if(hash->free_list)
            {
                next->next = hash->free_list; // new node point to old head
            }
            else // new free list
            {
                next->next = next; // self terminate
            }
            hash->free_list = next; // update head
        }
        next++;
    }
}

// completely add a store to the hash
void add_extend_to_hash(jthash_t *hash, jthash_extend_t *store)
{
    if(!hash->last_storage) hash->last_storage = &hash->storage;

    hash->last_storage->next_extended = store; // add to list
    hash->last_storage = store;                // save list end
    hash->extends++;
    add_extend_to_free(hash, store);
}

// allocate and add one extended store to the hash if enabled
jthash_ret_t add_extended(jthash_t *hash)
{
    if(!hash->do_not_extend)
    {
        jthash_extend_t *new_ext = calloc(1, sizeof(jthash_extend_t));
        if(!new_ext) return (jthash_ret_t){.err=JthashMemoryError};
        add_extend_to_hash(hash, new_ext);
        return (jthash_ret_t){.err=JthashOk, .value=hash->extends};
    }
    return (jthash_ret_t){.err=JthashMemoryError};
}

void add_to_free(jthash_t *hash, jthash_node_t *node)
{
    if(hash->free_list) // link back at head
    {
        node->next = hash->free_list;
        hash->free_list = node;
    }
    else  // link back at head with termination
    {
        node->next = node;
        hash->free_list = node;
    }
}

// just walk any self terminated list counting entries
uint32_t count_list(jthash_node_t *first)
{
    jthash_node_t *next = first;
    uint32_t cnt=0;    
    while(next)
    {
        cnt++;
        if(next->next == next) break;
        next = next->next;
    }
    return cnt;    
}

// count the free list
uint32_t count_free(jthash_t const *hash)
{
    return count_list(hash->free_list);
}

// count each bin list and all
uint32_t count_hash(jthash_t const *hash)
{
    uint32_t cnt = 0;
    for(int h=0; h < sizeof hash->storage.store/sizeof hash->storage.store[0]; h++)
    {
        uint32_t hcnt = count_list(hash->hash_table[h]);
        cnt += hcnt;
        printf("h%u->%u,",h, hcnt);
    }
    printf("\n");
    return cnt;
}


// the do_not_extend prevents future malloc calls
jthash_t *jthash_init(jthash_t *hash, DoNotExtend_t do_not_extend)
{
    if(!hash) // if not provided, allocate one
    {
        hash = malloc(sizeof(jthash_t));
        if(!hash) return NULL;
    }

    // zero out structure
    *hash = (jthash_t){.do_not_extend = do_not_extend};
    add_extend_to_free(hash, &hash->storage);
    return hash;
}

jthash_ret_t jthash_clear(jthash_t *hash, FreeExtParm_t free_extended)
{
    uint32_t extends = hash->extends;  // number of jthash_extend_t blocks allocated
    jthash_extend_t *first_extended = hash->storage.next_extended;
    jthash_extend_t *last_storage = hash->last_storage;
    // if last storage, last link should be null
    if(last_storage && last_storage->next_extended) // internal error
    {
        return (jthash_ret_t){.err=JthashError, .value=3};
    }
    // now clear all the hash bins and local store preserving the do_not_extend
    jthash_init(hash, hash->do_not_extend);

    //now clear up extended memory if any
    jthash_extend_t *next_extended = first_extended;
    uint32_t found = 0; // validate existing counter
    // now walk extended list to clear or free
    while(next_extended)
    {
        // grab next in chain before modifying current
        jthash_extend_t *next = next_extended->next_extended;
    
        if(++found > extends) // internal error
        {
            return (jthash_ret_t){.err=JthashError, .value=6};
        }

        if(free_extended)
        {
            next_extended->next_extended = NULL; // break link just in case
            free(next_extended);
        }
        else // keep the block but clear it and then relink
        {
            // clear the actual store and relink
            *next_extended = (jthash_extend_t){.next_extended=next};
            add_extend_to_hash(hash, next_extended);
        }
        next_extended = next; // move to next store
    }
    if(found != extends) // internal error
    {
        return (jthash_ret_t){.err=JthashError, .value=2};
    }

    // return current capicity
    return (jthash_ret_t){.value = JT_HASH_STORAGE * (hash->extends + 1), .err=JthashOk};
}

jthash_node_t *jtfind_key(jthash_t *hash, jtkey_t const *key, jthash_node_t ***prev)
{
    jthash_node_t *next = hash->hash_table[key->hash];
    if(prev) *prev = &(hash->hash_table[key->hash]);
    while(next)
    {
        if(compare_jtkey(&next->key, key))
        {
            return next;
        }
        if(prev) *prev = &next->next;
        if(next == next->next) break; // node self ref is terminator
        next = next->next;
    }
    return NULL; // not found
}


jthash_ret_t jthash_set(jthash_t *hash, jtkey_t const *key, jtval_t const *value)
{
    // find existing key if it exists
    jthash_node_t **link_prev;
    jthash_node_t *node = jtfind_key(hash, key, &link_prev);
    if(node) // if already exists just update value
    {
        node->value = *value;
    }
    else // else store new node
    {
        if(hash->free_list) // if available
        {
            node = hash->free_list; // grab from list
            if(node->next == node) // if last in list
            {
                hash->free_list = NULL; // list is empty
            }
            else
            {
                hash->free_list = node->next;
            }
        }
        else 
        {
            if(add_extended(hash).err)
            {
                return (jthash_ret_t){.err = JthashMemoryError};
            }
            node = hash->free_list;
            hash->free_list = hash->free_list->next;
        }
        node->key = *key;
        node->value = *value;
        node->next = node;  // self ref is list terminator
        *link_prev = node;  // hook into hash list
        hash->items++;
        return (jthash_ret_t){.value = (uintptr_t)node, .err=JthashNewKey};
    }
    return (jthash_ret_t){.value = (uintptr_t)node};
}


jtval_t jthash_update(jthash_t *hash, jtkey_t const *key, jtval_t const *value)
{
    // find existing key if it exists
    jthash_node_t *node = jtfind_key(hash, key, NULL);
    jtval_t old_val;
    if(node) // if already exists just update value
    {
        old_val = node->value;
        node->value = *value;
    }
    else
    {
        return (jtval_t){.len = JthashKeyNotFound};
    }
    return (jtval_t){.val = old_val.val, .len = old_val.len};
}


jtval_t jthash_get(jthash_t *hash, jtkey_t const *key)
{
    // find existing key if it exists
    jthash_node_t *node = jtfind_key(hash, key, NULL);

    if(node) // if exists return
    {
        return (jtval_t){.val = node->value.val , .len = node->value.len};
    }
    else
    {
        return (jtval_t){.len = JthashKeyNotFound};
    }
}

jtval_t jthash_pop(jthash_t *hash, jtkey_t const *key)
{
    jthash_node_t **link_prev;
    // find existing key if it exists
    jthash_node_t *node = jtfind_key(hash, key, &link_prev);
    jtval_t old_val;
    if(node) // node exists so remove it
    {
        old_val = node->value; // save value
        if(node == node->next)  // last node in list
        {
            *link_prev = NULL;          // clear list
        }
        else
        {
            *link_prev = node->next;    // remove from active list
        }
        add_to_free(hash, node);
        hash->items--;
    }
    else
    {
        return (jtval_t){.len = JthashKeyNotFound};
    }
    return (jtval_t){.val = old_val.val, .len = old_val.len};
}

