#include<stdlib.h>
#include<stdio.h>

#include "tky_llist.h"

typedef struct tky_lnode {
    void*         data;
    tky_lnode*    next;
    tky_lnode*    prev;
} tky_lnode;

tky_lnode* lnode_create(void* data) {
    tky_lnode* n = calloc(1, sizeof(tky_lnode));
    n->data = data;
    n->next = NULL;
    n->prev = NULL;
    return n;
}

tky_lnode* lnode_next(tky_lnode* n) {
    return n->next;
}
tky_lnode* lnode_prev(tky_lnode* n) {
    return n->prev;
}
void* lnode_data(tky_lnode* n) {
    return n->data;
}

void lnode_free(tky_lnode* n) {
    free(n);
}

typedef struct tky_llist {
  size_t        size;
  tky_lnode*    head;
  tky_lnode*    tail;
} tky_llist;

int llist_empty(tky_llist* l) {
    return l->size == 0;
}

tky_lnode* llist_get_head(tky_llist* l) {
    if (l == NULL) return NULL;
    return l->head;
}
tky_lnode* llist_get_tail(tky_llist* l) {
    if (l == NULL) return NULL;
    return l->tail;
}

tky_llist* llist_init() {
  tky_llist* l = (tky_llist*)malloc(sizeof(tky_llist));
  l->size = 0;
  l->head = l->tail = NULL;
  return l;
}

void llist_push_back(tky_llist* l, void* data) {
    if (l == NULL) return;
    tky_lnode* new_tail = lnode_create(data);
    if (llist_empty(l)) {
        l->head = l->tail = new_tail;
    }
    else {
        new_tail->prev = l->tail;
        l->tail->next = new_tail;
        l->tail = new_tail;
    }
    l->size++;
}

void llist_push_front(tky_llist* l, void* data) {
    tky_lnode* new_head = lnode_create(data);
    if (llist_empty(l)) l->head = l->tail = new_head;
    else {
        new_head->next = l->head;
        l->head->prev = new_head;
        l->head = new_head;
    }
    l->size++;
}

void llist_remove_back(tky_llist* l) {
   if (llist_empty(l)) return;
   else {
        tky_lnode* p = l->tail;
        l->tail = l->tail->prev;
        if (l->tail != NULL) {
            l->tail->next = NULL;
        }
        lnode_free(p);
   }
   l->size--;
}

void* llist_pop_back(tky_llist* l) {
    void* d = NULL;
    if (llist_empty(l)) return d;
    else {
        tky_lnode* p = l->tail;
        d = p->data;

        l->tail = l->tail->prev;
        if (l->tail != NULL) {
            l->tail->next = NULL;
        }
        lnode_free(p);
    }
    l->size--;
    return d;
}

void llist_remove_front(tky_llist* l) {
    if (llist_empty(l)) return;
    else {
        tky_lnode* p = l->head;
        l->head = l->head->next;
        if (l->head != NULL){
            l->head->prev = NULL;
        }
        lnode_free(p);
    }
    l->size--;
}

void* llist_pop_front(tky_llist* l) {
    void* d = NULL;
    if (llist_empty(l)) return d;
    else {
        tky_lnode* p = l->head;
        d = p->data;
        l->head = l->head->next;
        if (l->head != NULL){
            l->head->prev = NULL;
        }
        lnode_free(p);
    }
    l->size--;
    return d;
}

void* llist_get(tky_llist* l, int idx) { 
    if (idx >= l->size || idx < 0) return NULL;
    int i = 0;
    tky_lnode* p = l->head;
    while (i < idx) {
        p = p->next;
        i++;
    }
    return p->data;
}
void llist_remove_idx(tky_llist* l, int idx) {
    if (llist_empty(l)) return;
    if (idx >= l->size || idx < 0) return;
    if (idx == 0) llist_remove_front(l);
    else if (idx == l->size - 1) llist_remove_back(l);
    else {
        tky_lnode* p = l->head;
        int i = 0;
        while (i < idx) {
            p = p->next;
            i++;
        }
        tky_lnode* prev_node = p->prev;
        tky_lnode* next_node = p->next;
        prev_node->next = next_node;
        next_node->prev = prev_node;
        lnode_free(p);
    }
    l->size--;
}

void* llist_pop_idx(tky_llist* l, int idx) {
    if (llist_empty(l)) return NULL;
    if (idx >= l->size || idx < 0) return NULL;
    if (idx == 0) return llist_pop_front(l);
    else if (idx == l->size - 1) return llist_pop_back(l);
    else {
        tky_lnode* p = l->head;
        int i = 0;
        while (i < idx) {
            p = p->next;
            i++;
        }
        tky_lnode* prev_node = p->prev;
        tky_lnode* next_node = p->next;
        prev_node->next = next_node;
        next_node->prev = prev_node;
        void* data = p->data;
        lnode_free(p);
        l->size--;
        return data;
    }
}

void llist_display(tky_llist* l) {
    tky_lnode* p = l->head;
    while (p != NULL) {
        printf("%p ", p->data);
        p = p->next;
    }
    printf("\n");
}

size_t llist_size(tky_llist* l) {
    return l->size;
}

void llist_destroy(tky_llist* l) {
    tky_lnode* p = l->head;
    while (p != NULL) {
       tky_lnode* next = p->next;
       lnode_free(p);
       p = next;
    }
    free(l);
}
