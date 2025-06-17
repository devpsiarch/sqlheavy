#ifndef CMD_PROC_H
#define CMD_PROC_H

#include "token.h"

#define PRINT_HELP()                            \
do{                                             \
    printf("No manual is provided for now\n");  \
}while(0)   

typedef enum {
    META_SUCCESS = 0,
    META_FAIL = 1,
    META_EXIT = 2
}MetaResult;

typedef enum {
    QUERY_SUCCESS = 3,
    QUERY_FAIL = 4
}QueryResult;

MetaResult vm_execute_meta(String*str);
QueryResult vm_execute_query(String*str);

#endif
#ifndef CMD_PROC_IMPLI 
#define CMD_PROC_IMPLI

MetaResult vm_execute_meta(String*str){
    if(strcasecmp(str->items,".exit") == 0){
        return META_EXIT;
    }else if(strcasecmp(str->items,".help") == 0){
        PRINT_HELP();
        return META_SUCCESS;
    }else{
        return META_FAIL;
    }
} 
QueryResult vm_execute_query(String*str){
    arr_tokens tokens = parser(str->items);
    QueryResult result = QUERY_SUCCESS;
    // TODO: in the fure , we allows the user to input more commands
    // if there is no semicolen at the end 
    // for new we consider it as a error
    if(tokens.items[tokens.count-1].type != SEMI){
        printf("MISSING SEMICOLEN AT QUERY\n");
        return_defer(QUERY_FAIL);
    }
    // match the type of the command , for now we have 
    // 2 , "INSERT" and "SELECT" well add the "CREATE" later 
    // to hanlde the creation of new tables
    switch(tokens.items[0].type){
    case INSERT:
        printf("it is insert command\n");
        break;
    case SELECT:
        // we expect (select , id , from , table, ;)
        if(tokens.items[2].type != FROM){
            printf("Invalid syntax , missing \"from\" in col %zu\n",tokens.items[2].col);
            return_defer(QUERY_FAIL);
        }
        if(tokens.items[3].type != ID){
            printf("Invalid syntax , missing table name in col %zu\n",tokens.items[3].col);
            return_defer(QUERY_FAIL);
        }
        // look up if this table exists
        break;
    default:
        return_defer(QUERY_FAIL);
        break;
    }
defer:
    toke_arr_free(&tokens);
    return result;
}

#endif
