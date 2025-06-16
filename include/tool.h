#ifndef TOOL_H
#define TOOL_H

#include <stdio.h>

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

#endif
#ifndef TOOL_IMPLI
#define TOOL_IMPLI

#endif
