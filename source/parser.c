//TODOS():

//TODO(omar): add token info to errors

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
    OBJECT_BOOL,
    OBJECT_STRING,
    OBJECT_NUMBER,
    OBJECT_NIL
} object_type_t;

typedef struct
{
    expr_t e;

    // technically an object and a literal are not the same thing
    // and probably shouldn't share a type enum
    // but we don't care for now
    object_type_t type;
    union {
        float num;
        char* str;
        bool boolean;
    } val;
} expr_literal_t;

typedef struct
{
    expr_t e;

    expr_t* inner;
} expr_grouping_t;

typedef struct
{
    object_type_t type;
    union {
        float num;
        char* str;
        bool boolean;
    } val;
} object_t;


static expr_t* parse_expr();
static void free_expr(expr_t* expr);

static expr_t* parse_series();
static expr_t* parse_equality();
static expr_t* parse_comparison();
static expr_t* parse_term();
static expr_t* parse_factor();
static expr_t* parse_unary();
static expr_t* parse_primary();

object_t interpret_expr(expr_t* expr, const char** out_err);

static void free_object(object_t* obj);
static void print_object(const object_t* obj);

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

    const char* err = NULL;
    object_t obj = interpret_expr(expr, &err);
    if (err) {
        printf("RUNTIME ERROR: %s\n", err);
    }
    else {
        print_object(&obj);
    }

    free_object(&obj);
    free_expr(expr);
}

