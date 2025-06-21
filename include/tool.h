#ifndef TOOL_H
#define TOOL_H

#include <stdio.h>
#include <ctype.h>
#include <math.h>

#define ASSERT(cond,...)                                                \
do{                                                                     \
    if(!(cond)){                                                        \
        fprintf(stderr,"%s,%d: ASSERTION FAILED: ",__FILE__,__LINE__);  \
        fprintf(stderr,__VA_ARGS__);                                    \
        fprintf(stderr,"\n");                                           \
        exit(1);                                                        \
    }                                                                   \
}while(0)


#define return_defer(v) \
do{                     \
    result = (v);       \
    goto defer;         \
}while(0)               

#define INIT_DA_CAPACITY 1024

#define da_realloc(da)                                                                \
do{                                                                                   \
    if((da)->count >= (da)->capacity){                                                \
        (da)->capacity = ((da)->capacity == 0)? INIT_DA_CAPACITY : (da)->capacity*2;  \
        (da)->items = realloc((da)->items,(da)->capacity*sizeof(*(da)->items));       \
        ASSERT((da)->items != NULL,"NOT ENOUGH MEMORY\n");                            \
    }                                                                                 \
}while(0)

#define da_append(da,value)                                                     \
do{                                                                             \
    da_realloc((da));                                                           \
    (da)->items[(da)->count++] = (value);                                       \
}while(0)

#define da_flush(da) (da)->count = 0; 

#define da_free(da)     \
do {                    \
    free((da)->items);  \
    (da)->items = NULL; \
    (da)->capacity = 0; \
    (da)->count = 0;    \
}while (0)

#define string_append(str,value)        \
do {                                    \
    da_append((str),(value));           \
    (str)->items[(str)->count] = '\0';  \
}while(0)

#define string_pop(str)                      \
do {                                         \
    if ((str)->count-1 >= 0) {               \
        (str)->items[(str)->count-1] = '\0'; \
        (str)->count--;                     \
    }                                       \
}while(0)


typedef struct {
    char *items;
    size_t capacity;
    size_t count;
}String;

typedef struct {
    int *items;
    size_t capacity;
    size_t count;
}int_arr;

bool stoi(const char*str,int*ans);
float stof(const char *s);

#endif
#ifndef TOOL_IMPLI
#define TOOL_IMPLI

bool stoi(const char *str, int *ans) {
    bool result = true;
    size_t i = 0;
    int sign = 1;

    // Empty string? fail immediately
    if (str[0] == '\0')
        return false;

    // Handle optional sign
    if (str[0] == '-' || str[0] == '+') {
        if (str[0] == '-')
            sign = -1;
        i++;
        // “-” or “+” only with no digits is invalid
        if (str[i] == '\0')
            return false;
    }

    *ans = 0;
    for (; str[i] != '\0'; i++) {
        unsigned char c = (unsigned char)str[i];
        if (!isdigit(c))
            return_defer(false);

        *ans = (*ans * 10) + (c - '0');
    }

    *ans *= sign;

defer:
    return result;
}

float stof(const char *s) {
    // 1) Skip leading whitespace
    while (isspace((unsigned char)*s)) {
        s++;
    }

    // 2) Handle optional sign
    int sign = 1;
    if (*s == '+' || *s == '-') {
        if (*s == '-') sign = -1;
        s++;
    }

    // 3) Parse integer part
    float result = 0.0f;
    while (isdigit((unsigned char)*s)) {
        result = result * 10.0f + (*s - '0');
        s++;
    }

    // 4) Parse fractional part, if any
    if (*s == '.') {
        s++;
        float place = 1.0f;
        while (isdigit((unsigned char)*s)) {
            place *= 0.1f;
            result += (*s - '0') * place;
            s++;
        }
    }

    // 5) Parse exponent part, if any
    if (*s == 'e' || *s == 'E') {
        s++;
        int exp_sign = 1;
        if (*s == '+' || *s == '-') {
            if (*s == '-') exp_sign = -1;
            s++;
        }
        int exp_val = 0;
        while (isdigit((unsigned char)*s)) {
            exp_val = exp_val * 10 + (*s - '0');
            s++;
        }
        result *= powf(10.0f, exp_sign * exp_val);
    }

    return sign * result;
}

#endif
