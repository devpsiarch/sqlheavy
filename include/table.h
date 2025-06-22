#ifndef TABLE_H
#define TABLE_H

#include "tool.h"
#include <stdarg.h>
#include <string.h>
#include <assert.h>
#include "pager.h"

#define MAX_SIZE_STR 255    // for now i will allow only string that are 255 size in bytes.


 

#define UNIMPLIMENTED_TYPE(c)                                           \
do{                                                                     \
    fprintf(stderr,"type/attribute \"%c\" not yet implimented.\n",(c)); \
}while(0)                       

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

// given a table (there for a expression) it print the data using printf to stdout.
// used mostly for debugging and stuff.
void print_varadic_expression_ref(Table*t,va_list args);


Row* init_row(Table*t);
void kill_row(Row*r);

// bases on the types of the expression returns the const size for said row
size_t calculate_size_row(String*expression,unsigned int count);
// give it values it gives back a representaion of a row
void serilize_row(Row*dst,String*expression,const unsigned int count,...);
// version that takes va_list that the above calls
void serilize_row_v(Row*dst,String*expression,const unsigned int count,va_list args);
// give it a row representaion and pointer to datamemebers and it will fill them out for you
void deserilize_row(const Row*src,String*expression,const unsigned int count,...);
// same memo 
void deserilize_row_v(const Row*src,String*expression,const unsigned int count,va_list args);

Table* init_table(const unsigned int count,...);
void kill_table(Table*t);


// for this function , we will give it a table (already initilized) and 
// a number of row we want to read from it , it shall : 
// - find the page the row is in 
// - allocated page if not allocated 
// - get the pointer to the start of the row for the API to extract
void* row_select(Table* t,unsigned int row_num);

// this function shall write the serilized representation of a row 
// for the first availble position in the table.
// this behavior will remain for now untile further notice.
// it shall: 
// - create a row on the heap from varadic values
// - copy its serilized to the first availble position using row_select
void write_row(Table*t,unsigned int count,...);

// this shall read only from the rows of the table 
// when provided with a row_num
void read_row(Table*t,unsigned int row_num,unsigned int count,...);


// alternate versions of the above that do the exact same only 
// that we send them pointers to atrbitrary types then the functions fills them up 
// and lets us decode them using the expression string 

typedef struct {
    void**ptr;
}outs_package;

/* Convenience macro to handle an out‑of‑memory error */
#define OOM_CHECK(p)                              \
    do { if (!(p)) {                              \
            fprintf(stderr, "Out of memory\n");  \
            exit(EXIT_FAILURE);                  \
        } } while (0)

/* INIT_OUTS: allocates the outs.ptr array and a buffer for each column */
#define INIT_OUTS(T, outs)                                                \
    do {                                                                  \
        size_t _n = (T)->num_attri;                                       \
        (outs).ptr = malloc(_n * sizeof *(outs).ptr);                     \
        OOM_CHECK((outs).ptr);                                            \
        for (size_t i = 0; i < _n; i++) {                                 \
            char _fmt = (T)->expression[i].items[1];                      \
            switch (_fmt) {                                               \
                case 'c':                                                 \
                    (outs).ptr[i] = malloc(sizeof(char));                 \
                    break;                                                \
                case 'd':                                                 \
                    (outs).ptr[i] = malloc(sizeof(int));                  \
                    break;                                                \
                case 'f':                                                 \
                    (outs).ptr[i] = malloc(sizeof(float));                \
                    break;                                                \
                case 's':                                                 \
                    (outs).ptr[i] = malloc(MAX_SIZE_STR);                 \
                    break;                                                \
                default:                                                  \
                    /* clean up anything we’ve already malloc’d */        \
                    for (size_t _j = 0; _j < i; _j++)                     \
                        free((outs).ptr[_j]);                             \
                    free((outs).ptr);                                     \
                    fprintf(stderr, "Unimplemented type: %c\n", _fmt);    \
                    exit(EXIT_FAILURE);                                   \
            }                                                             \
            OOM_CHECK((outs).ptr[i]);                                     \
        }                                                                 \
    } while (0)


