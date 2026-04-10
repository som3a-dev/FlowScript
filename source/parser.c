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

static stmt_t* parse_declaration();
static stmt_t* parse_stmt();
static expr_t* parse_expr();

static expr_t* parse_series();
static expr_t* parse_assignment();
static expr_t* parse_or();
static expr_t* parse_and();
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

stmt_list_t parse(token_list_t* _tokens, bool* has_error)
{
    curr = 0;
    tokens = _tokens;

    stmt_list_t list = { 0 };
    while (curr < tokens->len)
    {
        stmt_t* stmt = parse_declaration();
        if (!stmt)
        {
            if (has_error)
            {
                *has_error = true;
            }
            break;
        }

        stmt_list_push(&list, stmt);
    }

    return list;
}

static stmt_var_t* parse_declaration_var();

static stmt_t* parse_declaration()
{
    token_t tok = get_token(false);
    if (tok.type == TOKEN_VAR)
    {
        curr++;

        return (stmt_t*)(parse_declaration_var());
    }

    return (stmt_t*)(parse_stmt());
}

static stmt_var_t* parse_declaration_var()
{
    token_t name = get_token(true);
    if (name.type != TOKEN_IDENTIFIER)
    {
        printf("PARSER ERROR: Expected variable name.\n");
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
        printf("PARSER ERROR: Expected ';' after variable declaration.\n");
        expr_free(expr);
        return NULL;
    }
    curr++;

    stmt_var_t* s = malloc(sizeof(stmt_var_t));
    s->s.type = STMT_VAR;
    s->expr = expr;
    s->name = name;

    return s;
}

static stmt_print_t* parse_stmt_print();
static stmt_expr_t* parse_stmt_expr();
static stmt_if_t* parse_stmt_if();
static stmt_block_t* parse_stmt_block();
static stmt_while_t* parse_stmt_while();

static stmt_t* parse_stmt()
{
    token_t tok = get_token(false);
    if (tok.type == TOKEN_PRINT)
    {
        curr++;

        return (stmt_t*)(parse_stmt_print());
    }
    else if (tok.type == TOKEN_IF)
    {
        curr++;
        return (stmt_t*)(parse_stmt_if());
    }
    else if (tok.type == TOKEN_LEFT_BRACE)
    {
        curr++;

        return (stmt_t*)(parse_stmt_block());
    }
    else if (tok.type == TOKEN_WHILE)
    {
        curr++;

        return (stmt_t*)(parse_stmt_while());
    }

    return (stmt_t*)(parse_stmt_expr());
}

static stmt_while_t* parse_stmt_while()
{
    token_t tok = get_token(false);
    if (tok.type != TOKEN_LEFT_PAREN)
    {
        printf("PARSER ERROR: Expected '(' after 'while'.\n");
        return NULL;
    }
    curr++;

    expr_t* condition = parse_expr();
    if (!condition)
    {
        return NULL;
    }

    tok = get_token(false);
    if (tok.type != TOKEN_RIGHT_PAREN)
    {
        printf("PARSER ERROR: Expected ')' after condition.\n");
        expr_free(condition);
        return NULL;
    }
    curr++;

    stmt_t* body = parse_stmt();
    if (!body)
    {
        expr_free(condition);
        return NULL;
    }

    stmt_while_t* stmt = malloc(sizeof(stmt_while_t));
    stmt->s.type = STMT_WHILE;
    stmt->condition = condition;
    stmt->body = body;

    return stmt;
}

static stmt_block_t* parse_stmt_block()
{
    stmt_block_t* block = malloc(sizeof(stmt_block_t));
    block->s.type = STMT_BLOCK;
    block->stmts = (stmt_list_t) { 0 };

    token_t tok = get_token(false);
    while (tok.type != TOKEN_RIGHT_BRACE && tok.type != TOKEN_NONE)
    {
        stmt_list_push(&(block->stmts), parse_declaration());
        tok = get_token(false);
    }

    if (tok.type != TOKEN_RIGHT_BRACE)
    {
        printf("PARSER ERROR: Expected '}' at the end of the block.\n");
        stmt_free((stmt_t*)block);

        return NULL;
    }
    else
    {
        curr++;
    }

    return block;
}

