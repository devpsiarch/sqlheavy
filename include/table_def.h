#ifndef TABLE_DEF_H
#define TABLE_DEF_H

#include "pager.h"

typedef struct {
    size_t num_attri;
    String name_of_table;         // used to identify/index the table when many exist
    String* expression;           // this is a da of strings that experss the content of the rows in this database.
                                  // exmaple : "%d" "%s" "%lf" the db has a (int,string,double) attributes 
                                  // stored in memry linearly in this order. there for each row's size is 
                                  // 4 + 8 + 8 = 20 bytes.
    String* attributes;           // these are the names of said attributes for take the above example : 
                                  // "ID" "Name" "Balance". 
    unsigned int SIZE_ROW;        // its a bit redundent to have this , but since table and row stucts never really meet then 
                                  // it does not matter much , for now this stays , if i find something better sure.
    unsigned int ROWS_PER_PAGE;   // tells us how many rows per page , like duh
    unsigned int TABLE_MAX_ROWS;  // self explained.
    size_t count_rows;            // tells us the current row we are int , acts like the count for da.
    
    Pager* pager;                 // since for now the data is stored in memory , but this is where the data will live
                                  // in the form of pages that we index and find the specific row we want 
}Table;

// let me explain how would we handle the rows in some table
// each table has a number of rows ordered in a set of pages 
// the datastructure Row which will encode the datawe are trying to write/read 
// or store is stored in binary form no matter the length or the type of data inside 
// and only with the String*expression we can decode it 
// example : 0x123456 is 3 bytes and with expression "%c" "%c" "%c"
// that is the database stores 3 chars with names dictated by the table



#endif
