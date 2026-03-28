// TODOS():

// TODO(omar): add token info to errors
// TODO(omar): handle division by zero ourselves

#include "parser.h"
#include "interpreter.h"

#include <assert.h>

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static declaration_t* parse_declaration();
static stmt_t* parse_stmt();
static expr_t* parse_expr();

static expr_t* parse_series();
static expr_t* parse_equality();
static expr_t* parse_comparison();
static expr_t* parse_term();
static expr_t* parse_factor();
static expr_t* parse_unary();
static expr_t* parse_primary();

static char* expr_to_str(const expr_t* expr);
static char* stmt_to_str(const stmt_t* stmt);
static char* declaration_to_str(const declaration_t* d);

static bool str_equal(const char* str, int n, ...)
{
    if (!str)
    {
        return false;
    }

    va_list args;
    va_start(args, n);

    bool equal = false;

    for (int i = 0; i < n; i++)
    {
        const char* str2 = va_arg(args, const char*);
        assert(str2);

        if (strcmp(str, str2) == 0)
        {
            equal = true;
            break;
        }
    }

    va_end(args);
    return equal;
}

static int curr = 0;
static token_list_t* tokens = NULL;

static inline token_t get_token(bool advance)
{
    token_t tok = { 0 };
    tok.type = TOKEN_NONE;

    if (curr < tokens->len)
    {
        tok = tokens->tokens[curr];
        if (advance)
        {
            curr++;
        }
    }

    return tok;
}

declaration_list_t parse(token_list_t* _tokens, bool* has_error)
{
    curr = 0;
    tokens = _tokens;

    declaration_list_t list = { 0 };
    while (curr < tokens->len)
    {
        declaration_t* stmt = parse_declaration();
        if (!stmt)
        {
            if (has_error)
            {
                *has_error = true;
            }
            break;
        }

        declaration_list_push(&list, stmt);
    }

    return list;
}

void free_declaration(declaration_t* d)
{
    switch (d->type)
    {
    case DECLARATION_STMT:
    {
        declaration_stmt_t* decl = (declaration_stmt_t*)d;
        free_stmt(decl->stmt);
    }
    break;

    case DECLARATION_VAR:
    {
        declaration_var_t* decl = (declaration_var_t*)d;
        free_expr(decl->expr);
    }
    break;
    }

    free(d);
}

void print_declaration(const declaration_t* d)
{
    char* str = declaration_to_str(d);
    printf("%s\n", str);
    free(str);
}

static declaration_var_t* parse_var_declaration();
static declaration_stmt_t* parse_stmt_declaration();

static declaration_t* parse_declaration()
{
    token_t tok = get_token(false);
    if (tok.type == TOKEN_VAR)
    {
        curr++;

        return (declaration_t*)(parse_var_declaration());
    }

    return (declaration_t*)(parse_stmt_declaration());
}

static declaration_var_t* parse_var_declaration()
{
    token_t name = get_token(true);
    if (name.type != TOKEN_IDENTIFIER)
    {
        printf("Expected variable name.\n");
        return NULL;
    }

    expr_t* expr = NULL;
    token_t tok = get_token(false);
    if (tok.type == TOKEN_EQUAL)
    {
        curr++;
        expr = parse_expr();

        if (!expr)
        {
            return NULL;
        }
    }

    tok = get_token(false);
    if (tok.type != TOKEN_SEMICOLON)
    {
        printf("Expected ';' after variable declaration.\n");
        free_expr(expr);
        return NULL;
    }
    curr++;

    declaration_var_t* d = malloc(sizeof(declaration_var_t));
    d->d.type = DECLARATION_VAR;
    d->expr = expr;
    d->name = name;

    return d;
}

static declaration_stmt_t* parse_stmt_declaration()
{
    declaration_stmt_t* d = malloc(sizeof(declaration_stmt_t));
    d->d.type = DECLARATION_STMT;
    d->stmt = parse_stmt();

    if (d->stmt == NULL)
    {
        free(d);
        return NULL;
    }

    return d;
}

static stmt_print_t* parse_print_stmt();
static stmt_expr_t* parse_expr_stmt();

static stmt_t* parse_stmt()
{
    token_t tok = get_token(false);
    if (tok.type == TOKEN_PRINT)
    {
        curr++;

        return (stmt_t*)(parse_print_stmt());
    }

    return (stmt_t*)(parse_expr_stmt());
}

