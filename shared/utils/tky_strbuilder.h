#ifndef STRBUILDER_H
#define STRBUILDER_H

#define SB_INIT_SIZE 500

#include<stdarg.h>
#include<stddef.h>

typedef struct tky_strbuilder tky_strbuilder;

tky_strbuilder*     strbuilder_init();
void                strbuilder_append(tky_strbuilder* s, const char* t);
void                strbuilder_appendf(tky_strbuilder* s, const char* t, ...);
void                strbuilder_appendc(tky_strbuilder* s, const char c);
void                strbuilder_fill(tky_strbuilder* s, const char* t, int fill_amount);

char*               strbuilder_getstr(tky_strbuilder* s);

void                strbuilder_reset(tky_strbuilder* s);

size_t              strbuilder_size(tky_strbuilder* s);
size_t              strbuilder_capacity(tky_strbuilder* s);

void                strbuilder_free(tky_strbuilder* s);


#endif
