#ifndef CMD_PROC_H
#define CMD_PROC_H

#include "token.h"
#include "table.h"

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
QueryResult vm_execute_query(Table*t,String*str);

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
QueryResult vm_execute_query(Table*t,String*str){
    arr_tokens* tokens = parser(str->items);
    QueryResult result = QUERY_SUCCESS;
    if(str->items[0] == '\0') return_defer(QUERY_FAIL);
    // TODO: in the fure , we allows the user to input more commands
    // if there is no semicolen at the end 
    // for new we consider it as a error
    if(tokens->items[tokens->count-1].type != SEMI){
        printf("MISSING SEMICOLEN AT QUERY\n");
        return_defer(QUERY_FAIL);
    }
    // match the type of the command , for now we have 
    // 2 , "INSERT" and "SELECT" well add the "CREATE" later 
    // to hanlde the creation of new tables
    switch(tokens->items[0].type){
    case INSERT: {
        size_t current_token = 1;
        Row*r = init_row(t);
        size_t offset = 0;
        if(tokens->items[current_token].type == OPEN_PAR){
            current_token++;
            size_t num_expected_attri = t->num_attri;
            while(tokens->items[current_token].type != CLOSE_PAR){
                // populate the row depending on the type.
                switch(tokens->items[current_token].type){
                    case INT:{
                        int val;
                        if(stoi(tokens->items[current_token].lexeme,&val) == false){
                            printf("Error in converting to in while interpreting query\n");
                            goto defer_insert;
                        }
                        memcpy(r->data+offset,&val,sizeof(int));
                        offset += sizeof(int);
                        break;
                    }
                    case CHAR:{
                        char val = *tokens->items[current_token].lexeme;
                        memcpy(r->data+offset,&val,sizeof(char));
                        offset += sizeof(char);
                        break;
                    }
                    case FLOAT:{
                        float val = stof(tokens->items[current_token].lexeme);
                        memcpy(r->data+offset,&val,sizeof(float));
                        offset += sizeof(float);
                        break;
                    }
                    case STRING:{
                        size_t len = strlen(tokens->items[current_token].lexeme);
                        memcpy(r->data+offset,tokens->items[current_token].lexeme,len);
                        memset(r->data+offset+len,0,MAX_SIZE_STR-len);
                        offset += MAX_SIZE_STR;
                        break;
                    }
                    default:
                        printf("Unexpected type ... \n");
                        goto defer_insert;
                        break;
                }
                current_token++;
                num_expected_attri--;
                if(tokens->items[current_token].type != COMMA){
                    if(tokens->items[current_token].type == CLOSE_PAR) continue;
                    else{
                        printf("Missing the closing parenthesis for insertion on col %zu.\n",
                        tokens->items[current_token].col);
                        goto defer_insert;
                    }
                }else{
                    current_token++;
                }
            }
            if(num_expected_attri != 0){
                printf("Invalid number of attributes , exprected %zu gotten %zu\n",
                t->num_attri,t->num_attri-num_expected_attri);
                goto defer_insert;
            }
            current_token++;
            if(tokens->items[current_token].type != INTO){
                printf("Missing \"into\" instruction at col %zu\n",tokens->items[current_token].col);
                goto defer_insert;
            }
            current_token++;
            if(tokens->items[current_token].type != ID){
                printf("Invalid syntax , missing table name in col %zu\n",tokens->items[3].col);
                goto defer_insert;
            }
            // look up if this table exists
            // for now we only accept the table "users".    
            // this section is supposed it be for the future a search 
            // and indicate for table namespace 
            if(strcmp(tokens->items[current_token].lexeme,"users") != 0){
                printf("Table \"%s\" not found in database.\n",tokens->items[current_token].lexeme);
                goto defer_insert;
            }
            // we perform the job here
            write_row_dyn(t,r);
        }else{
            goto defer_insert;
        }


        kill_row(r);
        return_defer(QUERY_SUCCESS);
        defer_insert:
           kill_row(r);
           return_defer(QUERY_FAIL);
        break;
    }
    case SELECT : {
        // check if we only passed "select;"
        if(tokens->count == 2){
            printf("Can deduce intruction , too few tokens in col %zu\n",tokens->items[1].col);
            return_defer(QUERY_FAIL);
        }
        // we expect (select , id ... , from , table, ;)
        size_t current_token = 1;
        int_arr rows_wanted = {0};
        bool show_all = false;
        while(tokens->items[current_token].type == INT){
            int res;
            if(stoi(tokens->items[current_token].lexeme,&res) == false){
                printf("Invalid identifier \"%s\".\n",tokens->items[current_token].lexeme);
                goto defer_select;
            }
            da_append(&rows_wanted,res);
            current_token++;
        }
       
        if(tokens->items[current_token].type == ALL){
            show_all = true;
            current_token++;
        }

        if(tokens->items[current_token].type != FROM){
            printf("Invalid syntax , missing \"from\" in col %zu\n",tokens->items[2].col);
            return_defer(QUERY_FAIL);
        }
        current_token++;
        if(tokens->items[current_token].type != ID){
            printf("Invalid syntax , missing table name in col %zu\n",tokens->items[3].col);
            return_defer(QUERY_FAIL);
        }
        // look up if this table exists
        // for now we only accept the table "users".    
        // this section is supposed it be for the future a search 
        // and indicate for table namespace 
        if(strcmp(tokens->items[current_token].lexeme,"users") != 0){
            printf("Table \"%s\" not found in database.\n",tokens->items[current_token].lexeme);
            goto defer_select;
        }
        // here we loop through the tables and seach by the rows that
        // we got from the command
        if(show_all){
            // for i from 0 to table.num_rows print row 
            outs_package outs;
            INIT_OUTS(t,outs);
            for(size_t i = 0 ; i < t->count_rows ; i++){
                read_row_dyn(t,i,&outs);
                PRINT_OUTS(t,outs);
            }
            FREE_OUTS(t,outs);   
        }else{
            for(size_t i = 0 ; i < rows_wanted.count ; i++){
                // call the select row by number rows_wanted.items[i]
                // call deserilize 
                // print output in format
                //read_row(t,i,t->num_attri,outs);
                //print_outs(outs);
                outs_package outs;
                INIT_OUTS(t,outs);
                read_row_dyn(t,rows_wanted.items[i],&outs);
                PRINT_OUTS(t,outs);
                FREE_OUTS(t,outs);
            }  
        }

    defer_select:
        da_free(&rows_wanted);
        break;
    }
    default:
        return_defer(QUERY_FAIL);
        break;
    }
defer:
    toke_arr_free(tokens);
    free(tokens);
    return result;
}

#endif
