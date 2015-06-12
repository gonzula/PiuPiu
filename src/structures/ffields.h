#ifndef FFIELDS_H_INCLUDED
#define FFIELDS_H_INCLUDED

#include <stdlib.h>

typedef enum
{
    str_f,
    int_f,
    uint_f,
    long_f,
    ulong_f,
    float_f,
    double_f,
    char_f,
    uchar_f,
} FieldType;

typedef struct
{
    FieldType *fields; // list of field types
    size_t *offsets;   // offsets of the fields
    size_t fieldc;     // fields count
    int *indexes;
    size_t idxc;
} FileFields;

FileFields *ffields_create(size_t count, ...);
void ffields_print(FileFields *ff);

size_t ftype_size_of(FieldType ft);
size_t ffields_size(FileFields *ff);


#endif
