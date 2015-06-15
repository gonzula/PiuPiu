#ifndef __str_H_
#define __str_H_

#include <stdio.h>
#include <string.h>
#include "vector.h"

typedef struct
{
    union {
        char *string;
        unsigned char *ustring;
    };
    size_t len;
    size_t bufferSize;
} String;

#define str_cmp(s1, s2) strcmp((s1)->string, (s2)->string)
#define str_eq(s1, s2) !str_cmp((s1), (s2))

String *str_init();
String *str_create(const char * original);
String *str_escape_cstring(char * string);
String *str_escape(String *str);
String *str_from_file(FILE *fp, const char *stopchars);
String *str_from_stdin();

String *str_from_int(int i);
String *str_from_long(long int l);

Vector *str_wrap(String *str, int width);


void str_append(String *str, const char * toAppend);
void str_append_char(String *str, const unsigned char c);
void str_center(String *str, int size);
void str_rjust(String *str, int size);
void str_ljust(String *str, int size);
int str_unicode_len(String *str);


#endif //__str_H_
