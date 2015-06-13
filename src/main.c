#include <stdlib.h>
#include <stdio.h>

#include "tweet.h"

int
_tweet_cmp(const void *v1, const void *v2)
{
    return tweet_cmp(*(Tweet **)v1, *(Tweet **)v2);
}

int
menu()
{
    String *blank = str_from_stdin();
    release(blank);
    system("clear");
    printf("Escolha uma opção:\n");
    printf("\t1 - Inserir Tweets\n");
    printf("\t2 - Listar Tweets\n");
    printf("\t3 - Buscar Tweet por usuário\n");
    printf("\t4 - Buscar Tweet por número de favoritos\n");
    printf("\t5 - Buscar Tweet por idioma\n");
    printf("\t6 - Buscar Tweet por número de favoritos e idioma\n");
    printf("\t7 - Buscar Tweet por número de favoritos ou idioma\n");
    printf("\n");
    printf("\t0 - Sair\n");
    String *choice = str_from_stdin();
    int ichoice = atoi(choice->string);
    release(choice);
    return ichoice;
}

int
main(int argc, char *argv[])
{
    FileFields *ff = tweet_filefields();
    FileManager *fman = fman_create("/Users/gonzo/Desktop/PiuPiuFiles/PiuPiu", ff);
    release(ff);

    int quit = 0;
    while(!quit)
    {
        switch (menu())
        {
            case 1:
            {
                while(1)
                {
                    printf("\nDigite as informações, para terminar a insersão deixe o campo usuário em branco.\n\n");
                    Tweet *t = tweet_from_stdin();
                    if (!t)break;
                    fman_add_entry(fman, t);
                    release(t);
                }
            }
            break;
            case 2:
            {
                Vector *offset_vector = fman_list_all(fman);
                printf("\nfound:%zu\n", offset_vector->count);
                for (int i = 0; i < offset_vector->count; ++i)
                {
                    Tweet *t = tweet_init();
                    long int offset = *((long int *)offset_vector->objs[i]);
                    fman_entry_at_offset(fman, offset, t);
                    printf("@%3ld -> ", offset);
                    tweet_print(t);
                    release(t);
                }
                release(offset_vector);
            }
            break;
            case 3:
            {
                printf("Usuário: ");
                String *user = str_from_stdin();
                Vector *offset_vector = fman_search_by_field(fman, 1, user);
                release(user);
                printf("\nfound:%zu\n", offset_vector->count);
                for (int i = 0; i < offset_vector->count; ++i)
                {
                    Tweet *t = tweet_init();
                    long int offset = *((long int *)offset_vector->objs[i]);
                    fman_entry_at_offset(fman, offset, t);
                    tweet_print(t);
                    release(t);
                }
                release(offset_vector);
            }
            break;
            case 4:
            {
                printf("Número de favoritos: ");
                String *fav_s = str_from_stdin();
                int fav = atoi(fav_s->string);
                Vector *offset_vector = fman_search_by_field(fman, 3, &fav);
                printf("\nfound:%zu\n", offset_vector->count);
                for (int i = 0; i < offset_vector->count; ++i)
                {
                    Tweet *t = tweet_init();
                    long int offset = *((long int *)offset_vector->objs[i]);
                    fman_entry_at_offset(fman, offset, t);
                    tweet_print(t);
                    release(t);
                }
                release(offset_vector);
                release(fav_s);
            }
            break;
            case 5:
            {
                printf("idioma: ");
                String *language = str_from_stdin();
                Vector *offset_vector = fman_search_by_field(fman, 4, language);
                printf("\nfound:%zu\n", offset_vector->count);
                for (int i = 0; i < offset_vector->count; ++i)
                {
                    Tweet *t = tweet_init();
                    long int offset = *((long int *)offset_vector->objs[i]);
                    fman_entry_at_offset(fman, offset, t);
                    tweet_print(t);
                    release(t);
                }
                release(offset_vector);
                release(language);
            }
            break;
            case 6:
            {
                printf("Número de favoritos: ");
                String *fav_s = str_from_stdin();
                int fav = atoi(fav_s->string);
                printf("idioma: ");
                String *language = str_from_stdin();
                Vector *language_vector = fman_search_by_field(fman, 4, language);
                Vector *fav_vector = fman_search_by_field(fman, 3, &fav);

                Vector *offset_vector = fman_match_offsets(language_vector, fav_vector);

                printf("\nfound:%zu\n", offset_vector->count);
                for (int i = 0; i < offset_vector->count; ++i)
                {
                    Tweet *t = tweet_init();
                    long int offset = *((long int *)offset_vector->objs[i]);
                    fman_entry_at_offset(fman, offset, t);
                    tweet_print(t);
                    release(t);
                }
                release(offset_vector);
                release(language_vector);
                release(fav_vector);
                release(language);
                release(fav_s);
            }
            break;
            case 7:
            {
                printf("Número de favoritos: ");
                String *fav_s = str_from_stdin();
                int fav = atoi(fav_s->string);
                printf("idioma: ");
                String *language = str_from_stdin();
                Vector *language_vector = fman_search_by_field(fman, 4, language);
                Vector *fav_vector = fman_search_by_field(fman, 3, &fav);

                Vector *offset_vector = fman_merge_offsets(language_vector, fav_vector);

                printf("\nfound:%zu\n", offset_vector->count);
                for (int i = 0; i < offset_vector->count; ++i)
                {
                    Tweet *t = tweet_init();
                    long int offset = *((long int *)offset_vector->objs[i]);
                    fman_entry_at_offset(fman, offset, t);
                    tweet_print(t);
                    release(t);
                }
                release(offset_vector);
                release(language_vector);
                release(fav_vector);
                release(language);
                release(fav_s);
            }
            break;


            case 0:
                quit = 1;
                break;
            default:
                printf("Opção inválida\n");
                break;
        }
    }
    release(fman);
    printf("\nGood bye\n\n");
    return EXIT_SUCCESS;
}
