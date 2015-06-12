#include "structures.h"
#include "fman.h"

#include <stdio.h>
#include <stdlib.h>

void fidx_release(void *o);
void idx_release(void *o);

FieldIndex *
fidx_create(FieldType ftype, String *db_name, int field_idx)
{
    FieldIndex *fidx = alloc(sizeof(FieldIndex), fidx_release);
    fidx->ftype = ftype;
    fidx->field_idx = field_idx;
    fidx->index = NULL;
    fidx->db_name = db_name;
    retain(db_name);
    return fidx;
}

IndexEntry *
idx_create(FieldType ftype, void *data, long int offset)
{
    IndexEntry *idx = alloc(sizeof(IndexEntry), idx_release);
    idx->ftype = ftype;
    idx->offset = offset;
    switch(ftype)
    {
        case str_f:    idx->str =  (String *)data;          break;
        case int_f:    idx->i   = *((int *)data);           break;
        case uint_f:   idx->ui  = *((unsigned int *)data);  break;
        case long_f:   idx->l   = *((long *)data);          break;
        case ulong_f:  idx->ul  = *((unsigned long *)data); break;
        case float_f:  idx->f   = *((float *)data);         break;
        case double_f: idx->lf  = *((double *)data);        break;
        case char_f:   idx->c   = *((char *)data);          break;
        case uchar_f:  idx->uc  = *((unsigned char *)data); break;
    }
    if (ftype == str_f)
    {
        retain(idx->str);
    }
    return idx;
}

void
fidx_create_index(FieldIndex *fidx)
{
    printf("creating index part 1\n");
    if (fidx->index) //index already exists
        return;
    printf("creating index part 2\n");
    fidx->index = vector_init();
    String *f1_name = str_create(fidx->db_name->string);
    str_append_char(f1_name, '.');
    str_append_char(f1_name, fidx->field_idx+'0');
    str_append(f1_name, ".idx");
    FILE *f1 = fopen(f1_name->string, "rb");
    release(f1_name);

    String *f2_name = str_create(fidx->db_name->string);
    str_append_char(f2_name, '.');
    str_append_char(f2_name, fidx->field_idx+'0');
    str_append(f2_name, ".lst");
    FILE *f2 = fopen(f2_name->string, "rb");
    release(f2_name);

    if (!f1)
    {
        if (f2)
        {
            fclose(f2);
            remove(f2_name->string);
        }
        return;
    }
    if (!f2)
    {
        if (f1)
        {
            fclose(f1);
            remove(f1_name->string);
        }
        return;
    }

    Vector *index = fidx->index;
    size_t field_size = ftype_size_of(fidx->ftype);
    while(!feof(f1))
    {
        if (fidx->ftype == str_f)
        {
            String *str = str_from_file(f1, "");
            long int next_entry = -1;
            fread(&next_entry, sizeof(next_entry), 1, f1);
            while (next_entry != -1)
            {
                long int offset = -1;
                fread(&offset, sizeof(offset), 1, f2);

                IndexEntry *entry = idx_create(fidx->ftype, str, offset);
                vector_append(index, entry);
                release(entry);
                fread(&next_entry, sizeof(next_entry), 1, f2);
            }
            release(str);
        }
        else
        {
            char this_value[30];
            fread(this_value, field_size, 1, f1);
            long int next_entry = -1;
            fread(&next_entry, sizeof(next_entry), 1, f1);
            while (next_entry != -1)
            {
                long int offset = -1;
                fread(&offset, sizeof(offset), 1, f2);

                IndexEntry *entry = idx_create(fidx->ftype, &this_value, offset);
                vector_append(index, entry);
                release(entry);
                fread(&next_entry, sizeof(next_entry), 1, f2);
            }
        }
    }
    fclose (f1);
    fclose (f2);
}

