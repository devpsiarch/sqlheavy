#ifndef TABLE_H
#define TABLE_H

#include "tool.h"
#include <stdarg.h>
#include <string.h>

#define MAX_SIZE_STR 255    // for now i will allow only string that are 255 size in bytes.

typedef struct {
    size_t num_attri;
    String name_of_table;   // used to identify/index the table when many exist
    String* expression;     // this is a da of strings that experss the content of the rows in this database.
                            // exmaple : "%d" "%s" "%lf" the db has a (int,string,double) attributes 
                            // stored in memry linearly in this order. there for each row's size is 
                            // 4 + 8 + 8 = 20 bytes.
    String* attributes;     // these are the names of said attributes for take the above example : 
                            // "ID" "Name" "Balance". 
}Table;

// let me explain how would we handle the rows in some table
// each table has a number of rows ordered in a set of pages 
// the datastructure Row which will encode the datawe are trying to write/read 
// or store is stored in binary form no matter the length or the type of data inside 
// and only with the String*expression we can decode it 
// example : 0x123456 is 3 bytes and with expression "%c" "%c" "%c"
// that is the database stores 3 chars with names dictated by the table

#ifdef _WIN32
    // Windows-specific code
    #pragma pack(push, 1)
    typedef struct {
        size_t size_bytes;
        void *data;
    } Row;
    #pragma pack(pop)
#else
    // Non-Windows code (Linux, macOS, etc.)
    typedef struct __attribute__((packed)) {
        size_t size_bytes;
        void *data;
    } Row;
#endif

// bases on the types of the expression returns the const size for said row
size_t calculate_size_row(String*expression,unsigned int count);

Row* init_row(String*expression,unsigned int count);
void kill_row(Row*r);


// give it values it gives back a representaion of a row
void serilize_row(Row*dst,String*expression,const unsigned int count,...);
// give it a row representaion and pointer to datamemebers and it will fill them out for you
void deserilize_row(const Row*src,String*expression,const unsigned int count,...);

Table* init_table(const unsigned int count,...);
void kill_table(Table*t);

#endif
#ifndef TABLE_IMPLI 
#define TABLE_IMPLI


size_t calculate_size_row(String*expression,unsigned int count){
    size_t current_size = 0;
    for(unsigned int i = 0 ; i < count ; i++){
        if(expression[i].items[0] != '%') goto fail;
        switch (expression[i].items[1]) {
            case 'd':
                current_size += sizeof(int);
                break;
            case 'f':
                current_size += sizeof(float);
                break;
            case 'c':
                current_size += sizeof(char);
                break;
            case 's':
                current_size += MAX_SIZE_STR;
                break;
            default:
                goto fail;
                break;
        }
    }
    return current_size;
fail:
    // this indicated error since we cant have something 
    // that takes up 0 memory
    return 0;
}

Row* init_row(String*expression,unsigned int count){
    size_t current_size = calculate_size_row(expression,count);
    if(current_size == 0){
        return NULL;
    }
    Row* created = malloc(sizeof(Row));
    created->size_bytes = current_size;
    if(created == NULL){
        return NULL; 
    }
    created->data = malloc(current_size);
    if(created->data == NULL){
        free(created);
        return NULL;
    }
    return created;
}
void kill_row(Row*r){
    if(r != NULL) free(r->data);
    free(r);
    r = NULL;
}

// WARNING: This functions fails the moment a invalid input is passed 
// that is , failuire of this function is the failiure all of the program
// for now this behavior will stay 
// although we hope to change it , it for the compiler to be carful when calling it 
// once create functionality is added
Table* init_table(const unsigned int count,...){
    bool result = true;
    Table* obj = malloc(sizeof(Table));
    if(obj == NULL) return_defer(NULL);
    obj->num_attri = count;
    obj->expression = malloc(sizeof(String)*count);
    if(obj->expression == NULL) return_defer(NULL);
    obj->attributes = malloc(sizeof(String)*count);
    if(obj->attributes == NULL) return_defer(NULL);
    char*next_type;
    char*next_name;
    va_list args;
    va_start(args,count);
    for(unsigned int i = 0 ; i < count ; i++){
        next_type = va_arg(args,char*);
        next_name = va_arg(args,char*);
        
        // printf("%s\n",next_type);
        // printf("%s\n",next_name);

        obj->expression[i].items = malloc(strlen(next_type)+1);
        if(obj->expression[i].items == NULL) return_defer(false);
        strcpy(obj->expression[i].items,next_type);
        obj->attributes[i].items = malloc(strlen(next_name)+1);
        if(obj->attributes[i].items == NULL) return_defer(false);
        strcpy(obj->attributes[i].items,next_name);
    }
    va_end(args);
defer:
    if(result == false){
        kill_table(obj);
        va_end(args);
        return NULL;
    } 
    return obj;
}

