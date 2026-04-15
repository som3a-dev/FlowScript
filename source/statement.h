/*
 * File: statement.h
 * Created on Thu Apr 09 2026
 *
 * Copyright (c) 2026 Omar Eltayeb
 */

#ifndef _STATEMENT_H
#define _STATEMENT_H

#include "expression.h"
#include "list.h"

typedef enum
{
    STMT_EXPR,
    STMT_PRINT,
    STMT_IF,
    STMT_VAR, // variable declaration
    STMT_BLOCK, // block/scope,
    STMT_WHILE, // while loop
    STMT_FUNCTION // Function declaration
} stmt_type_t;

typedef struct stmt_t
{
    stmt_type_t type;
} stmt_t;

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
    list_stmt_t stmts;
} stmt_block_t;

typedef struct
{
    stmt_t s;

    expr_t* condition;
    stmt_t* then_branch;
    stmt_t* else_branch;
} stmt_if_t;

typedef struct
{
    stmt_t s;

    expr_t* condition;
    stmt_t* body;
} stmt_while_t;

typedef struct
{
    stmt_t s;

    token_t name;
    list_token_t params;
    stmt_block_t* body;
} stmt_function_t;

void stmt_free(stmt_t* stmt);
void stmt_print(const stmt_t* stmt);

char* stmt_to_str(const stmt_t* stmt);

#endif