// given a serilized row , this will populate outs bases on the row
#define POPULATE_OUTS(t,outs,r,i,offset)                                \
do{                                                                     \
    char fmt = (t)->expression[(i)].items[1];                           \
    switch (fmt) {                                                      \
    case 'c':                                                           \
        *(char*)(outs)->ptr[(i)] = *(char*)((r)->data + (offset));      \
        (offset) += sizeof(char);                                       \
        break;                                                          \
    case 'd':                                                           \
        memcpy((outs)->ptr[(i)], (r)->data + (offset), sizeof(int));    \
        (offset) += sizeof(int);                                        \
        break;                                                          \
    case 's':                                                           \
        memcpy((outs)->ptr[(i)], (r)->data + (offset), MAX_SIZE_STR);   \
        (offset) += MAX_SIZE_STR;                                       \
        break;                                                          \
    case 'f':                                                           \
        memcpy((outs)->ptr[(i)], (r)->data + (offset), sizeof(float));  \
        (offset) += sizeof(float);                                      \
        break;                                                          \
    default:                                                            \
        UNIMPLIMENTED_TYPE(fmt);                                        \
        exit(1);                                                        \
        break;                                                          \
    }                                                                   \
}while(0)                                                   

/* FREE_OUTS: frees each buffer and then the ptr array itself */
#define FREE_OUTS(T, outs)                       \
    do {                                         \
        size_t _n = (T)->num_attri;              \
        for (size_t _i = 0; _i < _n; _i++)       \
            free((outs).ptr[_i]);               \
        free((outs).ptr);                       \
        (outs).ptr = NULL;                      \
    } while (0)


#define PRINT_OUTS(T,outs)                                                          \
do{                                                                                 \
    for(size_t i = 0 ; i < (T)->num_attri ; i++){                                   \
        char fmt = (T)->expression[i].items[1];                                     \
        switch(fmt){                                                                \
            case 'd':                                                               \
                printf("%s : %d,",t->attributes[i].items,*(int*)(outs.ptr[i]));     \
                break;                                                              \
            case 'f':                                                               \
                printf("%s : %f,",t->attributes[i].items,*(float*)(outs.ptr[i]));   \
                break;                                                              \
            case 'c':                                                               \
                printf("%s : %c,",t->attributes[i].items,*(char*)(outs.ptr[i]));    \
                break;                                                              \
            case 's':                                                               \
                printf("%s : %s,",t->attributes[i].items,(char*)(outs.ptr[i]));     \
                break;                                                              \
            default:                                                                \
                UNIMPLIMENTED_TYPE(fmt);                                            \
                exit(1);                                                            \
                break;                                                              \
        }                                                                           \
    }                                                                               \
    printf("\n");                                                                   \
}while(0)

// these functions shall manage there own memory and "outs" as they 
// should be already inited

// this expects you populate a row ds during the interpritation of query 
// with data then it writes it to a table
void write_row_dyn(Table*t,Row*r);
// this fills the outs ds and lets you do whatever you want with it
void read_row_dyn(Table*t,unsigned int row_num,outs_package*outs);

#endif
#ifndef TABLE_IMPLI 
#define TABLE_IMPLI

// expects the table to be inited already
void print_varadic_expression_ref(Table*t,va_list args){
    for(size_t i = 0 ; i < t->num_attri ; i++){
        switch(t->expression[i].items[1]){
            case 'd':
                printf("%s : %d,",t->attributes[i].items,*va_arg(args,int*));
                break;
            case 'f':
                printf("%s : %f,",t->attributes[i].items,*va_arg(args,float*));
                break;
            case 'c':
                printf("%s : %c,",t->attributes[i].items,*va_arg(args,char*));
                break;
            case 's':
                printf("%s : %s,",t->attributes[i].items,va_arg(args,char*));
                break;
            default:
                break;
        }
    }
    printf("\n");
}


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

