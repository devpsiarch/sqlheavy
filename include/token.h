#ifndef TOKEN_H
#define TOKEN_H

#include "tool.h"
#include <ctype.h>
#include <string.h>

#define DELIM " "

// hey dumy these require a small mod to the parser.

// TODO: the command "CREATE" should be implimentented later
// ALSO: '..' token to indicate a series of ints
typedef enum {
    SELECT,
    INSERT,
    ID,
    ALL,
    FROM,
    TABLE,
    SEMI,
    OPEN_PAR,
    CLOSE_PAR,
    COMMA,
    UNKNOWN
}token_t;



typedef struct {
    token_t type;   // type of said token
    char*lexeme;    // owned , used to get the value 
    size_t col;     // usefull to display where the syntax error happened 
}Token;

#define Token_free(t) \
do{                   \
    free((t)->lexeme);\
    (t)->lexeme=NULL; \
}while(0)     

// for convinence and to use the tool.h macros
typedef struct {
    Token* items;
    size_t count;
    size_t capacity;
}arr_tokens;

#define toke_append(da,t,l,c)                   \
do{                                             \
    Token temp = {                              \
        .type = (t),                            \
        .lexeme = (l),                          \
        .col = (c),                             \
    };                                          \
    da_append((da),(temp));                     \
}while(0)

#define toke_arr_free(da)                     \
do{                                           \
    for(size_t i = 0 ; i < (da)->count ; i++){\
        Token_free(&(da)->items[i]);          \
    }                                         \
    free((da)->items);                        \
    (da)->items = NULL;                       \
    (da)->count = 0;                          \
    (da)->capacity = 0;                       \
}while(0)

// takes a string from the user input and retuns an array 
// of tokens
arr_tokens* parser(const char*str);

#endif
#ifndef TOKEN_IMPLI 
#define TOKEN_IMPLI


arr_tokens* parser(const char*str){
    arr_tokens* result = malloc(sizeof(arr_tokens));
    result->items = NULL;
    result->count = 0;
    result->capacity = 0;
    
    size_t current_col = 0;
    const char*p = str;
    
    while(*p){
        // skip white space
        if(isspace((unsigned char)*p)){
            p++;
            current_col++;
            continue;
        }
        // init attributes 
        size_t tok_start = current_col;
        token_t type = UNKNOWN;
        char* lexeme = NULL;
        // handle one char elements
        switch(*p) {
            case '*': type = ALL; break;
            case ';': type = SEMI; break;
            case '(': type = OPEN_PAR; break;
            case ')': type = CLOSE_PAR; break;
            case ',': type = COMMA; break;
            default:
                if(isdigit(*p)) type = ID;
                break;
        }
        if(type != UNKNOWN){
            lexeme = malloc(2);
            if(lexeme){
                lexeme[0] = *p;
                lexeme[1] ='\0';
                toke_append(result,type,lexeme,tok_start);
            }
            p++;
            current_col++;
            continue;
        }
        // hanlde identifier or keyword 
        if(isalpha((unsigned char)*p) || *p == '_'){
            const char* start = p;
            while(*p && (isalnum((unsigned char)*p) || *p == '_')){
                p++;
                current_col++;
            }
            size_t len = p - start;
            lexeme = malloc(len+1);
            if(lexeme){
                strncpy(lexeme, start, len);
                lexeme[len] = '\0';
                
                // Determine token type
                if (strcasecmp(lexeme, "select") == 0) type = SELECT;
                else if (strcasecmp(lexeme, "insert") == 0) type = INSERT;
                else if (strcasecmp(lexeme, "from") == 0) type = FROM;
                else if (strcasecmp(lexeme, "table") == 0) type = TABLE;
                else type = ID;
            
                toke_append(result,type,lexeme,tok_start);
            }
            continue;
        }
        // Handle unknown tokens (single character)
        lexeme = malloc(2);
        if (lexeme) {
            lexeme[0] = *p;
            lexeme[1] = '\0';
            toke_append(result, UNKNOWN, lexeme, tok_start);
        }
        p++;
        current_col++;
    }
    return result;
}

#endif
