#include "./hmap.h"
#include<stdint.h>
#include<stdlib.h>
#include<string.h>


static void hmap_rehash(hmap* m) {
	list_resize(m->hash_buckets);
	for(int i = 0; i < list_get_capacity(m->hash_buckets) / 2; i++) {
		llist* l = (llist*) list_get(m->hash_buckets, i);
		if (l != NULL) {
			list_set(m->hash_buckets, i, NULL);
			lnode* p = l->head;
			while (p != NULL) {
				hentry* h = (hentry*) p->data;
				hmap_put(m, h->key, h->keylen, h->data);
				free(h->key);
				free(h);
				p = p->next;
			}
			llist_destroy(l);
		}
	}
}

uint64_t fnv1_1a_hash(char* key, uint64_t keylen, int tablesize) {
	uint32_t hash = 2166136261u;
	for(int i = 0; i < keylen; i++) {
		hash ^= (uint8_t) key[i];
		hash *= 16777619;
	}
	return hash % tablesize;
}

uint64_t simple_hash(char* key, uint64_t keylen, int tablesize) {
	int h = 0;
	for(int i = 0; i < keylen; i++) h += key[i];
	return h % tablesize;
}

hentry* hentry_create(void* data, char* key, int keylen) {
	hentry* h = (hentry*) malloc(sizeof(hentry));

	h->data = data;
	h->key = (char*) malloc(sizeof(char) * keylen);
	memcpy(h->key, key, keylen);
	h->keylen = keylen;

	return h;
}

hmap* hmap_create(hashfunc hf) {
	hmap* h = (hmap*) malloc(sizeof(hmap));
	h->hf = hf;
	h->hash_buckets = list_create();
	h->size = 0;
	h->capacity = h->hash_buckets->capacity;

	return h;
}

void hmap_put(hmap* m, char* key, int keylen, void* data) {
	hentry* t;
	if (( t = hmap_get_entry(m, key, keylen)) != NULL){
		t->data = data;
		return;
	}

	uint64_t index = m->hf(key, keylen, list_get_capacity(m->hash_buckets));
	if (index > list_get_capacity(m->hash_buckets)) hmap_rehash(m);

	hentry* h = hentry_create(data, key, keylen);
	llist* l;
	if (list_get(m->hash_buckets, index) == NULL) {
		l = llist_create();
		list_set(m->hash_buckets, index, (void*) l);
	}
	else l = (llist*) list_get(m->hash_buckets, index);
	llist_push_back(l, (void*) h);
	m->capacity = m->hash_buckets->capacity;
	m->size++;
}

void* hmap_get(hmap* m, char* key, int keylen) {

	uint64_t index = m->hf(key, keylen, m->capacity);
	llist* l = (llist*) list_get(m->hash_buckets, index);
	if (l == NULL) return NULL;

	else {
		lnode* p = l->head;

		while (p != NULL) {
			hentry* h = (hentry*) p->data;
			if (hmap_compare_keys(key, keylen, h->key, h->keylen) == 0) return h->data;
			p = p->next;
		}
	}
	return NULL;
}

hentry* hmap_get_entry(hmap* m, char* key, int keylen) {
	uint64_t index = m->hf(key, keylen, m->capacity);
	llist* l = (llist*) list_get(m->hash_buckets, index);
	if (l == NULL) return NULL;
	else {
		lnode* p = l->head;
		while (p != NULL) {
			hentry* h = (hentry*) p->data;
			if (hmap_compare_keys(key, keylen, h->key, h->keylen) == 0) return h;
			p = p->next;
		}
	}
	return NULL;
}

int hmap_compare_keys(char* key1, int key1len, char* key2, int key2len)
{
    if (key1len != key2len) return -1;
	else {
		for(int i = 0; i < key1len; i++) {
			if (key1[i] != key2[i]) return -1;
		}
	}
	return 0;
}


void hmap_destroy(hmap* m) {
	
	for(int i = 0; i < list_get_capacity(m->hash_buckets); i++) {
		llist* cur = (llist*) list_get(m->hash_buckets, i);
		if (cur) llist_destroy(cur);
	}
	free(m);
}