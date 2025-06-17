#include <criterion/criterion.h>
#include <stdio.h>

#include "include/interface.h"
#define INTERFACE_IMPLI

#include "include/token.h"

#include "include/table.h"

Test(parsing, basic_select) {
    char* test = "select * from users;";
    arr_tokens result = parser(test);
    
    cr_expect_eq(result.count, 5, "Expected 5 tokens");
    cr_expect_eq(result.items[0].type, SELECT);
    cr_expect_str_eq(result.items[0].lexeme, "select");
    cr_expect_eq(result.items[1].type, ALL);
    cr_expect_str_eq(result.items[1].lexeme, "*");
    cr_expect_eq(result.items[2].type, FROM);
    cr_expect_str_eq(result.items[2].lexeme, "from");
    cr_expect_eq(result.items[3].type, ID);
    cr_expect_str_eq(result.items[3].lexeme, "users");
    cr_expect_eq(result.items[4].type, SEMI);
    cr_expect_str_eq(result.items[4].lexeme, ";");
    
    // Cleanup
    for (size_t i = 0; i < result.count; i++) free(result.items[i].lexeme);
    free(result.items);
}

Test(parsing, insert_command) {
    char* test = "insert 1 from table;";
    arr_tokens result = parser(test);
    
    cr_expect_eq(result.count, 5);
    cr_expect_eq(result.items[0].type, INSERT);
    cr_expect_eq(result.items[1].type, UNKNOWN);
    cr_expect_str_eq(result.items[1].lexeme, "1");
    cr_expect_eq(result.items[2].type, FROM);
    cr_expect_eq(result.items[3].type, TABLE);
    cr_expect_eq(result.items[4].type, SEMI);
    
    // Cleanup
    for (size_t i = 0; i < result.count; i++) free(result.items[i].lexeme);
    free(result.items);
}

Test(parsing, with_parentheses) {
    char* test = "select (id, name) from users;";
    arr_tokens result = parser(test);
    
    cr_expect_eq(result.count, 9);
    cr_expect_eq(result.items[0].type, SELECT);
    cr_expect_eq(result.items[1].type, OPEN_PAR);
    cr_expect_eq(result.items[2].type, ID);
    cr_expect_eq(result.items[3].type, COMMA);
    cr_expect_eq(result.items[4].type, ID);
    cr_expect_eq(result.items[5].type, CLOSE_PAR);
    cr_expect_eq(result.items[6].type, FROM);
    cr_expect_eq(result.items[7].type, ID);
    cr_expect_eq(result.items[8].type, SEMI);

    
    // Cleanup
    for (size_t i = 0; i < result.count; i++) free(result.items[i].lexeme);
    free(result.items);
}

Test(parsing, unknown_tokens) {
    char* test = "select @ from #users$;";
    arr_tokens result = parser(test);
    
    cr_expect_eq(result.items[0].type, SELECT);
    cr_expect_eq(result.items[1].type, UNKNOWN);
    cr_expect_str_eq(result.items[1].lexeme, "@");
    cr_expect_eq(result.items[2].type, FROM);
    cr_expect_eq(result.items[3].type, UNKNOWN);
    cr_expect_str_eq(result.items[3].lexeme, "#");
    cr_expect_eq(result.items[4].type, ID);
    cr_expect_str_eq(result.items[4].lexeme, "users");
    cr_expect_eq(result.items[5].type, UNKNOWN);
    cr_expect_str_eq(result.items[5].lexeme, "$");
    cr_expect_eq(result.items[6].type, SEMI);
    
    // Cleanup
    for (size_t i = 0; i < result.count; i++) free(result.items[i].lexeme);
    free(result.items);
}

Test(parsing, whitespace_handling) {
    char* test = "  select\t*\nfrom  users  ;  ";
    arr_tokens result = parser(test);
    
    cr_expect_eq(result.count, 5);
    cr_expect_eq(result.items[0].type, SELECT);
    cr_expect_eq(result.items[1].type, ALL);
    cr_expect_eq(result.items[2].type, FROM);
    cr_expect_eq(result.items[3].type, ID);
    cr_expect_str_eq(result.items[3].lexeme, "users");
    cr_expect_eq(result.items[4].type, SEMI);
    
    
    // Cleanup
    for (size_t i = 0; i < result.count; i++) free(result.items[i].lexeme);
    free(result.items);
}

Test(parsing, empty_string) {
    char* test = "";
    arr_tokens result = parser(test);
    
    cr_expect_eq(result.count, 0);
    cr_expect_eq(result.items, NULL);
    
    // No cleanup needed for empty result
}

Test(parsing, only_whitespace) {
    char* test = " \t\n ";
    arr_tokens result = parser(test);
    
    cr_expect_eq(result.count, 0);
    cr_expect_eq(result.items, NULL);
    
    // No cleanup needed for empty result
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

#if 1

int main(void){
    db_interactive();
    printf("session is over\n");
    return 0;
}

#endif
