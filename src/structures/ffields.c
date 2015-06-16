#include <stdarg.h>

#include "structures.h"

#include "fman.h"
#include "ffields.h"

void ffields_release(void *o);

FileFields *
ffields_create(size_t count, ...)   // cria o FileFields
{
    va_list ap;
    int j;
    FileFields *ff = (FileFields *)alloc(sizeof(FileFields), ffields_release);
    ff->fields = (FieldType *)malloc(sizeof(FieldType) * count);
    ff->offsets = (size_t *)malloc(sizeof(size_t) * count);
    ff->indexes = (int *)malloc(sizeof(int) * count);
    ff->fieldc = count;

    ff->idxc = 0;
    va_start(ap, count);

    for(j = 0; j < count * 3; j++)
    {
        if ((j % 3) == 0)
        {
            ff->fields[j/3] = va_arg(ap, FieldType); //salva o tipo
        }
        else if ((j % 3) == 1)
        {
            ff->offsets[j/3] = va_arg(ap, size_t); //salva o offset
        }
        else if ((j % 3) == 2)
        {
            if (va_arg(ap, int)) // se usuário pediu indice
            {
                ff->indexes[ff->idxc] = j/3;
                ff->idxc++; // incrementa numero de indices
            }
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
ftype_size_of(FieldType ft) //retorna o tamanho de um campo
{
    switch (ft)
    {
        case str_f:    return sizeof(char);
        case int_f:    return sizeof(int);
        case uint_f:   return sizeof(unsigned int);
        case long_f:   return sizeof(long);
        case ulong_f:  return sizeof(unsigned long);
        case float_f:  return sizeof(float);
        case double_f: return sizeof(double);
        case char_f:   return sizeof(char);
        case uchar_f:  return sizeof(unsigned char);
    }
    return 0;
}

size_t // retorna o tamanho de conjunto de campos
ffields_size(FileFields *ff, size_t str_size)
{                                   // ↑ o tamanho que deve ser considerado quando o campo é string
    size_t size = 0;
    for (int i = 0; i < ff->fieldc; ++i)
    {
        if (ff->fields[i] == str_f)
        {
            size += str_size;
        }
        else
        {
            size += ftype_size_of(ff->fields[i]);
        }
    }
    return size;
}

void
ffields_release(void *o)
{
    FileFields *ff = o;
    free(ff->fields);
    free(ff->offsets);
    free(ff->indexes);
}
