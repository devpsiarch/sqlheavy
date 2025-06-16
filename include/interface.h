#ifndef INTERFACE_H
#define INTERFACE_H

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "tool.h"
#include "cmd_proc.h"

#define PRINT_PROMPT() printf("db => ");

bool read_input(String*str);
bool db_interactive(void);

/*
 *  show (1,2,...,ALL) from table;
 *  tokinized => [SHOW,LETERAL,FROM,TABLE_LITERAL]
 *  select (1,2,....,ALL) from table;
 *  insert (data1,...,datan) to table;
 *  => [INSERT,]
 *  create table {name} (schema);
 *  map<name_of_table,tableptr>
 * */

/*
 * Statement = { 
 *      type_of_statement = [insert ? ...]
 *      Token* tokens; if it is a insert , this will produce a row
 * }    
 * 
 *  Token {
 *      type_of_token; = [FROM,SEMI...]
 *      row; in the source command
 *      col; 
 *  }
 *  // acculy executes the statement
 *  vm_execute_bytecode(Statement*s){
 *      check_type_of_statament(s);
 *      execute_in_real_time() if unexpected ? 
 *          return a status code 
 *          either syntax (insert () missing?->{to} table)
 *  }
 *  execute meta commands sthat start with .
 *  vm_execurte_builtins(Statement*s)
 * */

/*
 * table_schema {
 *      string* expression;
 *      string* names;
 *      this only contains strings that can be interprited 
 *      by the sscanf 
 *      for examples a user table_schema would look like
 *      expression = "%d" "%s" "%s" 
 *      names = "id" "email" "password"
 *      which the db understands as 
 *      (somehing int,something string,something string)
 * }
 * 
 * row {
 *      size_t size; tells me how much bytes does this row take
 *      string* expression; 
 * }
 * 
 *  read_row(row*r){
 *      for expr in r->expression {
 *          match(*expr) {
 *              "%d" => {
 *                  int result;
 *                  memcpy(&result,r,sizeof(int));
 *              } 
 *              ...
 *              _ => {
 *                  WTF
 *              }
 *          }
 *      }
 *  }
 *
 * */

/*
 *  pager {
 *      pages[MAX_NUM_FOR_PAGES]; this is basicly cache
 *      int fd; for the file being written to
 *      size_t file_length; in bytes
 *  }
 *  table {
 *      pager*;
 *      size_t total_rows_for_this_table;
 *
 *  }
 *
 *  db {
 *      tables*;
 *  }
 * */

#endif
#ifndef INTERFACE_IMPLI 
#define INTERFACE_IMPLI


bool read_input(String*str){
    ssize_t bytes_read = getline(&str->items,&str->count,stdin);
    if(bytes_read <= 0){
        printf("Error reading line... exiting.");
        return false;
    }
    str->count = (size_t)(bytes_read-1);
    str->items[bytes_read-1] = 0;
    return true;
}

bool db_interactive(void){
    bool result = true;
    String command = {
        // getline allocates and resises auto
        .items = NULL,
        .capacity = 50,
        .count = 0
    };
    PRINT_HELP();
    while(true){
        PRINT_PROMPT(); 
        if(read_input(&command) == false) return_defer(false);
        
        // handle the input 
        if(command.items[0] == '.'){
            switch(vm_execute_meta(&command)){
                case META_SUCCESS:
                    printf("Meta command executed with success.\n");
                    break;
                case META_FAIL:
                    printf("Meta command not reconized.\n");
                    break;
                case META_EXIT:
                    printf("Goodbye.\n");
                    goto defer;
                    break;
                default:
                    exit(1);
                    break;
            }
        }else{
            switch(vm_execute_query(&command)){
                case QUERY_SUCCESS:
                    break;
                case QUERY_FAIL:
                    printf("Query not reconized.\n");
                    break;           
                default:
                    exit(1);
                    break;
            }
        }
    }
defer:
    da_free(&command);
    return result;
}
#endif
