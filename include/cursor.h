#ifndef CURSOR_H
#define CURSOR_H

#include "defs.h"

// the curor implimentation basicly is a abstraction of an indicator 
// to some row in some table , we use this to index and find rows to 
// perfom operations on instrad of sending a integer to index 

// when we init a cursor it can either be at the 
// end or not , so instead of making two functionalitis , we pass one 
// of these for the initer to know
typedef enum {
    START_TABLE,
    END_TABLE
}Table_position;

typedef struct {
    size_t current_row;
    bool at_end;
    Table* t;
}cursor;

// cursor's are bind to a table to act as indicator to perform 
// something 
#define UNBIND(cursor) (cursor)->t = NULL;

// binds a cursor to a table no matter its arch
cursor* init_cursor(Table*t,Table_position p);
// kills and resets the cursor without doing anything to the 
// table
void kill_cursor(cursor*c);

// walks forward in the data structure for now the:array
void cursor_advance(cursor*c);
// get the row of on the current_row
void* cursor_value(cursor*c);

#endif
#ifndef CURSOR_IMPLI 
#define CURSOR_IMPLI

cursor* init_cursor(Table*t,Table_position p){
    cursor* bind = malloc(sizeof(cursor));
    bind->t = t;
    switch(p){
        case START_TABLE:
            bind->current_row = 0;
            bind->at_end = (t->count_rows  == 0);
            break;
        case END_TABLE:
            bind->current_row = t->count_rows;
            bind->at_end = true;
            break;
        default:
            return NULL;
            break;
    }
    return bind;
}

void kill_cursor(cursor*c){
    UNBIND(c); 
    free(c);
    c = NULL;
}

void cursor_advance(cursor*c){
    c->current_row += 1;
    if(c->current_row >= c->t->count_rows) c->at_end = true;
}
void* cursor_value(cursor*c){
    size_t row_num = c->current_row;
    unsigned int page_num = row_num / c->t->ROWS_PER_PAGE;
    void* page = get_page(c->t->pager,page_num); 
    unsigned int row_offset = row_num % c->t->ROWS_PER_PAGE;
    unsigned int bytes_offset = row_offset * c->t->SIZE_ROW;
    return (char*)page + bytes_offset;
}

#endif
