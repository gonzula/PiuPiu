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
Tweet *tweet_create(
    String *text,
    String *user,
    String *coordinates,
    int favorite_count,
    String *language,
    int retweet_count,
    long views_count
    );
Tweet *tweet_from_stdin();

int tweet_cmp(Tweet *t1, Tweet *t2);

void tweet_print(Tweet *t);

FileFields *tweet_filefields();


#endif
