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
    // FILE *indexf;
    // FILE *invlstf;
    Vector *index;
} FieldIndex;

// typedef enum
// {
//     str_f,
//     int_f,
//     uint_f,
//     long_f,
//     ulong_f,
//     float_f,
//     double_f,
//     char_f,
//     uchar_f,
// } FieldType;


#endif
