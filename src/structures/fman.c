#include <string.h>
#include <float.h>
#include "structures.h"

#include "fman.h"

#define ABS(a) \
   ({ __typeof__ (a) _a = (a);\
     _a < 0 ? -_a : _a; })

void fman_release(void *o);
void fman_inc_entryc(FileManager *fman, int amnt);

FileManager *
fman_create(char *fname, FileFields *ff)
{
    String *bin_fname = str_create(fname);
    str_append(bin_fname, ".bin");

    FileManager *fman = alloc(sizeof(FileManager), fman_release);
    fman->fp = fopen(bin_fname->string, "rb+");
    if (!fman->fp) // if doesn't exists
    {
        fman->fp = fopen(bin_fname->string, "w"); //create
        long int stack_top = -1;
        fwrite(&stack_top, sizeof(stack_top), 1, fman->fp); // write top of stack
        fclose(fman->fp);
        fman->fp = fopen(bin_fname->string, "rb+"); //and reopen in update mode
    }
    fman->ff = ff;
    retain(ff);

    fman->entryc = 0;
    Vector *offsets = fman_list_all(fman);
    fman_inc_entryc(fman, offsets->count);
    release(offsets);

    release(bin_fname);

    fman->db_name = str_create(fname);

    fman->indexes = vector_init();
    for (int i = 0; i < fman->ff->idxc; i++)
    {
        FieldIndex *fidx = fidx_create(fman->ff->fields[i], fman->db_name, i);
        vector_append(fman->indexes, fman->indexes);
        release(fidx);
    }
    return fman;
}

void
fman_inc_entryc(FileManager *fman, int amnt)
{
    fman->entryc += amnt;
    if (fman->entryc >= 10)
    {
        for (int i = 0; i < fman->indexes->count; i++)
        {
            // fidx_create_index(fman->indexes->objs[i]);
        }
    }
}

long int
fman_free_offset_for_size(FileManager *fman, int size)
{
    //TODO: implement best fit
    fseek(fman->fp, 0, SEEK_SET);
    long int next_entry = -1;
    fread(&next_entry, sizeof(next_entry), 1, fman->fp);
    long int best_fit = 0;
    size_t best_fit_size = 0xFFFF;
    size_t min_entry_size = ffields_size(fman->ff);
    while(next_entry != -1 && feof(fman->fp))
    {
        fseek(fman->fp, next_entry, SEEK_SET);
        int this_entry_size;
        fread(&this_entry_size, sizeof(this_entry_size), 1, fman->fp);
        this_entry_size = -this_entry_size;
        if (this_entry_size <= 0)
        {
            break;
        }
        else if (this_entry_size == size) //best-fit
        {
            best_fit = ftell(fman->fp);
            break;
        }
        else if (this_entry_size - min_entry_size > 0 &&
                 this_entry_size - min_entry_size < best_fit_size)
        {
            best_fit_size = this_entry_size - min_entry_size;
            best_fit = ftell(fman->fp);
        }
    }
    if (best_fit)
        return best_fit;
    fseek(fman->fp, 0, SEEK_END);
    return ftell(fman->fp);
}

Vector *
fman_list_all(FileManager *fman)
{
    Vector *offset_vector = vector_init();
    fseek(fman->fp, sizeof(size_t), SEEK_SET); //skip stack pointer
    while(!feof(fman->fp))
    {
        int this_entry_size;
        size_t elements_read = fread(&this_entry_size, sizeof(this_entry_size), 1, fman->fp);
        if (!elements_read)break; //feof
        long int this_entry_offset = ftell(fman->fp);
        if (this_entry_size < 0) //deleted entry
        {
            this_entry_size = -this_entry_size;
            fseek(fman->fp, this_entry_size, SEEK_CUR);
        }
        else
        {
            long int *entry_offset = alloc(sizeof(long int), NULL);
            *entry_offset = this_entry_offset;
            vector_append(offset_vector, entry_offset);
            release(entry_offset);
            fseek(fman->fp,
                this_entry_offset + this_entry_size,
                SEEK_SET);
        }
    }
    return offset_vector;
}

void
fman_entry_at_offset(FileManager *fman, long int offset, void *entry /* inout */)
{
    fseek(fman->fp, offset, SEEK_SET);
    for (int i = 0; i < fman->ff->fieldc; i++)
    {
        void *field = entry + fman->ff->offsets[i];
        if (fman->ff->fields[i] == str_f)
        {
            String *str = str_from_file(fman->fp, "");
            memcpy(field, &str, sizeof(String *));
        }
        else
        {
            size_t field_size = ftype_size_of(fman->ff->fields[i]);
            char this_value[30];
            fread(this_value, field_size, 1, fman->fp);
            memcpy(field, this_value, field_size);
        }
    }
}

