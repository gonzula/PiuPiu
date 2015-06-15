#include <signal.h>
#include <stdlib.h>
#include <stdio.h>

#include "tweet.h"


FileManager *fman;

void INThandler(int);
int menu();
void good_bye();

int
main(int argc, char *argv[])
{
    signal(SIGINT, INThandler);
    FileFields *ff = tweet_filefields();
    fman = fman_create("PiuPiu", ff);
    release(ff);

    int quit = 0;
    while(!quit)
    {
        int option = menu();
        switch (option)
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
                if (!offset_vector->count)
                {
                    printf("Nenhum resultado\n");
                }
                Vector *tweet_vector = vector_init();
                for (int i = 0; i < offset_vector->count; ++i)
                {
                    Tweet *t = tweet_init();
                    long int offset = *((long int *)offset_vector->objs[i]);
                    if(fman_entry_at_offset(fman, offset, t))
                    {
                        vector_append(tweet_vector, t);
                    }
                    release(t);
                }
                tweet_print_many(tweet_vector);
                release(tweet_vector);
                release(offset_vector);
            }
            break;
            case 3:
            {
                Vector *offset_vector = fman_list_all(fman);
                if (!offset_vector->count)
                {
                    printf("Nenhum resultado\n");
                }
                else
                {
                    system("clear");
                }
                Vector *tweet_vector = vector_init();
                for (int i = 0; i < offset_vector->count; ++i)
                {
                    Tweet *t = tweet_init();
                    long int offset = *((long int *)offset_vector->objs[i]);
                    if(fman_entry_at_offset(fman, offset, t))
                    {
                        vector_append(tweet_vector, t);
                    }
                    release(t);

                }
                tweet_print_many_waiting(tweet_vector);
                release(tweet_vector);
                release(offset_vector);
            }
            break;
            case 4:
            {
                printf("Usuário: ");
                String *user = str_from_stdin();
                Vector *offset_vector = fman_search_by_field(fman, 1, user);
                release(user);

                if (!offset_vector->count)
                {
                    printf("Nenhum resultado\n");
                }
                Vector *tweet_vector = vector_init();
                for (int i = 0; i < offset_vector->count; ++i)
                {
                    Tweet *t = tweet_init();
                    long int offset = *((long int *)offset_vector->objs[i]);
                    if(fman_entry_at_offset(fman, offset, t))
                    {
                        vector_append(tweet_vector, t);
                    }
                    release(t);
                }
                tweet_print_many(tweet_vector);
                release(tweet_vector);
                release(offset_vector);
            }
            break;
            case 5:
            {
                printf("Número de favoritos: ");
                String *fav_s = str_from_stdin();
                int fav = atoi(fav_s->string);
                Vector *offset_vector = fman_search_by_field(fman, 3, &fav);
                if (!offset_vector->count)
                {
                    printf("Nenhum resultado\n");
                }
                Vector *tweet_vector = vector_init();
                for (int i = 0; i < offset_vector->count; ++i)
                {
                    Tweet *t = tweet_init();
                    long int offset = *((long int *)offset_vector->objs[i]);
                    if(fman_entry_at_offset(fman, offset, t))
                    {
                        vector_append(tweet_vector, t);
                    }
                    release(t);
                }
                tweet_print_many(tweet_vector);
                release(tweet_vector);
                release(offset_vector);
                release(fav_s);
            }
            break;
            case 6:
            {
                printf("idioma: ");
                String *language = str_from_stdin();
                Vector *offset_vector = fman_search_by_field(fman, 4, language);
                if (!offset_vector->count)
                {
                    printf("Nenhum resultado\n");
                }
                Vector *tweet_vector = vector_init();
                for (int i = 0; i < offset_vector->count; ++i)
                {
                    Tweet *t = tweet_init();
                    long int offset = *((long int *)offset_vector->objs[i]);
                    if(fman_entry_at_offset(fman, offset, t))
                    {
                        vector_append(tweet_vector, t);
                    }
                    release(t);
                }
                tweet_print_many(tweet_vector);
                release(tweet_vector);
                release(offset_vector);
                release(language);
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

                Vector *offset_vector = fman_match_offsets(language_vector, fav_vector);
                if (!offset_vector->count)
                {
                    printf("Nenhum resultado\n");
                }
                Vector *tweet_vector = vector_init();
                for (int i = 0; i < offset_vector->count; ++i)
                {
                    Tweet *t = tweet_init();
                    long int offset = *((long int *)offset_vector->objs[i]);
                    if(fman_entry_at_offset(fman, offset, t))
                    {
                        vector_append(tweet_vector, t);
                    }
                    release(t);
                }
                tweet_print_many(tweet_vector);
                release(tweet_vector);
                release(offset_vector);
                release(language_vector);
                release(fav_vector);
                release(language);
                release(fav_s);
            }
            break;
            case 8:
            {
                printf("Número de favoritos: ");
                String *fav_s = str_from_stdin();
                int fav = atoi(fav_s->string);
                printf("idioma: ");
                String *language = str_from_stdin();
                Vector *language_vector = fman_search_by_field(fman, 4, language);
                Vector *fav_vector = fman_search_by_field(fman, 3, &fav);

                Vector *offset_vector = fman_merge_offsets(language_vector, fav_vector);

                if (!offset_vector->count)
                {
                    printf("Nenhum resultado\n");
                }
                Vector *tweet_vector = vector_init();
                for (int i = 0; i < offset_vector->count; ++i)
                {
                    Tweet *t = tweet_init();
                    long int offset = *((long int *)offset_vector->objs[i]);
                    if(fman_entry_at_offset(fman, offset, t))
                    {
                        vector_append(tweet_vector, t);
                    }
                    release(t);
                }
                tweet_print_many(tweet_vector);
                release(tweet_vector);
                release(offset_vector);
                release(language_vector);
                release(fav_vector);
                release(language);
                release(fav_s);
            }
            break;
            case 9:
            {
                printf("Número de favoritos: ");
                String *fav_s = str_from_stdin();
                int fav = atoi(fav_s->string);
                Vector *offset_vector = fman_search_by_field(fman, 3, &fav);

                if (!offset_vector->count)
                {
                    printf("Nenhum resultado\n");
                }
                else
                {
                    Vector *tweet_vector = vector_init();
                    for (int i = 0; i < offset_vector->count; i++)
                    {
                        Tweet *t = tweet_init();
                        long int offset = *((long int *)offset_vector->objs[i]);
                        if(fman_entry_at_offset(fman, offset, t))
                        {
                            // printf("%d: ", i);
                            // tweet_print(t);
                            vector_append(tweet_vector, t);
                        }
                        release(t);
                    }
                    String *separator = tweet_separator();
                    String *header = tweet_header();
                    printf("%s\n", header->string);
                    release(header);
                    for (int i = 0; i < tweet_vector->count; i++)
                    {
                        printf("%d:\n", i);
                        puts(separator->string);
                        tweet_print(tweet_vector->objs[i]);
                        puts(separator->string);
                    }
                    release(separator);
                    printf("Selecione o tweet a ser removido ou -1 para cancelar: ");
                    String *choice = str_from_stdin();
                    int ichoice = atoi(choice->string);
                    release(choice);
                    if (ichoice == -1)
                    {
                        release(offset_vector);
                        release(fav_s);
                        release(tweet_vector);
                        break;
                    }
                    if (ichoice < 0 || ichoice >= offset_vector->count)
                    {
                        printf("Opção inválida\n");
                        release(offset_vector);
                        release(fav_s);
                        release(tweet_vector);
                        break;
                    }
                    fman_remove_entry_at_offset(fman, *(long int *)(offset_vector->objs[ichoice]));
                    release(tweet_vector);
                }
                release(offset_vector);
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
    good_bye();
    return EXIT_SUCCESS;
}

int
menu()
{
    static int first = 1;
    if (!first)
    {
        printf("[pressione enter]\n");
        String *blank = str_from_stdin();
        release(blank);
    }
    first = 0;
    system("clear");
    printf("Escolha uma opção:\n");
    printf("\t1 - Inserir Tweets\n");
    printf("\t2 - Listar Tweets\n");
    printf("\t3 - Listar Tweets um por vez\n");
    printf("\t4 - Buscar Tweet por usuário\n");
    printf("\t5 - Buscar Tweet por número de favoritos\n");
    printf("\t6 - Buscar Tweet por idioma\n");
    printf("\t7 - Buscar Tweet por número de favoritos e idioma\n");
    printf("\t8 - Buscar Tweet por número de favoritos ou idioma\n");
    printf("\t9 - Remover Tweet por número de favoritos\n");
    printf("\n");
    printf("\t0 - Sair\n");
    String *choice = str_from_stdin();
    int ichoice = atoi(choice->string);
    release(choice);
    return ichoice;
}

void  INThandler(int sig)
{
     signal(sig, SIG_IGN);
     good_bye();
}

void
good_bye()
{
    release(fman);
    printf("\nGood bye\n\n");
    exit(0);
}