static stmt_if_t* parse_stmt_if()
{
    token_t tok = get_token(true);
    if (tok.type != TOKEN_LEFT_PAREN)
    {
        printf("PARSER ERROR: Expected '(' after 'if'.\n");
        return NULL;
    }

    expr_t* condition = parse_expr();
    if (!condition)
    {
        return NULL;
    }

    tok = get_token(true);
    if (tok.type != TOKEN_RIGHT_PAREN)
    {
        printf("PARSER ERROR: Expected ')' after 'if'.\n");
        return NULL;
    }

    stmt_t* then_branch = parse_stmt();
    stmt_t* else_branch = NULL;

    tok = get_token(false);
    if (tok.type == TOKEN_ELSE)
    {
        curr++;
        else_branch = parse_stmt();
    }

    stmt_if_t* stmt = malloc(sizeof(stmt_if_t));
    stmt->s.type = STMT_IF;
    stmt->then_branch = then_branch;
    stmt->else_branch = else_branch;
    stmt->condition = condition;

    return stmt;
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

static stmt_expr_t* parse_stmt_expr()
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

static inline expr_assign_t* new_assign_expr()
{
    expr_assign_t* expr = calloc(1, sizeof(expr_assign_t));
    expr->e.type = EXPR_ASSIGN;

    return expr;
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

static inline expr_logical_t* new_logical_expr()
{
    expr_logical_t* expr = calloc(1, sizeof(expr_logical_t));
    expr->e.type = EXPR_LOGICAL;

    return expr;
}

static expr_t* parse_series()
{
    expr_t* expr = parse_assignment();
    if (expr == NULL)
    {
        return NULL;
    }

    token_t tok = get_token(false);
    while (str_equal(tok.lexeme, 1, ","))
    {
        curr++;

        expr_t* right = parse_assignment();

        expr_binary_t* new_expr = new_binary_expr();

        new_expr->left = expr;
        new_expr->right = right;
        new_expr->operator = tok;

        expr = (expr_t*)new_expr;

        tok = get_token(false);
    }

    return (expr_t*)expr;
}

static expr_t* parse_assignment()
{
    expr_t* expr = parse_or();
    if (expr == NULL)
    {
        return NULL;
    }

    token_t tok = get_token(false);
    if (tok.type == TOKEN_EQUAL)
    {
        curr++;

        if (expr->type == EXPR_VAR)
        {
            expr_var_t* var = (expr_var_t*)expr;
            token_t name = var->name;

            expr_t* val = parse_assignment();

            expr_assign_t* assign = new_assign_expr();
            assign->name = name;
            assign->val = val;

            expr_free(expr);
            expr = (expr_t*)assign;
        }
        else
        {
            printf("PARSER ERROR: Invalid assignment target\n");

            expr_free(expr);
            return NULL;
        }
    }

    return expr;
}

static expr_t* parse_or()
{
    expr_t* expr = parse_and();

    token_t tok = get_token(false);
    while (tok.type == TOKEN_OR)
    {
        curr++;
        expr_t* right = parse_and();

        expr_logical_t* logical = new_logical_expr();
        logical->left = expr;
        logical->right = right;
        logical->operator = tok;

        expr = (expr_t*)logical;

        tok = get_token(false);
    }

    return expr;
}

static expr_t* parse_and()
{
    expr_t* expr = parse_equality();

    token_t tok = get_token(false);
    while (tok.type == TOKEN_AND)
    {
        curr++;
        expr_t* right = parse_equality();

        expr_logical_t* logical = new_logical_expr();
        logical->left = expr;
        logical->right = right;
        logical->operator = tok;

        expr = (expr_t*)logical;

        tok = get_token(false);
    }

    return expr;
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