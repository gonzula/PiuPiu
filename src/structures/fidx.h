#ifndef FIDX_H_INCLUDED
#define FIDX_H_INCLUDED

#include <stdlib.h>
#include <stdio.h>
#include "structures.h"
#include "fman.h"



typedef struct
{
    FieldType ftype;
    union
    {
        void *v;
        String *str;
        int i;
        unsigned int ui;
        long l;
        unsigned long ul;
        float f;
        double lf;
        char c;
        unsigned char uc;
    };
    long int offset;
}IndexEntry;

typedef struct
{
    FieldType ftype;
    int field_idx;
    Vector *index;
    FileManager *fman;
} FieldIndex;

IndexEntry *idx_create(FieldType ftype, const void *data, long int offset);

FieldIndex *fidx_create(FileManager *fman, FieldType ftype, int field_idx);
void fidx_create_index(FieldIndex *fidx);

void fidx_sort(FieldIndex *fidx);

Vector *fidx_search(FieldIndex *fidx, const void *value);

#endif
