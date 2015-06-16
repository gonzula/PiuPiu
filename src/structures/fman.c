#include <string.h>
#include <float.h>
#include "structures.h"

#include "fman.h"

#define ABS(a) \
   ({ __typeof__ (a) _a = (a);\
     _a < 0 ? -_a : _a; })

void fman_release(void *o);
void fman_inc_entryc(FileManager *fman, int amnt);

FileManager *                             //
fman_create(char *fname, FileFields *ff)  // cria um file manager
{
    String *bin_fname = str_create(fname);
    str_append(bin_fname, ".bin");

    FileManager *fman = alloc(sizeof(FileManager), fman_release);
    fman->fp = fopen(bin_fname->string, "rb+");
    if (!fman->fp) // se não existe
    {
        fman->fp = fopen(bin_fname->string, "w"); // cria
        long int stack_top = -1;
        fwrite(&stack_top, sizeof(stack_top), 1, fman->fp); // escreve o topo da lista
        fclose(fman->fp);
        fman->fp = fopen(bin_fname->string, "rb+"); // e reabre em modo update
    }
    fman->ff = ff;
    retain(ff);

    fman->db_name = str_create(fname);
    fman->indexes = vector_init();                 //
    for (int i = 0; i < fman->ff->idxc; i++)       //
    {                                              //
        int field_idx = fman->ff->indexes[i];      //
        FieldIndex *fidx = fidx_create(fman,       //
        fman->ff->fields[field_idx], field_idx);   // cria os indices
        vector_append(fman->indexes, fidx);        //
        release(fidx);                             //
    }

    fman->entryc = 0; // contagem de registros
    Vector *offsets = fman_list_all(fman);
    fman_inc_entryc(fman, offsets->count);
    release(offsets);

    release(bin_fname);

    return fman;
}

void
fman_inc_entryc(FileManager *fman, int amnt) // incrementa e decrementa o numero de registros
{
    fman->entryc += amnt;
    for (int i = 0; i < fman->indexes->count; i++)
    {
        FieldIndex *fidx = fman->indexes->objs[i];
        if (fman->entryc >= 10)  // se >= 10
        {
            fidx_create_index(fidx);  //cria o indice para o campo
        }
    }
}

