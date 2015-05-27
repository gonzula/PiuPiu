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
    printf("\t1 - Criar Tweet\n");
    printf("\t2 - Listar Tweets\n");
    printf("\t3 - Deletar Tweet\n");
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
    FileManager *fman = fman_create("/Users/gonzo/Desktop/PiuPiu.bin", ff);
    release(ff);

    while(1)
    {
        switch (menu())
        {
            case 1:
            {
                Tweet *t = tweet_from_stdin();
                // tweet_print(t);
                // printf("%ld\n", t->views_count);
                fman_add_entry(fman, t);
                release(t);
            }
                break;
            case 2:
                break;
            case 3:
                break;
            case 0:
                return 0;
            default:
                printf("Opção inválida\n");
                break;
        }
    }
    release(fman);
    return EXIT_SUCCESS;
}
