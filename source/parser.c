#include "parser.h"

#include <assert.h>

#include <stdarg.h>
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
    token_t operator;
    expr_t* right;
} expr_binary_t;

typedef struct
{
    expr_t e;

    token_t operator;
    expr_t* right;
} expr_unary_t;

typedef enum
{
    LITERAL_BOOL,
    LITERAL_STRING,
    LITERAL_NUMBER,
    LITERAL_NULL
} expr_literal_type_t;

typedef struct
{
    expr_t e;

    expr_literal_type_t type;
    union {
        float num;
        const char* str;
        bool boolean;
    } val;
} expr_literal_t;

typedef struct
{
    expr_t e;

    expr_t* inner;
} expr_grouping_t;

static expr_t* parse_expr();
static void free_expr(expr_t* expr);

static expr_t* parse_equality();
static expr_t* parse_comparison();
static expr_t* parse_term();
static expr_t* parse_factor();
static expr_t* parse_unary();
static expr_t* parse_primary();

static char* expr_to_str(const expr_t* expr);

static bool str_equal(const char* str, int n, ...)
{
    if (!str) {
        return false;
    }

    va_list args;
    va_start(args, n);

    bool equal = false;

    for (int i = 0; i < n; i++)
    {
        const char* str2 = va_arg(args, const char*);
        assert(str2);

        if (strcmp(str, str2) == 0) {
            equal = true;
            break;
        }
    }

    va_end(args);
    return equal;
}

static int curr = 0;
static token_list_t* tokens = NULL;

static inline token_t get_token()
{
    token_t tok = {0};
    tok.type = TOKEN_NONE;

    if (curr < tokens->len) {
        tok = tokens->tokens[curr];
    }

    return tok;
}

void parse(token_list_t* _tokens)
{
    curr = 0;
    tokens = _tokens;

    expr_t* expr = parse_expr();
    if (expr == NULL)
    {
        return;
    }

    char* str = expr_to_str(expr);
    printf("Expr Str: %s\n", str);
    free(str);

    free_expr(expr);
}

static expr_t* parse_expr()
{
    return parse_equality();
}

static void free_expr(expr_t* expr)
{
    switch (expr->type)
    {
        case EXPR_BINARY:
        {
            expr_binary_t* e = (expr_binary_t*)expr;

            free_expr(e->left);
            free_expr(e->right);
        } break;

        case EXPR_GROUPING:
        {
            expr_grouping_t* e = (expr_grouping_t*)expr;

            free_expr(e->inner);
        } break;

        case EXPR_LITERAL:
        {
            expr_literal_t* e = (expr_literal_t*)expr;
            (void)e;
        } break;

        case EXPR_UNARY:
        {
            expr_unary_t* e = (expr_unary_t*)expr;

            free_expr(e->right);
        } break;
    }

    free(expr);
}

static inline expr_binary_t* new_binary_expr()
{
    expr_binary_t* expr = calloc(1, sizeof(expr_binary_t));
    expr->e.type = EXPR_BINARY;

    return expr;
}

static inline expr_unary_t* new_unary_expr()
{
    expr_unary_t* expr = calloc(1, sizeof(expr_unary_t));
    expr->e.type = EXPR_UNARY;

    return expr;
}

static inline expr_literal_t* new_literal_expr()
{
    expr_literal_t* expr = calloc(1, sizeof(expr_literal_t));
    expr->e.type = EXPR_LITERAL;

    return expr;
}

static inline expr_grouping_t* new_grouping_expr()
{
    expr_grouping_t* expr = calloc(1, sizeof(expr_grouping_t));
    expr->e.type = EXPR_GROUPING;

    return expr;
}

static expr_t* parse_equality()
{
    expr_t* expr = parse_comparison();
    if (expr == NULL)
    {
        return NULL;
    }

    token_t tok = get_token();
    while (str_equal(tok.lexeme, 2, "==", "!="))
    {
        curr++;

        expr_t* right = parse_comparison();
        
        expr_binary_t* new_expr = new_binary_expr();
        new_expr->left = expr;
        new_expr->right = right;
        new_expr->operator = tok;

        expr = (expr_t*)new_expr;

        tok = get_token();
    }

    return (expr_t*)expr;
}

static expr_t* parse_comparison()
{
    expr_t* expr = parse_term();
    if (expr == NULL)
    {
        return NULL;
    }

    token_t tok = get_token();
    while (str_equal(tok.lexeme, 4, ">", ">=", "<=", "<"))
    {
        curr++;

        expr_t* right = parse_term();
        
        expr_binary_t* new_expr = new_binary_expr();
        new_expr->left = expr;
        new_expr->right = right;
        new_expr->operator = tok;

        expr = (expr_t*)new_expr;
        
        tok = get_token();
    }

    return (expr_t*)expr;
}

