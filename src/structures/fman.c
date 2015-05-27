#include <stdarg.h>
#include "structures.h"

#include "../tweet.h"

#include "fman.h"

void ffields_release(void *o);
void fman_release(void *o);

FileFields *
ffields_create(size_t count, ...)
{
    va_list ap;
    int j;
    FileFields *ff = (FileFields *)alloc(sizeof(FileFields), ffields_release);
    ff->fields = (FieldType *)malloc(sizeof(FieldType) * count);
    ff->offsets = (size_t *)malloc(sizeof(size_t) * count);
    ff->fieldc = count;
    va_start(ap, count); //Requires the last fixed parameter (to get the address)
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
    fman->ff = ff;
    retain(ff);
    return fman;
}

void
fman_add_entry(FileManager *fman, void *o)
{
    tweet_print(o);
    Tweet *t = o;
    printf("pos:%p\n", &(t->language));
    for (int i = 0; i < fman->ff->fieldc; i++)
    {
        void *entry = o + fman->ff->offsets[i];
        printf("entry:%p\n", entry);

        switch(fman->ff->fields[i])
        {
            case str_f:
            {
                puts((*(String **)(entry))->string);
            }
            break;
            case int_f:
            {
                printf("%d\n", (*(int *)entry));
            }
            break;
            case uint_f:
            {
                printf("%u\n", (*(unsigned int *)entry));
            }
            break;
            case long_f:
            {
                printf("%ld\n", (*(long *)entry));
            }
            break;
            case ulong_f:
            {
                printf("%ld\n", (*(long *)entry));
            }
            break;
            case float_f:
            {
                printf("%f\n", (*(float *)entry));
            }
            break;
            case double_f:
            {
                printf("%lf\n", (*(double *)entry));
            }
            case char_f:
            {
                printf("%c\n", (*(char *)entry));
            }
            break;
            case uchar_f:
            {
                printf("%c\n", (*(unsigned char *)entry));
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