long int
fman_free_offset_for_size(FileManager *fman, int size)  // busca um espaço livre para o novo registro
{
    size += sizeof(size);  // adiciona o espaço do campo de tamanho do registro
    fseek(fman->fp, 0, SEEK_SET);                         //
    long int next_entry = -1;                             //
    fread(&next_entry, sizeof(next_entry), 1, fman->fp);  // lê o topo da lista

    long int previous_stack_top_offset = 0;
    long int previous_best_stack_top_offset = 0;
    long int next_stack_top_offset = -1;
    long int best_fit_offset = 0;


    int best_fit_size = 0xFFFF; //inf
    int fragment = 0;

    size_t min_entry_size = ffields_size(fman->ff, //
    sizeof(char) * 2) + sizeof(int);               // tamanho mínimo de um registro
    while (next_entry != -1) // enquanto houver próximo
    {
        int space_left = 0;
        fseek(fman->fp, next_entry - sizeof(int), SEEK_SET);
        int this_entry_size;
        fread(&this_entry_size, sizeof(this_entry_size), 1, fman->fp);
        this_entry_size = -this_entry_size;
        this_entry_size += sizeof(this_entry_size);
        space_left = this_entry_size - size;
        if (this_entry_size <= 0) //something went wrong :(
        {
            break;
        }
        else if (space_left == 0) //coube exatamente, para de procurar que não vai encontrar melhor
        {
            best_fit_offset = ftell(fman->fp);
            fread(&next_stack_top_offset, sizeof(next_stack_top_offset), 1, fman->fp);
            previous_best_stack_top_offset = previous_stack_top_offset;
            fragment = 0;  // não vai ter fragmentação interna
            break;
        }
        else if (space_left > 0 &&              // ← sobra algum espaço
                 space_left > min_entry_size && // ← cabe outro registro nesse espaço que sobra
                 space_left < best_fit_size)    // ← é menor que os espaços já encontrados
        {
            fragment = 1;  // vai ter fragmentação interna
            best_fit_size = space_left; // é o melhor por enquanto
            best_fit_offset = ftell(fman->fp); // guarda a posição desse espaço
            fread(&next_stack_top_offset, sizeof(next_stack_top_offset), 1, fman->fp); // lê onde fica o próximo
            fseek(fman->fp, best_fit_offset, SEEK_SET); // volta o cursor pro começo do espaço
            previous_best_stack_top_offset = previous_stack_top_offset;
        }
        previous_stack_top_offset = next_entry;
        fseek(fman->fp, next_entry, SEEK_SET);
        fread(&next_entry, sizeof(next_entry), 1, fman->fp);  // lê a próxima entrada
    }

    if (best_fit_offset)  // se encontrou algum espaço
    {
        if (fragment) // se tem fragmentação interna
        {
            fseek(fman->fp, best_fit_offset + size - sizeof(int), SEEK_SET); //vai até o começo da fragmentação
            best_fit_size -= sizeof(int);
            best_fit_size = -best_fit_size;
            fwrite(&best_fit_size, sizeof(best_fit_size), 1, fman->fp); // salva o tamanho dela
            long int fragment_offset = ftell(fman->fp);
            fwrite(&next_stack_top_offset, sizeof(next_stack_top_offset), 1, fman->fp); // e coloca pra onde ela aponta
            fseek(fman->fp, previous_best_stack_top_offset, SEEK_SET);
            fwrite(&fragment_offset, sizeof(fragment_offset), 1, fman->fp); // e quem aponta pra ela
        }
        else // sem fragmentação
        {
            fseek(fman->fp, previous_best_stack_top_offset, SEEK_SET);                  // só precisa escrever no item anterior
            fwrite(&next_stack_top_offset, sizeof(next_stack_top_offset), 1, fman->fp); // a posição do próximo
        }
        return best_fit_offset;
    }
    fseek(fman->fp, 0, SEEK_END);          // se não encontrou nenhum espaço,
    return ftell(fman->fp) + sizeof(int);  // retorna o final do arquivo.
}

Vector *
fman_list_all(FileManager *fman)
{
    Vector *offset_vector = vector_init();
    fseek(fman->fp, sizeof(size_t), SEEK_SET); //pula o topo da pilha
    while(!feof(fman->fp))
    {
        int this_entry_size;
        size_t elements_read = fread(&this_entry_size, sizeof(this_entry_size), 1, fman->fp);
        if (!elements_read)break; // feof
        long int this_entry_offset = ftell(fman->fp);
        if (this_entry_size < 0) // entrada removida
        {
            this_entry_size = -this_entry_size;         // o oposto do tamanho de registro
            fseek(fman->fp, this_entry_size, SEEK_CUR); // removido é o tamanho
        }
        else
        {
            long int *entry_offset = alloc(sizeof(long int), NULL);
            *entry_offset = this_entry_offset;
            vector_append(offset_vector, entry_offset); // coloca no vetor de saida o offset
            release(entry_offset);
            fseek(fman->fp,                           //
                this_entry_offset + this_entry_size,  // e avança o cursor
                SEEK_SET);                            //
        }
    }
    return offset_vector;
}

