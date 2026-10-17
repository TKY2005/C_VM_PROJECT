#include<stdio.h>
#include<stdlib.h>
#include<stddef.h>
#include<stdint.h>
#include<stdbool.h>
#include<string.h>

#include "tky_hashmap.h"
#include "tky_dlist.h"
#include "tky_llist.h"

uint64_t hf_fnv1a(char* key, size_t keylen) {
    uint64_t hash = 0xCBF29CE484222325;
    for(size_t i = 0; i < keylen; i++) {
        hash ^= key[i];
        hash *= 0x100000001B3;
    }
    return hash;
}

typedef struct tky_hentry {
    char*   key;
    size_t  keylen;
    void*   data;
} tky_hentry;

tky_hentry* hentry_create(char* key, size_t keylen, void* data) {
    tky_hentry* h = (tky_hentry*) malloc(sizeof(tky_hentry));
    h->key = (char*) malloc(sizeof(char) * keylen);
    memcpy(h->key, key, keylen);
    h->keylen = keylen;
    h->data = data;
    return h;
}
void hentry_free(tky_hentry* h) {
    free(h->key);
    free(h);
}

tky_hentry* hentry_copy(const tky_hentry* src) {
    tky_hentry* new_entry = (tky_hentry*) malloc(sizeof(tky_hentry));
    new_entry->key = (char*) malloc(sizeof(char) * src->keylen);
    new_entry->keylen = src->keylen;
    memcpy(new_entry->key, src->key, src->keylen);
    new_entry->data = src->data;
    return new_entry;
}

typedef struct tky_hashmap {
    size_t          size;
    size_t          rehash_counter;
    hfunction       hf;
    tky_dlist*      buckets;
} tky_hashmap;

static size_t hashmap_calculate_index(tky_hashmap* m, char* key, size_t keylen, size_t bucket_len) {
    uint64_t hashcode = m->hf(key, keylen);
    return hashcode % bucket_len;
}

static void hashmap_rehash(tky_hashmap* m) {
    size_t new_capacity = dlist_capacity(m->buckets) * 2;
    size_t old_capacity = dlist_capacity(m->buckets);

    tky_dlist* new_buckets = dlist_init(&new_capacity);
    dlist_match_capacity(new_buckets);
    for(size_t i = 0; i < new_capacity; i++) {
        dlist_set(new_buckets, i, llist_init());
    }

    for(size_t i = 0; i < old_capacity; i++) {
        tky_llist* l;
        if ((l = (tky_llist*) dlist_get(m->buckets, i)) != NULL) {
            tky_hentry* entry;
            tky_lnode* p = llist_get_head(l);
            while (p != NULL) {
                entry = (tky_hentry*) lnode_data(p);

                uint64_t new_index = hashmap_calculate_index(m, entry->key, entry->keylen, new_capacity);
                tky_llist* new_list = (tky_llist*) dlist_get(new_buckets, new_index);
                
                llist_push_back(new_list, (void*) entry);

                p = lnode_next(p);
            }
            llist_destroy(l);
        }
    }

    dlist_free(m->buckets);
    m->buckets = new_buckets;
    m->rehash_counter++;
}

static bool compare_keys(char* k1, size_t l1, char* k2, size_t l2) {
    if (l1 != l2) return false;
    for(size_t i = 0; i < l1; i++) if (k1[i] != k2[i]) return false;
    return true;
}

tky_hashmap* hashmap_init(hfunction h) {
    tky_hashmap* m = (tky_hashmap*) malloc(sizeof(tky_hashmap));
    m->size = 0;
    m->rehash_counter = 0;
    m->hf = h;
    m->buckets = dlist_init(&(size_t){HASHMAP_INIT_SIZE});
    size_t capacity = dlist_capacity(m->buckets);
    dlist_match_capacity(m->buckets);
    for(size_t i = 0; i < capacity; i++){
        dlist_set(m->buckets, i, llist_init());
    }
    return m;
}

static tky_hentry* hashmap_get_entry(tky_hashmap* m, char* key, size_t keylen) {

    uint64_t index = hashmap_calculate_index(m, key, keylen, dlist_capacity(m->buckets));
    tky_llist* l = (tky_llist*) dlist_get(m->buckets, index);

    tky_lnode* p = llist_get_head(l);
    tky_hentry* h;
    while (p != NULL) {
        h = (tky_hentry*) lnode_data(p);
        if (compare_keys(key, keylen, h->key, h->keylen)) return h;
        p = lnode_next(p);
    }
    return NULL;
}

static tky_hentry* hashmap_get_entry_precalc(tky_hashmap* m, char* key, size_t keylen, uint64_t index) {

    tky_llist* l = (tky_llist*) dlist_get(m->buckets, index);

    tky_lnode* p = llist_get_head(l);
    tky_hentry* h;
    while (p != NULL) {
        h = (tky_hentry*) lnode_data(p);
        if (compare_keys(key, keylen, h->key, h->keylen)) return h;
        p = lnode_next(p);
    }
    return NULL;
}

void hashmap_put(tky_hashmap* m, char* key, size_t keylen, void* data) {
    tky_hentry* h;
    uint64_t index = hashmap_calculate_index(m, key, keylen, dlist_capacity(m->buckets));
    if ((h = hashmap_get_entry_precalc(m, key, keylen, index)) != NULL) {
        h->data = data;
        return;
    }
    h = hentry_create(key, keylen, data);
    
    tky_llist* l = (tky_llist*) dlist_get(m->buckets, index);
    llist_push_back(l, (void*) h);
    m->size++;

    if (m->size > hashmap_load_threshold(m)) {
        hashmap_rehash(m);
    } 
}

void* hashmap_get(tky_hashmap* m, char* key, size_t keylen) {
    tky_hentry* h = hashmap_get_entry(m, key, keylen);
    if (h == NULL) return NULL;
    return h->data;
}

void hashmap_remove(tky_hashmap* m, char* key, size_t keylen) {
    uint64_t index = hashmap_calculate_index(m, key, keylen, dlist_capacity(m->buckets));
    tky_llist* l = dlist_get(m->buckets, index);

    tky_lnode* p = llist_get_head(l);
    int idx = 0;
    while (p != NULL) {
        tky_hentry* h = (tky_hentry*) lnode_data(p);
        if (compare_keys(key, keylen, h->key, h->keylen)){
            llist_remove_idx(l, idx);
            hentry_free(h);
            m->size--;
            return;
        }
        idx++;
        p = lnode_next(p);
    }
}

size_t hashmap_size(tky_hashmap* m) {
    return m->size;
}

size_t hashmap_load_threshold(tky_hashmap* m) {
    return dlist_capacity(m->buckets) * HASHMAP_DEFAULT_LOAD_FACTOR;
}

size_t hashmap_rehash_count(tky_hashmap* m) {
    return m->rehash_counter;
}

void hashmap_free(tky_hashmap* m) {
    for(size_t i = 0; i < dlist_capacity(m->buckets); i++) {
        tky_llist* l;
        if ((l = (tky_llist*) dlist_get(m->buckets, i)) != NULL) {
            tky_lnode* p = llist_get_head(l);
            while (p != NULL) {
                hentry_free((tky_hentry*) lnode_data(p));
                p = lnode_next(p);
            }
            llist_destroy(l);
        }
    }
    dlist_free(m->buckets);
    free(m);
}