static stmt_print_t* parse_print_stmt()
{
    expr_t* expr = parse_expr();
    if (!expr)
    {
        return NULL;
    }

    token_t tok = get_token(true);
    if (tok.type != TOKEN_SEMICOLON)
    {
        printf("PARSER ERROR: Expected ';' after value.\n");
        free_expr(expr);
        return NULL;
    }

    stmt_print_t* stmt = malloc(sizeof(stmt_print_t));
    stmt->s.type = STMT_PRINT;
    stmt->expr = expr;

    return stmt;
}

static stmt_expr_t* parse_expr_stmt()
{
    expr_t* expr = parse_expr();
    if (!expr)
    {
        return NULL;
    }

    token_t tok = get_token(true);
    if (tok.type != TOKEN_SEMICOLON)
    {
        printf("PARSER ERROR: Expected ';' after value.\n");
        free_expr(expr);
        return NULL;
    }

    stmt_expr_t* stmt = malloc(sizeof(stmt_expr_t));
    stmt->s.type = STMT_EXPR;
    stmt->expr = expr;

    return stmt;
}

void declaration_list_push(declaration_list_t* list, declaration_t* stmt)
{
    assert(list);
    if (!stmt)
    {
        return;
    }

    list->len++;

    if (list->stmts)
    {
        list->stmts = realloc(list->stmts, sizeof(declaration_t*) * list->len);
    }
    else
    {
        list->stmts = malloc(sizeof(declaration_t*) * list->len);
    }

    list->stmts[list->len - 1] = stmt;
}

void declaration_list_print(const declaration_list_t* list)
{
    assert(list);

    for (int i = 0; i < list->len; i++)
    {
        char* str = declaration_to_str(list->stmts[i]);
        printf("%s\n", str);
        free(str);
    }
}

void declaration_list_free(declaration_list_t* list)
{
    assert(list);

    for (int i = 0; i < list->len; i++)
    {
        free_declaration(list->stmts[i]);
    }

    free(list->stmts);
}

static expr_t* parse_expr()
{
    return parse_series();
}

void free_stmt(stmt_t* stmt)
{
    if (!stmt)
    {
        return;
    }

    switch (stmt->type)
    {
    case STMT_EXPR:
    {
        stmt_expr_t* s = (stmt_expr_t*)stmt;
        free_expr(s->expr);
    }
    break;

    case STMT_PRINT:
    {
        stmt_print_t* s = (stmt_print_t*)stmt;
        free_expr(s->expr);
    }
    break;

    default:
        assert(false);
    }

    free(stmt);
}

void print_stmt(const stmt_t* stmt)
{
    char* str = stmt_to_str(stmt);
    printf("%s\n", str);
    free(str);
}

void free_expr(expr_t* expr)
{
    if (!expr)
    {
        return;
    }

    switch (expr->type)
    {
    case EXPR_BINARY:
    {
        expr_binary_t* e = (expr_binary_t*)expr;

        free_expr(e->left);
        free_expr(e->right);
    }
    break;

    case EXPR_GROUPING:
    {
        expr_grouping_t* e = (expr_grouping_t*)expr;

        free_expr(e->inner);
    }
    break;

    case EXPR_LITERAL:
    {
        expr_literal_t* e = (expr_literal_t*)expr;
        (void)e;
    }
    break;

    case EXPR_UNARY:
    {
        expr_unary_t* e = (expr_unary_t*)expr;

        free_expr(e->right);
    }
    break;

    case EXPR_VAR:
    {
        expr_var_t* e = (expr_var_t*)expr;

        (void)e;
    }
    break;

    default:
        assert(false);
    }

    free(expr);
}

