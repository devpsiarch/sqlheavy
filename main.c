#include <criterion/criterion.h>
#include <stdio.h>

#include "include/interface.h"
#define INTERFACE_IMPLI

#include "include/token.h"

#include "include/table.h"

Test(parsing, basic_select) {
    char* test = "select * from users;";
    arr_tokens* result = parser(test);
    
    cr_expect_eq(result->count, 5, "Expected 5 tokens");
    cr_expect_eq(result->items[0].type, SELECT);
    cr_expect_str_eq(result->items[0].lexeme, "select");
    cr_expect_eq(result->items[1].type, ALL);
    cr_expect_str_eq(result->items[1].lexeme, "*");
    cr_expect_eq(result->items[2].type, FROM);
    cr_expect_str_eq(result->items[2].lexeme, "from");
    cr_expect_eq(result->items[3].type, ID);
    cr_expect_str_eq(result->items[3].lexeme, "users");
    cr_expect_eq(result->items[4].type, SEMI);
    cr_expect_str_eq(result->items[4].lexeme, ";");
    
    // Cleanup
    toke_arr_free(result);
    free(result);
}

Test(parsing, insert_command) {
    char* test = "insert 1 from table;";
    arr_tokens* result = parser(test);
    
    cr_expect_eq(result->count, 5);
    cr_expect_eq(result->items[0].type, INSERT);
    cr_expect_eq(result->items[1].type, ID);
    cr_expect_str_eq(result->items[1].lexeme, "1");
    cr_expect_eq(result->items[2].type, FROM);
    cr_expect_eq(result->items[3].type, TABLE);
    cr_expect_eq(result->items[4].type, SEMI);
    
    // Cleanup
    toke_arr_free(result);
    free(result); 
}

Test(parsing, with_parentheses) {
    char* test = "select (id, name) from users;";
    arr_tokens* result = parser(test);
    
    cr_expect_eq(result->count, 9);
    cr_expect_eq(result->items[0].type, SELECT);
    cr_expect_eq(result->items[1].type, OPEN_PAR);
    cr_expect_eq(result->items[2].type, ID);
    cr_expect_eq(result->items[3].type, COMMA);
    cr_expect_eq(result->items[4].type, ID);
    cr_expect_eq(result->items[5].type, CLOSE_PAR);
    cr_expect_eq(result->items[6].type, FROM);
    cr_expect_eq(result->items[7].type, ID);
    cr_expect_eq(result->items[8].type, SEMI);

    
    // Cleanup
    toke_arr_free(result);
    free(result);
}

Test(parsing, unknown_tokens) {
    char* test = "select @ from #users$;";
    arr_tokens* result = parser(test);
    
    cr_expect_eq(result->items[0].type, SELECT);
    cr_expect_eq(result->items[1].type, UNKNOWN);
    cr_expect_str_eq(result->items[1].lexeme, "@");
    cr_expect_eq(result->items[2].type, FROM);
    cr_expect_eq(result->items[3].type, UNKNOWN);
    cr_expect_str_eq(result->items[3].lexeme, "#");
    cr_expect_eq(result->items[4].type, ID);
    cr_expect_str_eq(result->items[4].lexeme, "users");
    cr_expect_eq(result->items[5].type, UNKNOWN);
    cr_expect_str_eq(result->items[5].lexeme, "$");
    cr_expect_eq(result->items[6].type, SEMI);
    
    // Cleanup
    toke_arr_free(result);
    free(result);
}

Test(parsing, whitespace_handling) {
    char* test = "  select\t*\nfrom  users  ;  ";
    arr_tokens* result = parser(test);
    
    cr_expect_eq(result->count, 5);
    cr_expect_eq(result->items[0].type, SELECT);
    cr_expect_eq(result->items[1].type, ALL);
    cr_expect_eq(result->items[2].type, FROM);
    cr_expect_eq(result->items[3].type, ID);
    cr_expect_str_eq(result->items[3].lexeme, "users");
    cr_expect_eq(result->items[4].type, SEMI);
    
    
    // Cleanup
    toke_arr_free(result);
    free(result);
}

Test(parsing, empty_string) {
    char* test = "";
    arr_tokens* result = parser(test);
    
    cr_expect_eq(result->count, 0);
    cr_expect_eq(result->items, NULL);
    
    // No cleanup needed for empty result
    toke_arr_free(result);
    free(result);
}

Test(parsing, only_whitespace) {
    char* test = " \t\n ";
    arr_tokens* result = parser(test);
    
    cr_expect_eq(result->count, 0);
    cr_expect_eq(result->items, NULL);
    
    // No cleanup needed for empty result
    toke_arr_free(result);
    free(result);
}

