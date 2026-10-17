#include "tky_ntree.h"

#include<stdlib.h>
#include<stddef.h>

typedef struct tky_ntnode {
    void*            data;
    tky_ntnode*      left_child;
    tky_ntnode*      right_sibling;
} tky_ntnode;

typedef struct tky_ntree {
    tky_ntnode*      root;
    size_t           size;
} tky_ntree;

tky_ntree* ntree_init() {

    tky_ntree* t = (tky_ntree*) malloc(sizeof(tky_ntree));
    t->root = NULL;
    t->size = 0;

    return t;
}