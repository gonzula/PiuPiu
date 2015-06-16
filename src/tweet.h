#ifndef TWEET_H_INCLUDED
#define TWEET_H_INCLUDED

#include <string.h>

#include "structures/structures.h"


typedef struct
{
    String *text;
    String *user;
    String *coordinates;
    int favorite_count;
    String *language;
    int retweet_count;
    long views_count;
} Tweet;

Tweet *tweet_init();
Tweet *tweet_create(        //
    String *text,           //
    String *user,           //
    String *coordinates,    //
    int favorite_count,     // cria tweet com base nos valores passados
    String *language,       //
    int retweet_count,      //
    long views_count        //
    );                      //

Tweet *tweet_from_stdin();  // lê um tweet da entrada do teclado
                            // retorna NULL se o usuário está em branco

int tweet_cmp(Tweet *t1, Tweet *t2); // compara dois tweets
                                     // por usuário e texto respectivamente

String *tweet_separator(); // separador de impressão
String *tweet_header();  // cabeçalho de impressão
void tweet_print(Tweet *t);   // imprime 1
void tweet_print_many(Vector *v); // imprime varios
void tweet_print_many_waiting(Vector *v);  // imprime um por vez

FileFields *tweet_filefields();


#endif