// WARNING:if passes a not initilized table , it copies a possible wrong value
// therefore be warry and enforce that tables are created before any rows are.
// think of it as create some row from some table
Row* init_row(Table*t){
    size_t current_size = t->SIZE_ROW;
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
    obj->SIZE_ROW = 0;
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
    
        // We assert that the types are valid
        if(next_type[0] != '%' && next_type[2] != '\0') return_defer(false);

        switch (next_type[1]) {
            case 'd':
                obj->SIZE_ROW += sizeof(int);
                break;
            case 'f':
                obj->SIZE_ROW += sizeof(float);
                break;
            case 'c':
                obj->SIZE_ROW += sizeof(char);
                break;
            case 's':
                obj->SIZE_ROW += MAX_SIZE_STR;
                break;
            default:
                UNIMPLIMENTED_TYPE(next_type[1]);
                return_defer(false);
                break;
        }

        obj->expression[i].items = malloc(strlen(next_type)+1);
        if(obj->expression[i].items == NULL) return_defer(false);
        strcpy(obj->expression[i].items,next_type);
        obj->attributes[i].items = malloc(strlen(next_name)+1);
        if(obj->attributes[i].items == NULL) return_defer(false);
        strcpy(obj->attributes[i].items,next_name);
    }
    va_end(args);
    obj->ROWS_PER_PAGE  = PAGE_SIZE / obj->SIZE_ROW; 
    obj->TABLE_MAX_ROWS = obj->ROWS_PER_PAGE * TABLE_MAX_PAGES;
    obj->count_rows = 0;            // this missed init cost me 1h
    // init the pages for now in memory 
    // the pager takes reponsibility
    // we read from one file for now
    obj->pager = init_pager("db/main.db");
    obj->count_rows = obj->pager->file_length / obj->SIZE_ROW;
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
    // backup and store the table in a file
    Pager* pager = t->pager;
    size_t tot_pages = t->count_rows / t->ROWS_PER_PAGE;
    for(size_t i = 0 ; i < tot_pages ; i++){
        if(pager->pages[i] == NULL) continue;
        pager_flush(pager,i,PAGE_SIZE);
        free(pager->pages[i]);
        pager->pages[i] = NULL;
    }

    // potential partial files , later we wont need it when we have the b-tree

    size_t num_additional_rows = t->count_rows % t->ROWS_PER_PAGE;
    if(num_additional_rows > 0){
        unsigned int page_num = tot_pages;
        if (pager->pages[page_num] != NULL) {
          pager_flush(pager, page_num, num_additional_rows * t->SIZE_ROW);
          free(pager->pages[page_num]);
          pager->pages[page_num] = NULL;
        } 
    }
    // we close the file
    kill_pager(pager); 

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


void serilize_row_v(Row*dst,String*expression,const unsigned int count,va_list args){
    size_t original_size = dst->size_bytes;
    bool not_matching_size = false;
    bool wrong_format_expression = false;
    bool exceeded_max_str_size = false;

    dst->size_bytes = 0;
    for(unsigned int i = 0; i < count; i++) {
        if (expression[i].items[0] != '%') {
            wrong_format_expression = true;
            goto fail;
        }
        switch (expression[i].items[1]) {
        case 'd': {
            int val = va_arg(args, int);
            memcpy(dst->data + dst->size_bytes, &val, sizeof(val));
            dst->size_bytes += sizeof(val);
            break;
        }
        case 'f': {
            float val = (float)va_arg(args, double);
            memcpy(dst->data + dst->size_bytes, &val, sizeof(val));
            dst->size_bytes += sizeof(val);
            break;
        }
        case 'c': {
            char val = (char)va_arg(args, int);
            memcpy(dst->data + dst->size_bytes, &val, sizeof(val));
            dst->size_bytes += sizeof(val);
            break;
        }
        case 's': {
            char *val = va_arg(args, char*);
            if (!val) { exceeded_max_str_size = true; goto fail; }
            size_t len = strlen(val);
            if (len + 1 > MAX_SIZE_STR) { exceeded_max_str_size = true; goto fail; }
            memcpy(dst->data + dst->size_bytes, val, len + 1);
            memset(dst->data + dst->size_bytes + len + 1,
                   0,
                   MAX_SIZE_STR - (len + 1));
            dst->size_bytes += MAX_SIZE_STR;
            break;
        }
        default:
            fprintf(stderr, "Unknown format “%c”\n", expression[i].items[1]);
            exit(1);
        }
    }

    if (dst->size_bytes != original_size) {
        not_matching_size = true;
        goto fail;
    }

    return;

fail:
    if (not_matching_size)
        fprintf(stderr, "Size mismatch in serialize_row\n");
    if (wrong_format_expression)
        fprintf(stderr, "Bad format in serialize_row\n");
    if (exceeded_max_str_size)
        fprintf(stderr, "String too big in serialize_row\n");
    exit(1);
}