Test(getting_expression,init){
    Table* ans = init_table(2,"%d","ID","%s","Name");
    cr_expect_str_eq(ans->expression[0].items, "%d");
    cr_expect_str_eq(ans->attributes[0].items, "ID");
    cr_expect_str_eq(ans->expression[1].items, "%s");
    cr_expect_str_eq(ans->attributes[1].items, "Name");
    cr_expect_eq(ans->num_attri, 2);
    kill_table(ans);
}
Test(getting_expression, multiple_pairs){
    Table* ans = init_table(3,
                            "%d", "ID",
                            "%s", "Name",
                            "%f", "Balance");
    cr_assert_not_null(ans);
    cr_expect_str_eq(ans->expression[0].items, "%d");
    cr_expect_str_eq(ans->attributes[0].items, "ID");
    cr_expect_str_eq(ans->expression[1].items, "%s");
    cr_expect_str_eq(ans->attributes[1].items, "Name");
    cr_expect_str_eq(ans->expression[2].items, "%f");
    cr_expect_str_eq(ans->attributes[2].items, "Balance");
    cr_expect_eq(ans->num_attri, 3);
    

    kill_table(ans);
}

Test(calculating_size_of_row, simple){
    Table* ans = init_table(3,
                            "%d", "ID",
                            "%s", "Name",
                            "%f", "Balance");
    cr_expect_eq(ans->num_attri, 3);
    cr_expect_eq(calculate_size_row(ans->expression,ans->num_attri),sizeof(int)+MAX_SIZE_STR+sizeof(float));
    Row*test = init_row(ans);
    cr_expect_eq(test->size_bytes,sizeof(int)+MAX_SIZE_STR+sizeof(float));
    kill_row(test);
    kill_table(ans);
}

Test(serilize,init){
    Table*  tbl = init_table(3,
                             "%d", "ID",
                             "%s", "Name",
                             "%f", "Balance");
    cr_assert_not_null(tbl);
    cr_expect_eq(tbl->num_attri, 3);

    /* 2) Allocate a Row buffer for that schema */
    Row*    row = init_row(tbl);
    cr_assert_not_null(row);

    /* 3) Serialize one tuple: (42, "Alice", 99.5) */
    serilize_row(row,
                 tbl->expression,
                 tbl->num_attri,
                 42,               /* int */
                 "Alice", (size_t)8, /* string + max size */
                 99.5);            /* float */

    /* 4) Peel it back out and verify byte‑for‑byte */
    size_t off = 0;

    /* a) check the int */
    int   got_id;
    memcpy(&got_id, row->data + off, sizeof got_id);
    cr_expect_eq(got_id, 42, "ID field");
    off += sizeof(got_id);

    /* b) check the string (should be zero‑padded to length 8) */
    char  got_name[9] = {0};
    memcpy(got_name, row->data + off, 8);
    cr_expect_str_eq(got_name,"Alice");
    //printf("string gotten : %s\n",got_name);
    off += MAX_SIZE_STR;

    /* c) check the float */
    float got_bal;
    memcpy(&got_bal, row->data + off, sizeof got_bal);
    cr_expect_eq(got_bal, 99.5f);
    //printf("gotten balance : %f\n",got_bal);
    off += sizeof(got_bal);

    /* d) total size_bytes should match */
    cr_expect_eq(row->size_bytes, off);
    //printf("gotten size_bytes : %zu/%zu\n",off,row->size_bytes);

    /* 5) Clean up */
    kill_row(row);
    kill_table(tbl);
}

Test(serilize, custom_schema) {
    /* 1) Build a 4‑column table:
            char  Grade;
            int   Count;
            string Label (max LABEL_MAX);
            float Ratio; */
    Table* tbl = init_table(4,
                            "%c", "Grade",
                            "%d", "Count",
                            "%s", "Label",
                            "%f", "Ratio");
    cr_assert_not_null(tbl);
    cr_expect_eq(tbl->num_attri, 4);

    /* 2) Allocate a Row buffer for that schema */
    Row* row = init_row(tbl);
    cr_assert_not_null(row);

    /* 3) Serialize one tuple: ('B', 256, "Payload", 3.1415f) */
    serilize_row(row,
                 tbl->expression,
                 tbl->num_attri,
                 'B',                    /* char → promoted to int */
                 256,                    /* int */
                 "Payload",  /* string + max size */
                 3.1415);                /* float → promoted to double */

    /* 4) Peel it back out and verify byte‑for‑byte */
    size_t off = 0;

    /* a) check the char */
    char got_grade;
    memcpy(&got_grade, row->data + off, sizeof got_grade);
    cr_expect_eq(got_grade, 'B', "Grade field");
    off += sizeof got_grade;

    /* b) check the int */
    int got_count;
    memcpy(&got_count, row->data + off, sizeof got_count);
    cr_expect_eq(got_count, 256, "Count field");
    off += sizeof got_count;

    /* c) check the string (zero‑padded to LABEL_MAX) */
    char got_label[255] = {0};
    memcpy(got_label, row->data + off, 255);
    cr_expect_str_eq(got_label, "Payload", "Label field");
    off += MAX_SIZE_STR;

    /* d) check the float */
    float got_ratio;
    memcpy(&got_ratio, row->data + off, sizeof got_ratio);
    cr_expect_float_eq(got_ratio, 3.1415f, 1e-6, "Ratio field");
    off += sizeof got_ratio;

    /* e) total size_bytes should match */
    cr_expect_eq(row->size_bytes, off, "total serialized size");

    /* 5) Clean up */
    kill_row(row);
    kill_table(tbl);
    printf("Passed the second serilize function test\n");
}

