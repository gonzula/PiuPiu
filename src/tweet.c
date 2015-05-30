#include <stdio.h>

#include "tweet.h"


void
tweet_release(void *);

Tweet *
tweet_init()
{
    return alloc(sizeof(Tweet), tweet_release);
}

Tweet *
tweet_create(
    String *text,
    String *user,
    String *coordinates,
    int favorite_count,
    String *language,
    int retweet_count,
    long views_count
    )
{
    Tweet *t = tweet_init();
    t->text = text;
    retain(text);
    t->user = user;
    retain(user);
    t->coordinates = coordinates;
    retain(coordinates);
    t->favorite_count = favorite_count;
    t->language = language;
    retain(language);
    t->retweet_count = retweet_count;
    t->views_count = views_count;

    return t;
}

Tweet *
tweet_from_stdin()
{
    String *aux_str;
    printf("Insira o usuário: ");
    String *user = str_from_stdin();

    printf("Insira o texto: ");
    String *text = str_from_stdin();

    printf("Insira as coordenadas: ");
    String *coordinates = str_from_stdin();

    printf("Insira o número de favoritos: ");
    aux_str = str_from_stdin();
    int favorite_count = atoi(aux_str->string);
    release(aux_str);

    printf("Insira a língua: ");
    String *language = str_from_stdin();

    printf("Insira o número de retweets: ");
    aux_str = str_from_stdin();
    int retweet_count = atoi(aux_str->string);
    release(aux_str);

    printf("Insira o número de vizualizações: ");
    aux_str = str_from_stdin();
    long views_count = atol(aux_str->string);
    release(aux_str);

    Tweet *t = tweet_create(
        text, user, coordinates,
        favorite_count, language,
        retweet_count, views_count);
    release(user);
    release(text);
    release(coordinates);
    release(language);
    return t;
}

int
tweet_cmp(Tweet *t1, Tweet *t2)
{
    int user_result = strcmp(t1->user->string, t2->user->string);
    if (user_result)
        return user_result;
    return strcmp(t1->text->string, t2->text->string);
}

void
tweet_print(Tweet *t)
{
    printf("%s: \"%s\" @ %s (%ld)\n", t->user->string, t->text->string, t->coordinates->string, t->views_count);
}


void
tweet_release(void *o)
{
    Tweet *t = o;
    release(t->text);
    release(t->user);
    release(t->coordinates);
    release(t->language);
}

FileFields *
tweet_filefields()
{
    Tweet t;
    return ffields_create(7,
    str_f,  (void *)&t.text - (void *)&t,           /*text*/
    str_f,  (void *)&t.user - (void *)&t,           /*user*/
    str_f,  (void *)&t.coordinates - (void *)&t,    /*coordinates*/
    int_f,  (void *)&t.favorite_count - (void *)&t, /*favorite_count*/
    str_f,  (void *)&t.language - (void *)&t,       /*language*/
    int_f,  (void *)&t.retweet_count - (void *)&t,  /*retweet_count*/
    long_f, (void *)&t.views_count - (void *)&t     /*views_count*/
    );
}
