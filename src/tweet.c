#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>

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
    if (!user->len)
    {
        release(user);
        return NULL;
    }

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

int
_tty_width()
{
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

    return w.ws_col;
}

void
tweet_print(Tweet *t)
{
    int width = _tty_width() / 7 -2;

    Vector *columns = vector_init();
    {
        Vector *lines = str_wrap(t->user, width);
        vector_append(columns, lines);
        release(lines);
    }
    {
        Vector *lines = str_wrap(t->text, width);
        vector_append(columns, lines);
        release(lines);
    }
    {
        Vector *lines = str_wrap(t->coordinates, width);
        vector_append(columns, lines);
        release(lines);
    }
    {
        String *s = str_from_int(t->favorite_count);
        Vector *v = str_wrap(s, width);
        release(s);
        vector_append(columns, v);
        release(v);
    }
    {
        Vector *lines = str_wrap(t->language, width);
        vector_append(columns, lines);
        release(lines);
    }
    {
        String *s = str_from_int(t->retweet_count);
        Vector *v = str_wrap(s, width);
        release(s);
        vector_append(columns, v);
        release(v);
    }
    {
        String *s = str_from_long(t->views_count);
        Vector *v = str_wrap(s, width);
        release(s);
        vector_append(columns, v);
        release(v);
    }
    int max_height = 0;
    for (int i = 0; i < columns->count; i++)
    {
        Vector *lines = columns->objs[i];
        max_height = lines->count > max_height ? lines->count:max_height;

    }

    for (int i = 0; i < max_height; i++)
    {
        for (int j = 0; j < columns->count; ++j)
        {
            Vector *lines = columns->objs[j];
            if (i >= lines->count)
            {
                printf("|");
                for (int k = 0; k < width; k++)
                {
                    printf(" ");
                }
            }
            else
            {
                String *s = lines->objs[i];
                printf("|%s", s->string);
            }
        }
        printf("|\n");
    }
    release(columns);
}


String *
tweet_separator()
{
    int width = _tty_width() / 7 -2;
    String *separator = str_init();
    for (int i = 0; i < 7; i++)
    {
        str_append(separator, "+");
        for (int j = 0; j < width; j++)
        {
            str_append(separator, "-");
        }
    }
    str_append(separator, "+");
    return separator;
}
String *
tweet_header()
{
    int width = _tty_width() / 7 -2;
    String *separator = tweet_separator();
    String *header = str_init();
    str_append(header, separator->string);
    str_append(header, "\n");

    str_append(header, "|");
    String *s;
    s = str_create("Usuário");
    str_center(s, width);
    str_append(header, s->string);
    release(s);

    str_append(header, "|");
    s = str_create("Texto");
    str_center(s, width);
    str_append(header, s->string);
    release(s);

    str_append(header, "|");
    s = str_create("Coords.");
    str_center(s, width);
    str_append(header, s->string);
    release(s);

    str_append(header, "|");
    s = str_create("Fav count");
    str_center(s, width);
    str_append(header, s->string);
    release(s);

    str_append(header, "|");
    s = str_create("Língua");
    str_center(s, width);
    str_append(header, s->string);
    release(s);

    str_append(header, "|");
    s = str_create("Retweets");
    str_center(s, width);
    str_append(header, s->string);
    release(s);

    str_append(header, "|");
    s = str_create("Views");
    str_center(s, width);
    str_append(header, s->string);
    release(s);
    str_append(header, "|\n");

    str_append(header, separator->string);
    str_append(header, "\n");
    release(separator);
    return header;
}

void
tweet_print_many_waiting(Vector *v)
{
    for (int i = 0; i < v->count; ++i)
    {
        String *separator = tweet_separator();
        String *header = tweet_header();
        printf("%d de %zu\n", i+1, v->count);
        printf("%s", header->string);
        Tweet *t = v->objs[i];
        tweet_print(t);
        printf("%s\n", separator->string);
        printf("[pressione enter para continuar ou q para parar]\n");
        String *blank = str_from_stdin();
        if (!strcmp("q", blank->string) ||
            !strcmp("Q", blank->string))
        {
            release(separator);
            release(blank);
            release(header);
            break;
        }
        release(blank);
        system("clear");
        release(separator);
        release(header);
    }
}



void
tweet_print_many(Vector *v)
{
    String *separator = tweet_separator();
    String *header = tweet_header();
    printf("%s", header->string);
    for (int i = 0; i < v->count; i++)
    {
        if (i)
            printf("%s\n", separator->string);
        Tweet *t = v->objs[i];
        tweet_print(t);
    }
    printf("%s\n", separator->string);

    release(separator);
    release(header);
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
    str_f,  (void *)&t.text - (void *)&t,           0,  /*text*/
    str_f,  (void *)&t.user - (void *)&t,           0,  /*user*/
    str_f,  (void *)&t.coordinates - (void *)&t,    0,  /*coordinates*/
    int_f,  (void *)&t.favorite_count - (void *)&t, 1,  /*favorite_count*/
    str_f,  (void *)&t.language - (void *)&t,       1,  /*language*/
    int_f,  (void *)&t.retweet_count - (void *)&t,  0,  /*retweet_count*/
    long_f, (void *)&t.views_count - (void *)&t,    0   /*views_count*/
    );
}
