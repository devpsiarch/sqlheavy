#ifndef TABLE_H
#define TABLE_H

#include "tool.h"
#include <stdarg.h>
#include <string.h>

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

Table* init_table(const unsigned int count,...);
void kill_table(Table*t);

#endif
#ifndef TABLE_IMPLI 
#define TABLE_IMPLI


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

#endif
