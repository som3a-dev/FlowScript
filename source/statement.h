/*
 * File: statement.h
 * Created on Thu Apr 09 2026
 *
 * Copyright (c) 2026 Omar Eltayeb
 */

#ifndef _STATEMENT_H
#define _STATEMENT_H

#include "expression.h"

typedef enum
{
    STMT_EXPR,
    STMT_PRINT,
    STMT_IF,
    STMT_VAR, // variable declaration
    STMT_BLOCK // Block/Scope
} stmt_type_t;

typedef struct
{
    stmt_type_t type;
} stmt_t;

/*
 * This is a list of statements (like token_list_t), not a list
 * statement
 */
typedef struct
{
    stmt_t** stmts;
    int len;
} stmt_list_t;

typedef struct
{
    stmt_t s;
    expr_t* expr;
} stmt_expr_t;

typedef struct
{
    stmt_t s;
    expr_t* expr;
} stmt_print_t;

typedef struct
{
    stmt_t s;
    token_t name;
    expr_t* expr;
} stmt_var_t;

typedef struct
{
    stmt_t s;
    stmt_list_t stmts;
} stmt_block_t;

typedef struct
{
    stmt_t s;

    expr_t* condition;
    stmt_t* then_branch;
    stmt_t* else_branch;
} stmt_if_t;

void stmt_free(stmt_t* stmt);
void stmt_print(const stmt_t* stmt);

char* stmt_to_str(const stmt_t* stmt);

void stmt_list_push(stmt_list_t* list, stmt_t* stmt);
void stmt_list_print(const stmt_list_t* list);
void stmt_list_free(stmt_list_t* list);

#endif