int  // recupera um registro pelo offset
fman_entry_at_offset(FileManager *fman, long int offset, void *entry /* inout */)
{
    fseek(fman->fp, offset - sizeof(int), SEEK_SET); //vai até o offset
    int entry_size = 0;
    fread(&entry_size, sizeof(entry_size), 1, fman->fp); // lê o tamanho
    if (entry_size < 0) // se for negativo é pq o registro foi removido
    {
        return 0; // 0 quer dizer registro removido
    }
    for (int i = 0; i < fman->ff->fieldc; i++) // caso contrário
    {                                          // preenche os campos
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
    return 1;
}

Vector *
fman_search_by_field(FileManager *fman,     //
    int field_idx, const void *value)       // busca um registro
{
    Vector *offset_vector = NULL;
    for (int i = 0; i < fman->indexes->count; i++) // verifica os indices
    {
        FieldIndex *fidx = fman->indexes->objs[i]; //
        if (fidx->field_idx == field_idx)          // se o campo requerido tem indice
        {
            offset_vector = fidx_search(fidx, value); // e se o indice já foi criado
        }                                             // se não foi, fidx_search retorna NULL
    }

    if (offset_vector) // procuou no indice
    {
        return offset_vector;
    }
    //caso contrário, não existe o indice, faz busca sequencial no arquivo

    offset_vector = vector_init();
    fseek(fman->fp, sizeof(size_t), SEEK_SET); //pula o cabeçalho
    while(!feof(fman->fp))
    {
        int this_entry_size;
        size_t elements_read = fread(&this_entry_size, sizeof(this_entry_size), 1, fman->fp);
        if (!elements_read)break; //feof
        long int this_entry_offset = ftell(fman->fp);
        if (this_entry_size < 0) //registro removido
        {
            this_entry_size = -this_entry_size;
            fseek(fman->fp, this_entry_size, SEEK_CUR);
            continue; // vai pro próximo…
        }
        for (int i = 0; i < fman->ff->fieldc; i++) // lê cada campo do registro
        {
            if (fman->ff->fields[i] == str_f)  // se for string
            {
                String *str = str_from_file(fman->fp, ""); // lê byte a byte até \0
                if (i == field_idx) // se for o campo requerido
                {
                    if (str_eq(str, (String *)value)) // compara, se igual:
                    {
                        // entry_offset = this_entry_offset;
                        long int *entry_offset = alloc(sizeof(long int), NULL); // aloca espaço pro offset
                        *entry_offset = this_entry_offset;
                        vector_append(offset_vector, entry_offset); // e salva no vetor
                        release(entry_offset);
                    }
                    fseek(fman->fp,                           //
                        this_entry_offset + this_entry_size,  // avança pro próximo
                        SEEK_SET);                            //
                    release(str);
                    break;
                }
                release(str);
            }
            else // se não é string, lê o campo
            {
                size_t field_size = ftype_size_of(fman->ff->fields[i]); // de tamanho field_size
                if (i == field_idx) // se for o campo requerido
                {
                    char this_value[30];
                    fread(this_value, field_size, 1, fman->fp); //lê o campo
                    if (fman->ff->fields[i] == float_f)
                    {
                        float a = *(float *)this_value;
                        float b = *(float *)value;
                        if (ABS(a - b) < FLT_EPSILON) // não pode simplesmente comparar float e double com ==
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
                    else // os outros tipos podem ser comparados bit a bit
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
                    fseek(fman->fp, field_size, SEEK_CUR); // avança campo
                }
            }
        }
    }
    return offset_vector;
}

void
fman_add_entry(FileManager *fman, void *o) // adiciona registro no arquivo
{
    int entry_size = 0;
    for (int j = 0; j < fman->ff->fieldc; j++)                             //
    {                                                                      //
        void *entry = o + fman->ff->offsets[j];                            // vê o tamanho do campo
        if (fman->ff->fields[j] == str_f)                                  // a ser guardado
        {                                                                  //
            entry_size += sizeof(char) * ((*(String **)(entry))->len + 1); // string é 1 byte/char + \0
        }                                                                  //
        else                                                               //
        {                                                                  //
            entry_size += ftype_size_of(fman->ff->fields[j]);              // o resto é o proprio tamanho
        }                                                                  //
    }                                                                      //
    long int offset = 0;
    offset = fman_free_offset_for_size(    //
        fman,                              // procura um offset livre
        entry_size);                       //
    fseek(fman->fp,
    offset - sizeof(entry_size),
    SEEK_SET);
    fwrite(&entry_size, sizeof(entry_size), 1, fman->fp); // escreve o tamanho do registro
    for (int j = 0; j < fman->ff->fieldc; j++) // e percorre os campos
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
    for (int i = 0; i < fman->indexes->count; i++)                   //
    {                                                                //
        FieldIndex *fidx = fman->indexes->objs[i];                   //
        void *field = o + fidx->fman->ff->offsets[fidx->field_idx];  // adiciona a nova entrada nos
        if (fidx->ftype == str_f)                                    // indices
        {                                                            //
            String *str = *(String **)field;                         //
            IndexEntry *e = idx_create(fidx->ftype, str, offset);    //
            vector_append(fidx->index, e);                           //
            release(e);                                              //
        }                                                            //
        else                                                         //
        {                                                            //
            IndexEntry *e = idx_create(fidx->ftype, field, offset);  //
            vector_append(fidx->index, e);                           //
            release(e);                                              //
        }                                                            //
        fidx_sort(fidx);                                             // e ordena
    }
    fman_inc_entryc(fman, 1); // incrementa a contagem de registros
}

void  // remove registro
fman_remove_entry_at_offset(FileManager *fman, long int offset)
{
    fseek(fman->fp, 0, SEEK_SET);
    long int stack_top = 0;
    fread(&stack_top, sizeof(stack_top), 1, fman->fp);
    fseek(fman->fp, offset - sizeof(int), SEEK_SET);
    int this_entry_size = 0;
    fread(&this_entry_size, sizeof(this_entry_size), 1, fman->fp);
    if(this_entry_size < 0) // já foi removido
    {
        return;
    }
    this_entry_size = -this_entry_size;
    fseek(fman->fp, offset - sizeof(int), SEEK_SET);
    fwrite(&this_entry_size, sizeof(this_entry_size), 1, fman->fp);
    fwrite(&stack_top, sizeof(stack_top), 1, fman->fp);
    fseek(fman->fp, 0, SEEK_SET);
    fwrite(&offset, sizeof(offset), 1, fman->fp);

    for (int i = 0; i < fman->indexes->count; i++)       //
    {                                                    //
        FieldIndex *fidx = fman->indexes->objs[i];       //
        if (fidx->index)                                 //
        {                                                //
            for (int j = 0; j < fidx->index->count; j++) //
            {                                            // remove as entradas dos indices
                IndexEntry *e = fidx->index->objs[j];    //
                if (e->offset == offset)                 //
                {                                        //
                    vector_remove(fidx->index, j);       //
                }                                        //
            }                                            //
        }                                                //
    }

    fman_inc_entryc(fman, -1); // decrementa a contagem de registros
}

int // função auxiliar pra ordenar offsets
_vect_sort_offset(const void *v1,const void *v2)
{
    long int *p1 = *(long int **)v1;
    long int *p2 = *(long int **)v2;
    return *p1 - *p2;
}

Vector *
fman_merge_offsets(Vector *v1, Vector *v2)//merge
{
    vector_sort(v1, _vect_sort_offset);
    vector_sort(v2, _vect_sort_offset);
    int i = 0;
    int k = 0;
    Vector *result = vector_init();
    while (i < v1->count && k < v2->count)
    {
        long int offset1 = *((long int *)(v1->objs[i]));
        long int offset2 = *((long int *)(v2->objs[k]));
        if (offset1 < offset2)
            vector_append(result, v1->objs[i++]);
        else if (offset1 > offset2)
            vector_append(result, v2->objs[k++]);
        else
        {
            vector_append(result, v1->objs[i]);
            i++;
            k++;
        }
    }

    for (; i < v1->count; i++)                    //
    {                                             //
        vector_append(result, v1->objs[i]);       //
    }                                             //
    for (; k < v2->count; k++)                    // os que sobraram
    {                                             //
        vector_append(result, v2->objs[k]);       //
    }                                             //
    return result;
}

Vector *
fman_match_offsets(Vector *v1, Vector *v2) //match
{
    vector_sort(v1, _vect_sort_offset);
    vector_sort(v2, _vect_sort_offset);
    int i = 0;
    int k = 0;
    Vector *result = vector_init();
    while (i < v1->count && k < v2->count)
    {
        long int offset1 = *((long int *)(v1->objs[i]));
        long int offset2 = *((long int *)(v2->objs[k]));
        if (offset1 < offset2)
            i++;
        else if (offset1 > offset2)
            k++;
        else
        {
            vector_append(result, v1->objs[i]);
            i++;
            k++;
        }
    }
    return result;
}

void
fman_release(void *o)
{
    FileManager *fman = o;

    release(fman->indexes);
    release(fman->ff);
    release(fman->db_name);
    fclose(fman->fp);
}
