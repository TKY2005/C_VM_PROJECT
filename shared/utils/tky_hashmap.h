#ifndef HASHMAP_H
#define HASHMAP_H

#include<stdint.h>
#include<stddef.h>

#define HASHMAP_DEFAULT_LOAD_FACTOR 0.75
#define HASHMAP_INIT_SIZE 32

typedef uint64_t (*hfunction)(char*,size_t);
typedef struct tky_hashmap tky_hashmap;
typedef struct tky_hentry tky_hentry;

uint64_t hf_fnv1a(char* key, size_t keylen);

/*
    hashmap structure
        hashmap(hashfunction)
        |   bucket1
        |   |   entry1.1
        |   |   entry1.2
        |   bucket2
        |   |   entry2.1
        |   |   entry2.2
*/

tky_hentry*     hentry_create(char* key, size_t keylen, void* data);
void            hentry_free(tky_hentry* h);

/*initializes the hashmap and uses the passed hash function to hash the keys*/
tky_hashmap*    hashmap_init(hfunction h);
/*places an element in hashmap by calculating the bucket index by hashing the 'key' and places the element in the bucket*/
void            hashmap_put(tky_hashmap* m, char* key, size_t keylen, void* data);
/*calculates the index of the bucket using the given 'key' and if the element is found at that bucket it returns it otherwise NULL*/
void*           hashmap_get(tky_hashmap* m, char* key, size_t keylen);
/*Removes the key-value pair specified by the key*/
void            hashmap_remove(tky_hashmap* m, char* key, size_t keylen);
/*returns the number of elements present in the hashmap*/
size_t          hashmap_size(tky_hashmap* m);
/*returns the current number of entries that can be present in the hashmap before it needs to expand*/
size_t          hashmap_load_threshold(tky_hashmap* m);
/*returns the number of times the hashmap had to expand and recalculate the indices for its entries*/
size_t          hashmap_rehash_count(tky_hashmap* m);


void            hashmap_free(tky_hashmap* m);

#endif