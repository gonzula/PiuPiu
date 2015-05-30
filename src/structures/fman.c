#include <stdarg.h>
#include <string.h>
#include <float.h>
#include "structures.h"

#include "fman.h"

#define ABS(a) \
   ({ __typeof__ (a) _a = (a);\
     _a < 0 ? -_a : _a; })

void ffields_release(void *o);
void fman_release(void *o);

size_t
ftype_size_of(FieldType ft)
{
    size_t field_size = 0;
    switch (ft)
    {
        case str_f:    field_size = sizeof(String *);       break;
        case int_f:    field_size = sizeof(int);            break;
        case uint_f:   field_size = sizeof(unsigned int);   break;
        case long_f:   field_size = sizeof(long);           break;
        case ulong_f:  field_size = sizeof(unsigned long);  break;
        case float_f:  field_size = sizeof(float);          break;
        case double_f: field_size = sizeof(double);         break;
        case char_f:   field_size = sizeof(char);           break;
        case uchar_f:  field_size = sizeof(unsigned char);  break;
    }
    return field_size;
}

FileFields *
ffields_create(size_t count, ...)
{
    va_list ap;
    int j;
    FileFields *ff = (FileFields *)alloc(sizeof(FileFields), ffields_release);
    ff->fields = (FieldType *)malloc(sizeof(FieldType) * count);
    ff->offsets = (size_t *)malloc(sizeof(size_t) * count);
    ff->fieldc = count;
    va_start(ap, count);
    for(j=0; j<count * 2; j++)
    {
        if (!(j % 2))
            ff->fields[j/2] = va_arg(ap, FieldType);
        else
            ff->offsets[j/2] = va_arg(ap, size_t);
    }
    va_end(ap);
    return ff;
}

void
ffields_print(FileFields *ff)
{
    for (int i = 0; i < ff->fieldc; ++i)
    {
        switch(ff->fields[i])
        {
            case str_f:    printf("str_f @ 0x%zx",    ff->offsets[i]); break;
            case int_f:    printf("int_f @ 0x%zx",    ff->offsets[i]); break;
            case uint_f:   printf("uint_f @ 0x%zx",   ff->offsets[i]); break;
            case long_f:   printf("long_f @ 0x%zx",   ff->offsets[i]); break;
            case ulong_f:  printf("ulong_f @ 0x%zx",  ff->offsets[i]); break;
            case float_f:  printf("float_f @ 0x%zx",  ff->offsets[i]); break;
            case double_f: printf("double_f @ 0x%zx", ff->offsets[i]); break;
            case char_f:   printf("char_f @ 0x%zx",   ff->offsets[i]); break;
            case uchar_f:  printf("uchar_f @ 0x%zx",  ff->offsets[i]); break;
        }
        if (i < ff->fieldc - 1)printf(", ");
        else printf("\n");
    }
}

FileManager *
fman_create(char *fname, FileFields *ff)
{
    FileManager *fman = alloc(sizeof(FileManager), fman_release);
    fman->fp = fopen(fname, "rb+");
    if (!fman->fp) // if doesn't exists
    {
        fman->fp = fopen(fname, "w"); //create
        long int stack_top = -1;
        fwrite(&stack_top, sizeof(stack_top), 1, fman->fp); // write top of stack
        fclose(fman->fp);
        fman->fp = fopen(fname, "rb+"); //and reopen in update mode
    }
    fman->ff = ff;
    retain(ff);
    return fman;
}

long int
fman_free_offset_for_size(FileManager *fman, size_t size)
{
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

        // switch(fman->ff->fields[j])
        // {
        //     case str_f:
        //     {
        //         puts((*(String **)(entry))->string);
        //         entry_size += sizeof(char) * ((*(String **)(entry))->len + 1);
        //     }
        //     break;
        //     case int_f:
        //     {
        //         printf("%d\n", (*(int *)entry));
        //         entry_size += sizeof(int);
        //     }
        //     break;
        //     case uint_f:
        //     {
        //         printf("%u\n", (*(unsigned int *)entry));
        //         entry_size += sizeof(unsigned int);
        //     }
        //     break;
        //     case long_f:
        //     {
        //         printf("%ld\n", (*(long *)entry));
        //         entry_size += sizeof(long);
        //     }
        //     break;
        //     case ulong_f:
        //     {
        //         printf("%ld\n", (*(unsigned long *)entry));
        //         entry_size += sizeof(unsigned long);
        //     }
        //     break;
        //     case float_f:
        //     {
        //         printf("%f\n", (*(float *)entry));
        //         entry_size += sizeof(float);
        //     }
        //     break;
        //     case double_f:
        //     {
        //         printf("%lf\n", (*(double *)entry));
        //         entry_size += sizeof(double);
        //     }
        //     break;
        //     case char_f:
        //     {
        //         printf("%c\n", (*(char *)entry));
        //         entry_size += sizeof(char);
        //     }
        //     break;
        //     case uchar_f:
        //     {
        //         printf("%c\n", (*(unsigned char *)entry));
        //         entry_size += sizeof(unsigned char);
        //     }
        //     break;
        // }
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
}

void
fman_release(void *o)
{
    FileManager *fman = o;
    release(fman->ff);
    fclose(fman->fp);
}

void
ffields_release(void *o)
{
    FileFields *ff = o;
    free(ff->fields);
}