void kill_table(Table *t) {
    if (t == NULL) 
        return;  // 1) guard against NULL pointer
    // 2) only loop over attributes/expression if both pointers are non-NULL
    if (t->attributes && t->expression) {
        // If attributes and expression have the same length:
        for (size_t i = 0; i < t->num_attri; i++) {
            da_free(&t->attributes[i]);
            da_free(&t->expression[i]);
        }
    }

    // 3) free the arrays themselves if they exist
    free(t->attributes);
    free(t->expression);

    // 4) finally free the Table struct itself
    free(t);
}

// this expects the number of elemnts in expression to be the same as count 
// if not ? it will exit with code 1
// WARNING: this overrides the values we had before , so be warry
void serilize_row(Row*dst,String*expression,const unsigned int count,...){
    bool not_matching_size = false;
    bool wrong_format_expression = false;
    bool exceeded_max_str_size = false;
    va_list args;
    va_start(args,count);
    // records the size at this point of some row , grow with each 
    // data member
    dst->size_bytes = 0; 
    for(unsigned int i = 0 ; i < count ; i++){
        if(expression[i].items[0] != '%'){
            wrong_format_expression = true;
            goto fail;
        }
        // match every (availble for now) type of data can be stored
        switch (expression[i].items[1]) {
            // if var args failed it will crash the prog
            case 'd': {
                int val = va_arg(args,int);
                memcpy(dst->data+dst->size_bytes,&val,sizeof(int));
                dst->size_bytes+=sizeof(int);
                break;
            }
            case 'f': {
                // va_args promotes type float to dounbe
                float val = (float)va_arg(args,double);
                memcpy(dst->data+dst->size_bytes,&val,sizeof(float));
                dst->size_bytes+=sizeof(float);
                break;
            }
            case 'c': {
                // and char , short .. to int
                char val = (char)va_arg(args,int);
                memcpy(dst->data+dst->size_bytes,&val,sizeof(char));
                dst->size_bytes+=sizeof(char);
                break;
            }
            case 's': {
                char *val = va_arg(args, char*);
                if (!val) {
                    exceeded_max_str_size = true;
                    goto fail;
                }

                size_t str_len = strlen(val);
                // Need +1 so there's room for the NUL
                if (str_len + 1 > MAX_SIZE_STR) {
                    exceeded_max_str_size = true;
                    goto fail;
                }

                // Copy the string _with_ its terminator
                memcpy(dst->data + dst->size_bytes, val, str_len + 1);
                // Zero‐pad any remainder
                memset(dst->data + dst->size_bytes + str_len + 1,
                       0,
                       MAX_SIZE_STR - (str_len + 1));

                dst->size_bytes += MAX_SIZE_STR;
                break;
            }
            default:
                fprintf(stderr,"type/attribute %c not yet implimented.\n",expression[i].items[1]);
                exit(1);
                break;
        }        
    }
    va_end(args);
    return;
fail:
    va_end(args);
    if(not_matching_size) fprintf(stderr,"Not matching sized when serilizing a row.\n");
    if(wrong_format_expression) fprintf(stderr,"wrong format expression.\n");
    if(exceeded_max_str_size) fprintf(stderr, "exceeded the maximum size for a string.\n");
    exit(1);
}

// we pass a Row as src and pass somepointers to extract the data to those pointers 
// WARNING: be sure to pass a correct arguments.
// This function shall not alter the state of the Row*src
void deserilize_row(const Row*src,String*expression,const unsigned int count,...){
    bool wrong_format_expression = false;
    va_list args;
    va_start(args,count);
    size_t indexer = 0;     // this will tell us where we start reading/copying from the row
    for(unsigned int i = 0 ; i < count ; i++){
        if(expression[i].items[0] != '%'){
            wrong_format_expression = true;
            goto fail;
        }
        switch(expression[i].items[1]){
            case 'd': {
                int* ref = va_arg(args,int*);
                memcpy(ref,src->data+indexer,sizeof(int));
                indexer += sizeof(int);
                break;
            }
            case 'f': {
                float* ref = va_arg(args,float*);
                memcpy(ref,src->data+indexer,sizeof(float));
                indexer += sizeof(float);
                break;
            }
            case 'c': {
                char* ref = va_arg(args,char*);
                memcpy(ref,src->data+indexer,sizeof(char));
                indexer += sizeof(char);
                break;
            }
            case 's': {
                char* ref = va_arg(args,char*);
                memcpy(ref,src->data+indexer,MAX_SIZE_STR);
                indexer += MAX_SIZE_STR;
                break;
            }
            default:
                fprintf(stderr,"type/attribute \"%c\" not yet implimented.\n",expression[i].items[1]);
                exit(1);
                break;
        }
    }
    va_end(args);
    return;
fail:
    va_end(args);
    if(wrong_format_expression) fprintf(stderr,"wrong format expression.\n");
    exit(1);
}

#endif