static expr_t* parse_term()
{
    expr_t* expr = parse_factor();
    if (expr == NULL)
    {
        return NULL;
    }

    token_t tok = get_token();
    while (str_equal(tok.lexeme, 2, "-", "+"))
    {
        curr++;

        expr_t* right = parse_factor();
        
        expr_binary_t* new_expr = new_binary_expr();
        new_expr->left = expr;
        new_expr->right = right;
        new_expr->operator = tok;

        expr = (expr_t*)new_expr;

        tok = get_token();
    }

    return (expr_t*)expr;
}

static expr_t* parse_factor()
{
    expr_t* expr = parse_unary();
    if (expr == NULL)
    {
        return NULL;
    }

    token_t tok = get_token();
    while (str_equal(tok.lexeme, 2, "/", "*"))
    {
        curr++;

        expr_t* right = parse_unary();
        
        expr_binary_t* new_expr = new_binary_expr();
        new_expr->left = expr;
        new_expr->right = right;
        new_expr->operator = tok;

        expr = (expr_t*)new_expr;
        
        tok = get_token();
    }

    return (expr_t*)expr;
}

static expr_t* parse_unary()
{
    token_t tok = get_token();
    if (str_equal(tok.lexeme, 2, "!", "-")) 
    {
        curr++;
        expr_unary_t* expr = new_unary_expr();
        expr->operator = tok;
        expr->right = parse_unary();

        return (expr_t*)expr;
    }

    return parse_primary();
}

static expr_t* parse_primary()
{
    token_t tok = get_token();
    
    switch (tok.type)
    {
        case TOKEN_FALSE:
        {
            curr++;

            expr_literal_t* expr = new_literal_expr();
            expr->type = LITERAL_BOOL;
            expr->val.boolean = false;

            return (expr_t*)expr;
        } break;

        case TOKEN_TRUE:
        {
            curr++;

            expr_literal_t* expr = new_literal_expr();
            expr->type = LITERAL_BOOL;
            expr->val.boolean = true;

            return (expr_t*)expr;
        } break;

        case TOKEN_NIL:
        {
            curr++;

            expr_literal_t* expr = new_literal_expr();
            expr->type = LITERAL_NULL;

            return (expr_t*)expr;
        } break;

        case TOKEN_STRING:
        {
            curr++;

            expr_literal_t* expr = new_literal_expr();
            expr->type = LITERAL_STRING;
            expr->val.str = tok.lexeme;
           
            return (expr_t*)expr;
        } break;

        case TOKEN_NUMBER:
        {
            curr++;

            expr_literal_t* expr = new_literal_expr();
            expr->type = LITERAL_NUMBER;
            expr->val.num = (float)(atof(tok.lexeme));

            return (expr_t*)expr;
        } break;

        case TOKEN_LEFT_PAREN:
        {
            curr++;

            expr_t* inner = parse_expr();
            token_t current = get_token();

            if (!str_equal(current.lexeme, 1, ")"))
            {
                printf("PARSER ERROR: Expected ')' after expression\n");
                free_expr(inner);
                return NULL;
            }

            expr_grouping_t* expr = new_grouping_expr();
            expr->inner = inner;
            return (expr_t*)expr;
        } break;
    }

    printf("PARSER ERROR: Expected an expression\n");
    return NULL;
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

            size_t str_size = sizeof(char) * (strlen(left) + strlen(right) + strlen(e->operator.lexeme) + 1 + 4); // accounting for null terminator and spaces
            char* str = malloc(str_size);
            snprintf(str, str_size, "(%s %s %s)", e->operator.lexeme, left, right);

            free(left);
            free(right);

            return str;
        } break;

        case EXPR_UNARY:
        {
            const expr_unary_t* e = (expr_unary_t*)expr;
            char* right = expr_to_str(e->right);

            size_t str_size = sizeof(char) + (strlen(right) + strlen(e->operator.lexeme) + 1);
            char* str = malloc(str_size);
            snprintf(str, str_size, "%s%s", e->operator.lexeme, right);

            free(right);
            
            return str;
        } break;

        case EXPR_GROUPING:
        {
            const expr_grouping_t* e = (expr_grouping_t*)expr;
            char* inner = expr_to_str(e->inner);

            size_t str_size = sizeof(char) * (strlen(inner) + strlen("()") + 1);
            char* str = malloc(str_size);
            snprintf(str, str_size, "(%s)", inner);

            free(inner);

            return str;
        } break;

        case EXPR_LITERAL:
        {
            const expr_literal_t* e = (expr_literal_t*)expr;

            char* str = NULL;
            switch (e->type)
            {
                case LITERAL_BOOL:
                {
                    const char* bool_str = "false";
                    if (e->val.boolean) {
                        bool_str = "true" ;
                    }

                    str = malloc(sizeof(char) * (strlen(bool_str) + 1));
                    strcpy(str, bool_str);
                } break;

                case LITERAL_NUMBER:
                {
                    int str_size = snprintf(NULL, 0, "%f", e->val.num) + 1;

                    str = malloc(str_size);

                    snprintf(str, str_size, "%f", e->val.num);
                } break;

                case LITERAL_STRING:
                {
                    str = malloc(strlen(e->val.str) + 1);
                    strcpy(str, e->val.str);
                } break;
            }

            assert(str);
            return str;
        } break;
    }

    return NULL;
}