Test(dessirilize,init){
    Table* tbl = init_table(4,
                            "%c", "Grade",
                            "%d", "Count",
                            "%s", "Label",
                            "%f", "Ratio");
    cr_assert_not_null(tbl);
    cr_expect_eq(tbl->num_attri, 4);

    /* 2) Allocate a Row buffer for that schema */
    Row* row = init_row(tbl);
    cr_assert_not_null(row);

    /* 3) Serialize one tuple: ('B', 256, "Payload", 3.1415f) */
    serilize_row(row,
                 tbl->expression,
                 tbl->num_attri,
                 'B',                    /* char → promoted to int */
                 256,                    /* int */
                 "Payload",  /* string + max size */
                 3.1415);                /* float → promoted to double */

    cr_expect_eq(tbl->SIZE_ROW,row->size_bytes);

    char got_c;
    int got_d;
    char *got_s = malloc(MAX_SIZE_STR);
    float got_f;

    deserilize_row(row,tbl->expression,tbl->num_attri,&got_c,&got_d,got_s,&got_f);

    cr_expect_eq(got_c,'B');
    cr_expect_eq(got_d,256);
    cr_expect_str_eq(got_s, "Payload");
    cr_expect_float_eq(got_f, 3.1415, 1e-6, "Ratio field");
    
    //printf("got_c (Grade): %c\n", got_c);
    //printf("got_d (Count): %d\n", got_d);
    //printf("got_s (Label): %s\n", got_s);
    //printf("got_f (Ratio): %f\n", got_f);

    free(got_s);
    kill_row(row);
    kill_table(tbl);
}


Test(write_read_table, init) {
    /* 1) build the table */
    Table* tbl = init_table(4,
                            "%c", "Grade",
                            "%d", "Count",
                            "%s", "Label",
                            "%f", "Ratio");
    cr_assert_not_null(tbl);
    cr_expect_eq(tbl->num_attri, 4);

    /* 2) write one row */
    write_row(tbl, tbl->num_attri,
              'B',            /* %c */
              15,             /* %d */
              "Knight",       /* %s */
              0.9f);          /* %f */

    /* 3) grab raw pointer to the first row’s payload */
    void *row_ptr = row_select(tbl, 0);
    cr_assert_not_null(row_ptr);

    /* 4) decode each field in turn */
    size_t offset = 0;

    /* char Grade */
    char grade = *(char *)((char *)row_ptr + offset);
    cr_expect_eq(grade, 'B');
    offset += sizeof(char);

    /* int Count */
    int count = *(int *)((char *)row_ptr + offset);
    cr_expect_eq(count, 15);
    offset += sizeof(int);

    /* string Label */
    char label_buf[MAX_SIZE_STR];
    memcpy(label_buf, (char *)row_ptr + offset, MAX_SIZE_STR);
    cr_expect_str_eq(label_buf, "Knight");
    offset += MAX_SIZE_STR;

    /* float Ratio */
    float ratio;
    memcpy(&ratio, (char *)row_ptr + offset, sizeof(float));
    cr_expect_float_eq(ratio, 0.9f, 1e-6);

    /* 5) cleanup */
    kill_table(tbl);
}


Test(read_from_table,init){
Table* tbl = init_table(4,
                            "%c", "Grade",                                                                                              "%d", "Count",
                            "%s", "Label",
                            "%f", "Ratio");
    cr_assert_not_null(tbl);
    cr_expect_eq(tbl->num_attri, 4);
    char got_c;
    int got_d;
    char *got_s = malloc(MAX_SIZE_STR);
    float got_f;


    write_row(tbl,tbl->num_attri,'B',256,"Payload",3.1415);


    read_row(tbl,0,tbl->num_attri,&got_c,&got_d,got_s,&got_f);

    cr_expect_eq(got_c,'B');
    cr_expect_eq(got_d,256);
    cr_expect_str_eq(got_s, "Payload");
    cr_expect_float_eq(got_f, 3.1415, 1e-6, "Ratio field");

    printf("got_c (Grade): %c\n", got_c);
    printf("got_d (Count): %d\n", got_d);
    printf("got_s (Label): %s\n", got_s);
    printf("got_f (Ratio): %f\n", got_f);

    kill_table(tbl);
    free(got_s);
}

#if 1

int main(void){
    db_interactive();
    printf("session is over\n");
    return 0;
}

#endif