void print_expr(const expr_t* expr)
{
    char* str = expr_to_str(expr);
    printf("%s\n", str);
    free(str);
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

static inline expr_var_t* new_var_expr()
{
    expr_var_t* expr = calloc(1, sizeof(expr_var_t));
    expr->e.type = EXPR_VAR;

    return expr;
}

static expr_t* parse_series()
{
    expr_t* expr = parse_equality();
    if (expr == NULL)
    {
        return NULL;
    }

    token_t tok = get_token(false);
    while (str_equal(tok.lexeme, 1, ","))
    {
        curr++;

        expr_t* right = parse_equality();

        expr_binary_t* new_expr = new_binary_expr();

        new_expr->left = expr;
        new_expr->right = right;
        new_expr->operator = tok;

        expr = (expr_t*)new_expr;

        tok = get_token(false);
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

    token_t tok = get_token(false);
    while (str_equal(tok.lexeme, 2, "==", "!="))
    {
        curr++;

        expr_t* right = parse_comparison();

        expr_binary_t* new_expr = new_binary_expr();
        new_expr->left = expr;
        new_expr->right = right;
        new_expr->operator = tok;

        expr = (expr_t*)new_expr;

        tok = get_token(false);
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

    token_t tok = get_token(false);
    while (str_equal(tok.lexeme, 4, ">", ">=", "<=", "<"))
    {
        curr++;

        expr_t* right = parse_term();

        expr_binary_t* new_expr = new_binary_expr();
        new_expr->left = expr;
        new_expr->right = right;
        new_expr->operator = tok;

        expr = (expr_t*)new_expr;

        tok = get_token(false);
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

    token_t tok = get_token(false);
    while (str_equal(tok.lexeme, 2, "-", "+"))
    {
        curr++;

        expr_t* right = parse_factor();

        expr_binary_t* new_expr = new_binary_expr();
        new_expr->left = expr;
        new_expr->right = right;
        new_expr->operator = tok;

        expr = (expr_t*)new_expr;

        tok = get_token(false);
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

    token_t tok = get_token(false);
    while (str_equal(tok.lexeme, 2, "/", "*"))
    {
        curr++;

        expr_t* right = parse_unary();

        expr_binary_t* new_expr = new_binary_expr();
        new_expr->left = expr;
        new_expr->right = right;
        new_expr->operator = tok;

        expr = (expr_t*)new_expr;

        tok = get_token(false);
    }

    return (expr_t*)expr;
}

static expr_t* parse_unary()
{
    token_t tok = get_token(false);
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
    token_t tok = get_token(false);

    switch (tok.type)
    {
    case TOKEN_FALSE:
    {
        curr++;

        expr_literal_t* expr = new_literal_expr();
        expr->type = OBJECT_BOOL;
        expr->val.boolean = false;

        return (expr_t*)expr;
    }
    break;

    case TOKEN_TRUE:
    {
        curr++;

        expr_literal_t* expr = new_literal_expr();
        expr->type = OBJECT_BOOL;
        expr->val.boolean = true;

        return (expr_t*)expr;
    }
    break;

    case TOKEN_NIL:
    {
        curr++;

        expr_literal_t* expr = new_literal_expr();
        expr->type = OBJECT_NIL;

        return (expr_t*)expr;
    }
    break;

    case TOKEN_STRING:
    {
        curr++;

        expr_literal_t* expr = new_literal_expr();
        expr->type = OBJECT_STRING;
        expr->val.str = tok.lexeme;

        return (expr_t*)expr;
    }
    break;

    case TOKEN_NUMBER:
    {
        curr++;

        expr_literal_t* expr = new_literal_expr();
        expr->type = OBJECT_NUMBER;
        expr->val.num = (float)(atof(tok.lexeme));

        return (expr_t*)expr;
    }
    break;

    case TOKEN_LEFT_PAREN:
    {
        curr++;

        expr_t* inner = parse_expr();
        token_t current = get_token(false);

        if (!str_equal(current.lexeme, 1, ")"))
        {
            printf("PARSER ERROR: Expected ')' after expression\n");
            free_expr(inner);
            return NULL;
        }

        expr_grouping_t* expr = new_grouping_expr();
        expr->inner = inner;
        return (expr_t*)expr;
    }
    break;

    case TOKEN_IDENTIFIER:
    {
        curr++;

        expr_var_t* expr = new_var_expr();
        expr->name = tok;

        return (expr_t*)expr;
    }
    break;
    }

    printf("PARSER ERROR: Expected an expression\n");
    return NULL;
}

static char* stmt_to_str(const stmt_t* stmt)
{
    if (!stmt)
    {
        return NULL;
    }

    switch (stmt->type)
    {
    case STMT_EXPR:
    {
        stmt_expr_t* s = (stmt_expr_t*)stmt;
        const char* prefix = "EXPRESSION STATEMENT: ";

        char* exp_str = expr_to_str(s->expr);
        size_t str_size = sizeof(char) * (strlen(exp_str) + strlen(prefix) + 1);

        char* str = malloc(str_size);
        snprintf(str, str_size, "%s%s", prefix, exp_str);
        free(exp_str);

        return str;
    }
    break;

    case STMT_PRINT:
    {
        stmt_print_t* s = (stmt_print_t*)stmt;
        const char* prefix = "PRINT STATEMENT: ";

        char* exp_str = expr_to_str(s->expr);
        size_t str_size = sizeof(char) * (strlen(exp_str) + strlen(prefix) + 1);

        char* str = malloc(str_size);
        snprintf(str, str_size, "%s%s", prefix, exp_str);
        free(exp_str);

        return str;
    }
    break;
    }

    return NULL;
}

static char* declaration_to_str(const declaration_t* d)
{
    if (!d)
    {
        return NULL;
    }

    switch (d->type)
    {
    case DECLARATION_STMT:
    {
        const declaration_stmt_t* decl = (declaration_stmt_t*)d;
        return stmt_to_str(decl->stmt);
    }
    break;

    case DECLARATION_VAR:
    {
        const declaration_var_t* decl = (declaration_var_t*)d;
        const char* format;
        size_t str_size;

        char* expr_str = expr_to_str(decl->expr);
        if (expr_str)
        {
            format = "%s declared to be %s";
            str_size = sizeof(char) * (strlen(expr_str) + strlen(format) + strlen(decl->name.lexeme) + 1);
            char* str = malloc(str_size);
            snprintf(str, str_size, format, decl->name.lexeme, expr_str);

            free(expr_str);
            return str;
        }
        else
        {
            format = "%s declared to be nil";
            str_size = sizeof(char) * (strlen(format) + strlen(decl->name.lexeme) + 1);
            char* str = malloc(str_size);
            snprintf(str, str_size, format, decl->name.lexeme);

            return str;
        }
    }
    break;
    }

    return NULL;
}

static char* expr_to_str(const expr_t* expr)
{
    if (!expr)
    {
        return NULL;
    }

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
    }
    break;

    case EXPR_UNARY:
    {
        const expr_unary_t* e = (expr_unary_t*)expr;
        char* right = expr_to_str(e->right);

        size_t str_size = sizeof(char) + (strlen(right) + strlen(e->operator.lexeme) + 1);
        char* str = malloc(str_size);
        snprintf(str, str_size, "%s%s", e->operator.lexeme, right);

        free(right);

        return str;
    }
    break;

    case EXPR_GROUPING:
    {
        const expr_grouping_t* e = (expr_grouping_t*)expr;
        char* inner = expr_to_str(e->inner);

        size_t str_size = sizeof(char) * (strlen(inner) + strlen("()") + 1);
        char* str = malloc(str_size);
        snprintf(str, str_size, "(%s)", inner);

        free(inner);

        return str;
    }
    break;

    case EXPR_LITERAL:
    {
        const expr_literal_t* e = (expr_literal_t*)expr;

        char* str = NULL;
        switch (e->type)
        {
        case OBJECT_BOOL:
        {
            const char* bool_str = "false";
            if (e->val.boolean)
            {
                bool_str = "true";
            }

            str = malloc(sizeof(char) * (strlen(bool_str) + 1));
            strcpy(str, bool_str);
        }
        break;

        case OBJECT_NUMBER:
        {
            int str_size = snprintf(NULL, 0, "%f", e->val.num) + 1;

            str = malloc(str_size);

            snprintf(str, str_size, "%f", e->val.num);
        }
        break;

        case OBJECT_STRING:
        {
            str = malloc(strlen(e->val.str) + 1);
            strcpy(str, e->val.str);
        }
        break;

        case OBJECT_NIL:
        {
        }
        break;
        }

        assert(str);
        return str;
    }
    break;

    case EXPR_VAR:
    {
        const expr_var_t* e = (expr_var_t*)expr;

        size_t str_size = sizeof(char) * (strlen(e->name.lexeme) + 1);
        char* str = malloc(str_size);
        snprintf(str, str_size, "%s", e->name.lexeme);

        return str;
    }
    break;
    }

    return NULL;
}