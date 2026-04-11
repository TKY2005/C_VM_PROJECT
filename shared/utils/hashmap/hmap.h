#ifndef HMAP_H
#define HMAP_H

#include "../singly_linked_list/llist.h"
#include "../dynamic_list/dynamic_list.h"

#include<stdint.h>

typedef uint64_t (*hashfunc) (char*, uint64_t, int);

uint64_t fnv1_1a_hash(char* key, uint64_t keylen, int tablesize);
uint64_t simple_hash(char* key, uint64_t keylen, int tablesize);

/* structure
	hashmap
	|
	|hash buckets list
	|	| linked list 1
	|	|	| hentry 1
	|	|	| hentry 2
	|	| linked list 2
	|	|	| hentry 1
	|	|	| hentry 2
	|
*/

typedef struct {
	void* data;
	char* key;
	int keylen;
} hentry;

hentry* hentry_create(void* data, char* key, int keylen);

typedef struct {
	hashfunc hf;
	dynamic_list* hash_buckets;
	int size;
	int capacity;
} hmap;

hmap* hmap_create(hashfunc hf);

void hmap_put(hmap* m, char* key, int keylen, void* data);
void* hmap_get(hmap* m, char* key, int keylen);

hentry* hmap_get_entry(hmap* m, char* key, int keylen);

void hmap_destroy(hmap* m);

int hmap_compare_keys(char* key1, int key1len, char* key2, int key2len);

#endif