static expr_t* parse_expr()
{
    return parse_series();
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

static expr_t* parse_series()
{
    expr_t* expr = parse_equality();
    if (expr == NULL)
    {
        return NULL;
    }

    token_t tok = get_token();
    while (str_equal(tok.lexeme, 1, ","))
    {
        curr++;

        expr_t* right = parse_equality();

        expr_binary_t* new_expr = new_binary_expr();

        new_expr->left = expr;
        new_expr->right = right;
        new_expr->operator = tok;

        expr = (expr_t*)new_expr;

        tok = get_token();
    }

    return (expr_t*)expr;
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
            expr->type = OBJECT_BOOL;
            expr->val.boolean = false;

            return (expr_t*)expr;
        } break;

        case TOKEN_TRUE:
        {
            curr++;

            expr_literal_t* expr = new_literal_expr();
            expr->type = OBJECT_BOOL;
            expr->val.boolean = true;

            return (expr_t*)expr;
        } break;

        case TOKEN_NIL:
        {
            curr++;

            expr_literal_t* expr = new_literal_expr();
            expr->type = OBJECT_NIL;

            return (expr_t*)expr;
        } break;

        case TOKEN_STRING:
        {
            curr++;

            expr_literal_t* expr = new_literal_expr();
            expr->type = OBJECT_STRING;
            expr->val.str = tok.lexeme;
           
            return (expr_t*)expr;
        } break;

        case TOKEN_NUMBER:
        {
            curr++;

            expr_literal_t* expr = new_literal_expr();
            expr->type = OBJECT_NUMBER;
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

static bool object_is_truthy(const object_t* obj)
{
    switch (obj->type)
    {
        case OBJECT_BOOL:
        {
            return (obj->val.boolean);
        } break;

        case OBJECT_NIL:
        {
            return false;
        } break;

        default:
        {
            return true;
        } break;
    }
}

static bool object_is_equal(const object_t* left, const object_t* right)
{
    if (left->type != right->type)
    {
        return false;
    }

    switch (left->type)
    {
        case OBJECT_STRING:
        {
            return (strcmp(left->val.str, right->val.str) == 0);
        } break;

        case OBJECT_NIL:
        {
            return true;
        } break;

        // this will work for any object type where the value is encoded in val
        default:
        {
            return left->val.num == right->val.num;
        } break;
    }
}

object_t interpret_expr(expr_t* expr, const char** out_err)
{
    object_t obj = {0};
    const char* err = NULL;

    switch (expr->type)
    {
        case EXPR_LITERAL:
        {
            expr_literal_t* e = (expr_literal_t*)expr;
            obj.type = e->type; 
            
            switch (obj.type)
            {
                case OBJECT_STRING:
                {
                    obj.val.str = malloc(sizeof(char) * (strlen(e->val.str) + 1));
                    strcpy(obj.val.str, e->val.str);
                } break;

                case OBJECT_BOOL:
                {
                    obj.val.boolean = e->val.boolean;
                } break;

                case OBJECT_NUMBER:
                {
                    obj.val.num = e->val.num;
                } break;

                case OBJECT_NIL:
                {
                    // TODO(omar): implement nil (everywhere)
                } break;

                default: assert(false);
            }

        } break;

        case EXPR_UNARY:
        {
            expr_unary_t* e = (expr_unary_t*)expr;
            object_t right = interpret_expr(e->right, &err);
            if (err) {
                break;
            }

            switch (e->operator.type)
            {
                case TOKEN_MINUS:
                {
                    if (right.type == OBJECT_NUMBER) {
                        obj.type = OBJECT_NUMBER;
                        obj.val.num = -(right.val.num);
                    }
                    else {
                        err = "Operand must be a number";
                        break;
                    }
                } break;

                case TOKEN_BANG:
                {
                    obj.type = OBJECT_BOOL;
                    obj.val.boolean = !(object_is_truthy(&right));
                } break;

                default: assert(false);
            }
        } break;

        case EXPR_BINARY:
        {
            expr_binary_t* e = (expr_binary_t*)expr;
            object_t left = interpret_expr(e->left, NULL);
            object_t right = interpret_expr(e->right, NULL);
            if (err) {
                break;
            }

            switch (e->operator.type)
            {
                case TOKEN_STAR:
                {
                    if ((left.type != right.type) || (left.type != OBJECT_NUMBER)) {
                        err = "Operands must be numbers";
                        break;
                    }

                    obj.type = OBJECT_NUMBER;
                    obj.val.num = left.val.num * right.val.num;
                }  break;

                case TOKEN_MINUS:
                {
                    if ((left.type != right.type) || (left.type != OBJECT_NUMBER)) {
                        err = "Operands must be numbers";
                        break;
                    }

                    obj.type = OBJECT_NUMBER;
                    obj.val.num = left.val.num - right.val.num;
                } break;

                case TOKEN_SLASH:
                {
                    if ((left.type != right.type) || (left.type != OBJECT_NUMBER)) {
                        err = "Operands must be numbers";
                        break;
                    }

                    obj.type = OBJECT_NUMBER;
                    obj.val.num = left.val.num / right.val.num;
                } break;

                case TOKEN_PLUS:
                {
                    if ((left.type == OBJECT_NUMBER) && (right.type == OBJECT_NUMBER)) {
                        obj.type = OBJECT_NUMBER;
                        obj.val.num = left.val.num + right.val.num;
                    }
                    else if ((left.type == OBJECT_STRING) && (right.type == OBJECT_STRING)) {
                        size_t str_len = strlen(left.val.str) + strlen(left.val.str);
                        char* str = malloc(sizeof(char) * (str_len + 1));

                        snprintf(str, str_len + 1, "%s%s", left.val.str, right.val.str);

                        obj.type = OBJECT_STRING;
                        obj.val.str = str;
                    }
                    else {
                        err = "Operands must be two numbers or two strings";
                        break;
                    }
                } break;

                case TOKEN_GREATER:
                {
                    if ((left.type != right.type) || (left.type != OBJECT_NUMBER)) {
                        err = "Operands must be numbers";
                        break;
                    }

                    obj.type = OBJECT_BOOL;
                    obj.val.boolean = left.val.num > right.val.num;
                } break;

                case TOKEN_LESS:
                {
                    if ((left.type != right.type) || (left.type != OBJECT_NUMBER)) {
                        err = "Operands must be numbers";
                        break;
                    }

                    obj.type = OBJECT_BOOL;
                    obj.val.boolean = left.val.num < right.val.num;
                } break;

                case TOKEN_GREATER_EQUAL:
                {
                    if ((left.type != right.type) || (left.type != OBJECT_NUMBER)) {
                        err = "Operands must be numbers";
                        break;
                    }

                    obj.type = OBJECT_BOOL;
                    obj.val.boolean = left.val.num >= right.val.num;
                } break;

                case TOKEN_LESS_EQUAL:
                {
                    if ((left.type != right.type) || (left.type != OBJECT_NUMBER)) {
                        err = "Operands must be numbers";
                        break;
                    }

                    obj.type = OBJECT_BOOL;
                    obj.val.boolean = left.val.num <= right.val.num;
                } break;

                case TOKEN_BANG_EQUAL:
                {
                    obj.type = OBJECT_BOOL;
                    obj.val.boolean = !object_is_equal(&left, &right) ;
                } break;

                case TOKEN_EQUAL_EQUAL:
                {
                    obj.type = OBJECT_BOOL;
                    obj.val.boolean = object_is_equal(&left, &right) ;
                } break;
            }

            free_object(&left);
            free_object(&right);
        } break;

        default: assert(false);
    }

    if (out_err)
    {
        *out_err = err;
    }
    return obj;
}

void free_object(object_t* obj)
{
    if (obj->type == OBJECT_STRING) {
        free(obj->val.str);
    }
}

void print_object(const object_t *obj)
{
    switch (obj->type)
    {
        case OBJECT_BOOL:
        {
            if (obj->val.boolean) {
                printf("true\n");
            }
            else {
                printf("false\n");
            }
        } break;

        case OBJECT_NUMBER:
        {
            printf("%f\n", obj->val.num);
        } break;

        case OBJECT_STRING:
        {
            printf("%s\n", obj->val.str);
        } break;

        case OBJECT_NIL:
        {
            printf("nil\n");
        } break;

        default: assert(false);
    }
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
                case OBJECT_BOOL:
                {
                    const char* bool_str = "false";
                    if (e->val.boolean) {
                        bool_str = "true" ;
                    }

                    str = malloc(sizeof(char) * (strlen(bool_str) + 1));
                    strcpy(str, bool_str);
                } break;

                case OBJECT_NUMBER:
                {
                    int str_size = snprintf(NULL, 0, "%f", e->val.num) + 1;

                    str = malloc(str_size);

                    snprintf(str, str_size, "%f", e->val.num);
                } break;

                case OBJECT_STRING:
                {
                    str = malloc(strlen(e->val.str) + 1);
                    strcpy(str, e->val.str);
                } break;

                case OBJECT_NIL:
                {
                    
                } break;
            }

            assert(str);
            return str;
        } break;
    }

    return NULL;
}