//////////////////////////////////////////////
//                                          //
//  Carlos Eduardo Ayoub Fialho #7563703    //
//  João Gustavo Cabral de Marins #7563982  //
//  Romeu Bertho Junior #7151905            //
//                                          //
//////////////////////////////////////////////


#include <signal.h>
#include <stdlib.h>
#include <stdio.h>

#include "tweet.h"


FileManager *fman;

void INThandler(int);
int menu();
void good_bye();

Vector *tweets_from_offsets(FileManager *fman, Vector *offset_vector);
void print_tweets_from_offset_vector(FileManager *fman, Vector *offset_vector, int waiting);

int
main(int argc, char *argv[])
{
    signal(SIGINT, INThandler);// captura o CTRL+C
    FileFields *ff = tweet_filefields(); // cria o FileFields
    fman = fman_create("PiuPiu", ff); // cria o FileManager
    release(ff); // não precisa mais do ff

    int quit = 0; // controla quando deve sair
    while(!quit)
    {
        int option = menu(); // retorna a opção escolhida pelo usuário
        switch (option)
        {
            case 1:
            {
                while(1) // inserir tweets
                {
                    printf("\nDigite as informações, para terminar a insersão deixe o campo usuário em branco.\n\n");
                    Tweet *t = tweet_from_stdin(); //retorna NULL se o usuário está em branco
                    if (!t)break; // caso de parada
                    fman_add_entry(fman, t); //salva no arquivo
                    release(t);
                }
            }
            break;
            case 2: // lista todos tweets
            case 3:
            {
                Vector *offset_vector = fman_list_all(fman); // retorna vetor de offsets
                if (!offset_vector->count)
                {
                    printf("Nenhum resultado\n");
                }
                else if (option == 3)
                {
                    system("clear");
                }
                print_tweets_from_offset_vector(fman, offset_vector, option==3);
                release(offset_vector);
            }
            break;
            case 4: //busca por usuário
            {
                printf("Usuário: ");
                String *user = str_from_stdin(); //pega o usuário a ser buscado
                                                                // ↓ 1 representa o campo usuário no FileFields
                Vector *offset_vector = fman_search_by_field(fman, 1, user); // busca
                release(user);
                if (!offset_vector->count)
                {
                    printf("Nenhum resultado\n");
                }
                print_tweets_from_offset_vector(fman, offset_vector, 0);
                release(offset_vector);
            }
            break;
            case 5:
            {
                printf("Número de favoritos: ");
                String *fav_s = str_from_stdin();
                int fav = atoi(fav_s->string);
                                                                // ↓ 1 representa o campo favorite_count no FileFields
                Vector *offset_vector = fman_search_by_field(fman, 3, &fav); // verifica se existe indice e faz a busca
                if (!offset_vector->count)
                {
                    printf("Nenhum resultado\n");
                }
                print_tweets_from_offset_vector(fman, offset_vector, 0);
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
                print_tweets_from_offset_vector(fman, offset_vector, 0);
                release(offset_vector);
                release(language);
            }
            break;
            case 7:
            {
                printf("Número de favoritos: ");
                String *fav_s = str_from_stdin();
                int fav = atoi(fav_s->string);
                printf("Idioma: ");
                String *language = str_from_stdin();
                Vector *language_vector = fman_search_by_field(fman, 4, language);
                Vector *fav_vector = fman_search_by_field(fman, 3, &fav);

                Vector *offset_vector = fman_match_offsets(language_vector, fav_vector); // faz o match dos dois vetores de offset
                if (!offset_vector->count)
                {
                    printf("Nenhum resultado\n");
                }
                print_tweets_from_offset_vector(fman, offset_vector, 0);
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
                printf("Idioma: ");
                String *language = str_from_stdin();
                Vector *language_vector = fman_search_by_field(fman, 4, language);
                Vector *fav_vector = fman_search_by_field(fman, 3, &fav);

                Vector *offset_vector = fman_merge_offsets(language_vector, fav_vector);// faz o merge dos dois vetores de offset

                if (!offset_vector->count)
                {
                    printf("Nenhum resultado\n");
                }
                print_tweets_from_offset_vector(fman, offset_vector, 0);
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
                Vector *offset_vector = fman_search_by_field(fman, 3, &fav); //busca os offsets tweets

                if (!offset_vector->count)
                {
                    printf("Nenhum resultado\n");
                }
                else
                {
                    Vector *tweet_vector = tweets_from_offsets(fman, offset_vector); //recupera os tweets
                    String *separator = tweet_separator();
                    String *header = tweet_header();
                    printf("%s\n", header->string);
                    release(header);
                    for (int i = 0; i < tweet_vector->count; i++)  //
                    {                                              //
                        printf("\n\nopção: %d\n", i);              //  imprime na tela os tweets
                        puts(separator->string);                   //  para o usuário escolher qual
                        tweet_print(tweet_vector->objs[i]);        //  apagar
                        puts(separator->string);                   //
                    }                                              //
                    release(separator);
                    printf("Selecione o tweet a ser removido ou -1 para cancelar: ");
                    String *choice = str_from_stdin();
                    if (!choice->len)
                    {                                                    //
                        release(offset_vector);                          //
                        release(fav_s);                                  //
                        release(tweet_vector);                           //
                        release(choice);                                 //
                        break;                                           //
                    }                                                    //
                    int ichoice = atoi(choice->string);                  //
                    release(choice);                                     //
                    if (ichoice == -1)                                   //
                    {                                                    //
                        release(offset_vector);                          //
                        release(fav_s);                                  //  trata os erros de entrada
                        release(tweet_vector);                           //
                        break;                                           //
                    }                                                    //
                    if (ichoice < 0 || ichoice >= offset_vector->count)  //
                    {                                                    //
                        printf("Opção inválida\n");                      //
                        release(offset_vector);                          //
                        release(fav_s);                                  //
                        release(tweet_vector);                           //
                        break;                                           //
                    }                                                    //
                    fman_remove_entry_at_offset(fman,
                        *(long int *)(offset_vector->objs[ichoice]));//remove o tweet escolhido pelo usuário
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

Vector *
tweets_from_offsets(FileManager *fman, Vector *offset_vector)
{
    Vector *tweet_vector = vector_init();
    for (int i = 0; i < offset_vector->count; ++i)
    {
        Tweet *t = tweet_init();
        long int offset = *((long int *)offset_vector->objs[i]);
        if(fman_entry_at_offset(fman, offset, t)) // retorna 1 se o tweet não está deletado
        {
            vector_append(tweet_vector, t); //salva na lista de tweets
        }
        release(t); // não precisa mais do tweet, o vetor está segurando ele.
    }
    return tweet_vector;
}

void
print_tweets_from_offset_vector(FileManager *fman, Vector *offset_vector, int waiting)
{
    Vector *tweet_vector = tweets_from_offsets(fman, offset_vector);
    if (!waiting)
        tweet_print_many(tweet_vector); //imprime os tweets
    else
        tweet_print_many_waiting(tweet_vector); //imprime os tweets um por vez
    release(tweet_vector);
}

void  INThandler(int sig)   //
{                           //
     signal(sig, SIG_IGN);  // gerencia o CTRL+C
     good_bye();            //
}                           //

void                            //
good_bye()                      //
{                               //
    release(fman);              // desaloca o FileManager e sai
    printf("\nGood bye\n\n");   //
    exit(0);                    //
}                               //
