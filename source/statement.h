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
    STMT_PRINT
} stmt_type_t;

typedef struct
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

void stmt_free(stmt_t* stmt);
void stmt_print(const stmt_t* stmt);

char* stmt_to_str(const stmt_t* stmt);

#endif