int
idx_cmp(IndexEntry* e1, IndexEntry* e2)
{
    if (e1->ftype != e2->ftype)return 0;
    int result = 0;
    switch(e1->ftype)
    {
        case str_f:
        {
            result = str_cmp(e1->str, e2->str);
        }
        break;
        case int_f:
        {
            if (e1->i < e2->i)
                result = -1;
            else if (e1->i > e2->i)
                result = 1;
            else
                result = 0;
        }
        break;
        case uint_f:
        {
            if (e1->ui < e2->ui)
                result = -1;
            else if (e1->ui > e2->ui)
                result = 1;
            else
                result = 0;
        }
        break;
        case long_f:
        {
            if (e1->l < e2->l)
                result = -1;
            else if (e1->l > e2->l)
                result = 1;
            else
                result = 0;
        }
        break;
        case ulong_f:
        {
            if (e1->ul < e2->ul)
                result = -1;
            else if (e1->ul > e2->ul)
                result = 1;
            else
                result = 0;
        }
        break;
        case float_f:
        {
            if (e1->f < e2->f)
                result = -1;
            else if (e1->f > e2->f)
                result = 1;
            else
                result = 0;
        }
        break;
        case double_f:
        {
            if (e1->lf < e2->lf)
                result = -1;
            else if (e1->lf > e2->lf)
                result = 1;
            else
                result = 0;
        }
        break;
        case char_f:
        {
            if (e1->c < e2->c)
                result = -1;
            else if (e1->c > e2->c)
                result = 1;
            else
                result = 0;
        }
        break;
        case uchar_f:
        {
            if (e1->uc < e2->uc)
                result = -1;
            else if (e1->uc > e2->uc)
                result = 1;
            else
                result = 0;
        }
        break;
    }
    return result;
}

int
_idx_entry_sort(const void *v1, const void *v2)
{
    IndexEntry **p1 = (IndexEntry **)v1;
    IndexEntry **p2 = (IndexEntry **)v2;
    IndexEntry *e1 = *p1;
    IndexEntry *e2 = *p2;

    int result = idx_cmp(e1, e2);
    if (!result)
    {
        if (e1->offset < e2->offset)
            return -1;
        if (e1->offset > e2->offset)
            return 1;
        return 0;
    }
    return result;
}

void
fidx_sort(FieldIndex *fidx)
{
    vector_sort(fidx->index, _idx_entry_sort);
}

void
fidx_write_file(FieldIndex *fidx)
{
    if (!fidx->index)
        return;

    String *f1_name = str_create(fidx->db_name->string);
    str_append_char(f1_name, '.');
    str_append_char(f1_name, fidx->field_idx+'0');
    str_append(f1_name, ".idx");
    FILE *f1 = fopen(f1_name->string, "wb");
    release(f1_name);

    String *f2_name = str_create(fidx->db_name->string);
    str_append_char(f2_name, '.');
    str_append_char(f2_name, fidx->field_idx+'0');
    str_append(f2_name, ".lst");
    FILE *f2 = fopen(f2_name->string, "wb");
    release(f2_name);

    Vector *index = fidx->index;
    for (int i = 0; i < index->count; i++)
    {
        IndexEntry *e = index->objs[i];
        if (e->ftype == str_f)
        {
            fwrite(e->str->string, sizeof(char), e->str->len + 1, f1);
            long int last_offset = -1;
            for (; i < index->count; i++)
            {
                IndexEntry *newE = index->objs[i];
                if (idx_cmp(e, newE))
                {
                    i--;
                    break;
                }
                fwrite(&newE->offset, sizeof(long int), 1, f2);
                fwrite(&last_offset, sizeof(last_offset), 1, f2);
                last_offset = ftell(f2) - sizeof(long int) - sizeof(last_offset);
            }
            fwrite(&last_offset, sizeof(last_offset), 1, f2);
        }
    }
    fclose (f1);
    fclose (f2);
}

void
idx_release(void *o)
{
    IndexEntry *idx = o;
    if (idx->ftype == str_f)
        release(idx->str);
}

void
fidx_release(void *o)
{
    FieldIndex *fidx = o;
    release(fidx->index);
    release(fidx->db_name);
}
