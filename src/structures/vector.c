#include <stdlib.h>

#include "vector.h"
#include "refcnt.h"

#define BUFFER_SIZE (1 << 8)


void vector_free(void *);

Vector *
vector_init()
{
    Vector * v = alloc(sizeof(Vector), vector_free);
    v->bufferSize = BUFFER_SIZE;
    v->objs = malloc(sizeof(void *) * v->bufferSize);
    v->count = 0;
    return v;
}

void
vector_append(Vector *v, void *obj)
{
    if (!v)return;
    if (v->count + 1 > v->bufferSize)
    {
        v->bufferSize += BUFFER_SIZE;
        v->objs = realloc(v->objs, sizeof(void *) * v->bufferSize);
    }
    v->objs[v->count] = obj;
    retain(obj);
    v->count++;
}

void
vector_remove(Vector *v, int idx)
{
    release(v->objs[idx]);
    for (int i = idx + 1; i < v->count; i++)
    {
        v->objs[i - 1] = v->objs[i];
    }
    v->count--;
}

void vector_free(void * o)
{
    Vector *v = (Vector *)o;
    for (int i = 0; i < v->count; i++)
    {
        release(v->objs[i]);
    }
    free(v->objs);
}

void vector_sort(Vector *v, int (*compar)(const void*,const void*))
{
    if (!v)return;
    qsort(v->objs, v->count, sizeof(void *), compar);
}
