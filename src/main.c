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
    printf("Escolha uma opção:\n");
    printf("\t1 - Inserir Tweets\n");
    printf("\t2 - Listar Tweets\n");
    printf("\t3 - Buscar Tweet por usuário\n");
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
    ffields_print(ff);
    FileManager *fman = fman_create("PiuPiu.bin", ff);
    release(ff);

    int quit = 0;
    while(!quit)
    {
        switch (menu())
        {
            case 1:
            {
                printf("Digite as informações, para interromper deixe o campo usuário em branco.\n");
                while(1)
                {
                    Tweet *t = tweet_from_stdin();
                    if (!t)break;
                    fman_add_entry(fman, t);
                    release(t);
                }
            }
            break;
            case 2:
            {
                // String *search = str_create("123");
                // long search = 100000;
                Vector *offset_vector = fman_list_all(fman);// = fman_search_by_field(fman, 6, &search);
                // release(search);
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
            case 0:
                quit = 1;
                break;
            default:
                printf("Opção inválida\n");
                break;
        }
    }
    release(fman);
    return EXIT_SUCCESS;
}
