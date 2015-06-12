#ifndef FIDX_H_INCLUDED
#define FIDX_H_INCLUDED

#include <stdlib.h>
#include <stdio.h>

typedef struct
{
    FieldType ftype;
    union
    {
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
    String *db_name;
    int field_idx;
    Vector *index;
} FieldIndex;

FieldIndex *fidx_create(FieldType ftype, String *db_name, int field_idx);
void fidx_create_index(FieldIndex *fidx);

#endif
