#include <stdarg.h>

#include "structures.h"

#include "fman.h"

void ffields_release(void *o);

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
    for(j=0; j<count * 3; j++)
    {
        if ((j % 3) == 0)
        {
            ff->fields[j/3] = va_arg(ap, FieldType);
        }
        else if ((j % 3) == 1)
        {
            ff->offsets[j/3] = va_arg(ap, size_t);
        }
        else if ((j % 3) == 2)
        {
            if (va_arg(ap, int));
        }
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


size_t
ftype_size_of(FieldType ft)
{
    size_t field_size = 0;
    switch (ft)
    {
        case str_f:    field_size = sizeof(char);          break;
        case int_f:    field_size = sizeof(int);           break;
        case uint_f:   field_size = sizeof(unsigned int);  break;
        case long_f:   field_size = sizeof(long);          break;
        case ulong_f:  field_size = sizeof(unsigned long); break;
        case float_f:  field_size = sizeof(float);         break;
        case double_f: field_size = sizeof(double);        break;
        case char_f:   field_size = sizeof(char);          break;
        case uchar_f:  field_size = sizeof(unsigned char); break;
    }
    return field_size;
}

size_t
ffields_size(FileFields *ff)
{
    size_t size = 0;
    for (int i = 0; i < ff->fieldc; ++i)
    {
        size += ftype_size_of(ff->offsets[i]);
    }
    return size;
}

void
ffields_release(void *o)
{
    FileFields *ff = o;
    free(ff->fields);
    free(ff->offsets);
}