// this expects the number of elemnts in expression to be the same as count 
// if not ? it will exit with code 1
// WARNING: this overrides the values we had before , so be warry
void serilize_row(Row*dst,String*expression,const unsigned int count,...){
    va_list args;
    va_start(args, count);
    serilize_row_v(dst, expression, count, args);
    va_end(args);
}


void deserilize_row_v(const Row*src,String*expression,const unsigned int count,va_list args){
    bool wrong_format_expression = false;
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
                UNIMPLIMENTED_TYPE(expression[i].items[1]);
                exit(1);
                break;
        }
    }
    ASSERT(indexer == src->size_bytes,"mismatch in size_bytes while  desiralization");
    return;
fail:
    if(wrong_format_expression) fprintf(stderr,"wrong format expression.\n");
    exit(1);
}

// we pass a Row as src and pass somepointers to extract the data to those pointers 
// WARNING: be sure to pass a correct arguments.
// This function shall not alter the state of the Row*src
void deserilize_row(const Row*src,String*expression,const unsigned int count,...){
    va_list args;
    va_start(args,count);
    deserilize_row_v(src,expression,count,args);
    va_end(args);
}


void* row_select(Table* t,unsigned int row_num){
    unsigned int page_num = row_num / t->ROWS_PER_PAGE;
    void* page = get_page(t->pager,page_num); 
    unsigned int row_offset = row_num % t->ROWS_PER_PAGE;
    unsigned int bytes_offset = row_offset * t->SIZE_ROW;
    return (char*)page + bytes_offset;
}


// TODO: change the make another version if the 
// row_select that does not allocate and only check is a row exits in a page 
// and retuns a NULL otherwise

void write_row(Table*t,unsigned int count,...){
    if(t->count_rows >= t->TABLE_MAX_ROWS) return;
    va_list args;
    va_start(args,count);
    Row*r = init_row(t);

    // init the row with the data
    serilize_row_v(r,t->expression,count,args);

    va_end(args);
    
    // copy the data to the right location in the table pages
    memcpy(row_select(t,t->count_rows),r->data,r->size_bytes);

    // inc the count_rows
    ++t->count_rows;
    
    // clean after yourself
    kill_row(r);
}


void read_row(Table*t,unsigned int row_num,unsigned int count,...){
    // the varadic args are pointers to data members.
    
    ASSERT(count == t->num_attri,"COUNT != num_attri while reading row");


    va_list args;
    va_start(args,count);
    
    // create a row for data to be copied to 
    Row*r = init_row(t);
    
    ASSERT(r->size_bytes == t->SIZE_ROW,"size_bytes != SIZE_ROWS while reading row");


    void *src = row_select(t, row_num);

    assert(src != NULL);
    assert(r->data != NULL);


    // copy data from spot to the row
    memcpy(r->data,src,r->size_bytes);

    va_list args_copy;
    va_copy(args_copy, args);


    // do the actual work
    deserilize_row_v(r,            // or &r if your signature wants a Row*
                      t->expression,
                      count,              // use the same count you passed
                      args);


    print_varadic_expression_ref(t, args_copy);

    // tidy up
    va_end(args_copy);
    va_end(args);
    kill_row(r);
}


// we assumse table is inited and right
void read_row_dyn(Table*t,unsigned int row_num,outs_package*outs){
    Row*r = init_row(t);
    
    ASSERT(r->size_bytes == t->SIZE_ROW,"size_bytes != SIZE_ROWS while reading row dynamiclly");

    void*src = row_select(t,row_num);
    
    ASSERT(src != NULL,"src is NULL while reading dynamiclly");
    ASSERT(r->data != NULL,"r->data is NULL while reading dynamiclly");
   
    memcpy(r->data,src,r->size_bytes);
    
    // populating the outs datastructe 
    size_t offset = 0;
    for(size_t i = 0 ; i < t->num_attri ; i++){
        POPULATE_OUTS(t,outs,r,i,offset);
    }

    kill_row(r);
}


void write_row_dyn(Table*t,Row*r){
    if(t->count_rows >= t->TABLE_MAX_ROWS) return;

    ASSERT(r->size_bytes == t->SIZE_ROW,"size_bytes != SIZE_ROWS while reading row dynamiclly");
    
    memcpy(row_select(t,t->count_rows),r->data,r->size_bytes);

    // inc the count_rows
    ++t->count_rows;
}

#endif
