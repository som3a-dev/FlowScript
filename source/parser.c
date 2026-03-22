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

    token_t* operator;
    expr_t* right;
} expr_unary_t;

typedef struct
{
    expr_t e;

    const char* val;
} expr_literal_t;

static char* expr_to_str(const expr_t* expr);

void parse(token_list_t* tokens)
{
    expr_literal_t l;
    l.e.type = EXPR_LITERAL;
    l.val = "12";

    token_t op;
    op.type = TOKEN_MINUS;
    op.lexeme = "-";
    op.line = 1;

    expr_unary_t u;
    u.e.type = EXPR_UNARY;
    u.operator = &op;
    u.right = (expr_t*)&l;

    char* str = expr_to_str((const expr_t*)&u);
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

        case EXPR_UNARY:
        {
            const expr_unary_t* e = (expr_unary_t*)expr;
            char* right = expr_to_str(e->right);

            size_t str_size = sizeof(char) + (strlen(right) + strlen(e->operator->lexeme) + 1);
            char* str = malloc(str_size);
            snprintf(str, str_size, "%s%s", e->operator->lexeme, right);

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