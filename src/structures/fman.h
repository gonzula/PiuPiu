#ifndef FMAN_H_INCLUDED
#define FMAN_H_INCLUDED

#include <stdio.h>
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
    size_t *offsets; // offsets of the fields
    size_t fieldc; // fields count
} FileFields;

typedef struct
{
    FileFields *ff;
    FILE *fp;
} FileManager;

FileFields *ffields_create(size_t count, ...);
void ffields_print(FileFields *ff);
FileManager *fman_create(char *fname, FileFields *ff);
void fman_add_entry(FileManager *fman, void *o);

void fman_entry_at_offset(FileManager *fman, long int offset, void *entry /* inout */);
Vector *fman_list_all(FileManager *fman);
Vector *fman_search_by_field(FileManager *fman,
    int field_idx, const void *value);

#endif
