/*
 * File: parser.c
 * Created on Sun Mar 22 2026
 *
 * Copyright (c) 2026 Omar Eltayeb
 */

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
        expr_free(expr);
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

static stmt_print_t* parse_stmt_print();
static stmt_expr_t* parse_expr_stmt();

static stmt_t* parse_stmt()
{
    token_t tok = get_token(false);
    if (tok.type == TOKEN_PRINT)
    {
        curr++;

        return (stmt_t*)(parse_stmt_print());
    }

    return (stmt_t*)(parse_expr_stmt());
}

static stmt_print_t* parse_stmt_print()
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
        expr_free(expr);
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
        expr_free(expr);
        return NULL;
    }

    stmt_expr_t* stmt = malloc(sizeof(stmt_expr_t));
    stmt->s.type = STMT_EXPR;
    stmt->expr = expr;

    return stmt;
}

static expr_t* parse_expr()
{
    return parse_series();
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
            expr_free(inner);
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