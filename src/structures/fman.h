#ifndef FMAN_H_INCLUDED
#define FMAN_H_INCLUDED

#include <stdio.h>

#include "ffields.h"

typedef struct
{
    FileFields *ff;
    FILE *fp;
    size_t entryc; //entry count
    String *db_name;
    Vector *indexes;
} FileManager;

#include "fidx.h"



FileManager *fman_create(char *fname, FileFields *ff);
void fman_add_entry(FileManager *fman, void *o);

int fman_entry_at_offset(FileManager *fman, long int offset, void *entry /* inout */); //returns 0 if the entry was deleted
Vector *fman_list_all(FileManager *fman);
Vector *fman_search_by_field(FileManager *fman,
    int field_idx, const void *value);

void fman_remove_entry_at_offset(FileManager *fman, long int offset);

Vector *fman_match_offsets(Vector *v1, Vector *v2);
Vector *fman_merge_offsets(Vector *v1, Vector *v2);

#endif