Vector *
fman_search_by_field(FileManager *fman,
    int field_idx, const void *value)
{
    Vector *offset_vector = vector_init();
    if (1) // TODO: test if idx file exists
    {
        fseek(fman->fp, sizeof(size_t), SEEK_SET); //skip stack pointer
        // long int entry_offset = -1;
        while(!feof(fman->fp))
        {
            int this_entry_size;
            size_t elements_read = fread(&this_entry_size, sizeof(this_entry_size), 1, fman->fp);
            if (!elements_read)break; //feof
            long int this_entry_offset = ftell(fman->fp);
            if (this_entry_size < 0) //deleted entry
            {
                this_entry_size = -this_entry_size;
                fseek(fman->fp, this_entry_size, SEEK_CUR);
                continue; // next entry…
            }
            for (int i = 0; i < fman->ff->fieldc; i++)
            {
                if (fman->ff->fields[i] == str_f)
                {
                    String *str = str_from_file(fman->fp, "");
                    if (i == field_idx)
                    {
                        if (str_eq(str, (String *)value))
                        {
                            // entry_offset = this_entry_offset;
                            long int *entry_offset = alloc(sizeof(long int), NULL);
                            *entry_offset = this_entry_offset;
                            vector_append(offset_vector, entry_offset);
                            release(entry_offset);
                        }
                        fseek(fman->fp,
                            this_entry_offset + this_entry_size,
                            SEEK_SET);
                        release(str);
                        break;
                    }
                    release(str);
                }
                else
                {
                    size_t field_size = ftype_size_of(fman->ff->fields[i]);
                    if (i == field_idx)
                    {
                        char this_value[30];
                        fread(this_value, field_size, 1, fman->fp);
                        if (fman->ff->fields[i] == float_f)
                        {
                            float a = *(float *)this_value;
                            float b = *(float *)value;
                            if (ABS(a - b) < FLT_EPSILON) // you can't simply compare byte per byte when it comes to float and double
                            {
                                long int *entry_offset = alloc(sizeof(long int), NULL);
                                *entry_offset = this_entry_offset;
                                vector_append(offset_vector, entry_offset);
                                release(entry_offset);
                            }
                        }
                        else if (fman->ff->fields[i] == double_f)
                        {
                            double a = *(double *)this_value;
                            double b = *(double *)value;
                            if (ABS(a - b) < DBL_EPSILON)
                            {
                                long int *entry_offset = alloc(sizeof(long int), NULL);
                                *entry_offset = this_entry_offset;
                                vector_append(offset_vector, entry_offset);
                                release(entry_offset);
                            }
                        }
                        else
                        {
                            if(memcmp(this_value, value, field_size) == 0)
                            {
                                // entry_offset = this_entry_offset;
                                long int *entry_offset = alloc(sizeof(long int), NULL);
                                *entry_offset = this_entry_offset;
                                vector_append(offset_vector, entry_offset);
                                release(entry_offset);
                            }
                        }
                        fseek(fman->fp,
                            this_entry_offset + this_entry_size,
                            SEEK_SET);
                        break;
                    }
                    else
                    {
                        fseek(fman->fp, field_size, SEEK_CUR);
                    }
                }
            }
        }
        return offset_vector;
    }
    return NULL;
}

void
fman_add_entry(FileManager *fman, void *o)
{
    int entry_size = 0;
    for (int j = 0; j < fman->ff->fieldc; j++)
    {
        void *entry = o + fman->ff->offsets[j];
        if (fman->ff->fields[j] == str_f)
        {
            entry_size += sizeof(char) * ((*(String **)(entry))->len + 1);
        }
        else
        {
            entry_size += ftype_size_of(fman->ff->fields[j]);
        }
    }
    long int offset = 0;
    offset = fman_free_offset_for_size(
        fman,
        entry_size + sizeof(entry_size) /* entry_size itself */);
    fseek(fman->fp,
    offset,
    SEEK_SET);
    fwrite(&entry_size, sizeof(entry_size), 1, fman->fp);
    for (int j = 0; j < fman->ff->fieldc; j++)
    {
        void *entry = o + fman->ff->offsets[j];
        switch(fman->ff->fields[j])
        {
            case str_f:
            {
                String *str = (*(String **)(entry));
                fwrite(str->string, sizeof(char), str->len + 1, fman->fp);
            }
            break;
            case int_f:
            {
                int i = (*(int *)entry);
                fwrite(&i, sizeof(int), 1, fman->fp);
            }
            break;
            case uint_f:
            {
                unsigned int u = (*(unsigned int *)entry);
                fwrite(&u, sizeof(unsigned int), 1, fman->fp);
            }
            break;
            case long_f:
            {
                long l = (*(long *)entry);
                fwrite(&l, sizeof(long), 1, fman->fp);
            }
            break;
            case ulong_f:
            {
                unsigned long ul = (*(unsigned long *)entry);
                fwrite(&ul, sizeof(unsigned long), 1, fman->fp);
            }
            break;
            case float_f:
            {
                float f = (*(float *)entry);
                fwrite(&f, sizeof(float), 1, fman->fp);
            }
            break;
            case double_f:
            {
                double lf = (*(double *)entry);
                fwrite(&lf, sizeof(double), 1, fman->fp);
            }
            break;
            case char_f:
            {
                char c = (*(char *)entry);
                fwrite(&c, sizeof(char), 1, fman->fp);
            }
            break;
            case uchar_f:
            {
                unsigned char c = (*(unsigned char *)entry);
                fwrite(&c, sizeof(unsigned char), 1, fman->fp);
            }
            break;
        }
    }
    fman_inc_entryc(fman, 1);
}

void
fman_release(void *o)
{
    FileManager *fman = o;
    release(fman->ff);
    release(fman->db_name);
    release(fman->indexes);
    fclose(fman->fp);
}
