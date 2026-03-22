#include "parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum
{
    EXPR_BINARY,
    EXPR_GROUPING,
    EXPR_LITERAL,
    EXPR_UNARY
} expr_type_t;

typedef struct
{
    expr_type_t type;
} expr_t;

typedef struct
{
    expr_t e;

    expr_t* left;
    token_t* operator;
    expr_t* right;
} expr_binary_t;

typedef struct
{
    expr_t e;

    const char* val;
} expr_literal_t;

static char* expr_to_str(const expr_t* expr);

void parse(token_list_t* tokens)
{
    expr_literal_t n1;
    n1.e.type = EXPR_LITERAL;
    n1.val = "5";

    expr_literal_t n2;
    n2.e.type = EXPR_LITERAL;
    n2.val = "3";

    expr_binary_t op;
    op.e.type = EXPR_BINARY;
    op.left = (expr_t*)&n1;
    op.right = (expr_t*)&n2;

    token_t token;
    token.type = TOKEN_PLUS;
    token.line = 1;
    token.lexeme = "+";
    op.operator = &token;

    expr_binary_t op2;
    op2.e.type = EXPR_BINARY;
    op2.left = (expr_t*)&n1;
    op2.right = (expr_t*)&op;

    token_t token2;
    token2.type = TOKEN_MINUS;
    token2.line = 1;
    token2.lexeme = "-";
    op2.operator = &token2;

    char* str = expr_to_str((const expr_t*)&op2);
    printf("Expr: %s\n", str);

    free(str);

    (tokens);
}

static char* expr_to_str(const expr_t* expr)
{
    switch (expr->type)
    {
        case EXPR_BINARY:
        {
            const expr_binary_t* e = (expr_binary_t*)expr;
            char* left = expr_to_str(e->left);
            char* right = expr_to_str(e->right);

            size_t str_size = sizeof(char) * (strlen(left) + strlen(right) + strlen(e->operator->lexeme) + 1 + 2); // accounting for null terminator and spaces
            char* str = malloc(str_size);
            snprintf(str, str_size, "%s %s %s", e->operator->lexeme, left, right);

            free(left);
            free(right);

            return str;
        } break;

        case EXPR_LITERAL:
        {
            const expr_literal_t* e = (expr_literal_t*)expr;
            char* str = malloc(sizeof(char) * (strlen(e->val) + 1));
            strcpy(str, e->val);

            return str;
        } break;
    }

    return NULL;
}