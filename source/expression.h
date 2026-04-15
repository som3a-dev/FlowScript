/*
 * File: expression.h
 * Created on Thu Apr 09 2026
 *
 * Copyright (c) 2026 Omar Eltayeb
 */

#ifndef _EXPRESSION_H
#define _EXPRESSION_H

#include "list.h"
#include "object.h"
#include "scanner.h"

typedef enum
{
    EXPR_ASSIGN,
    EXPR_BINARY,
    EXPR_GROUPING,
    EXPR_LITERAL,
    EXPR_UNARY,
    EXPR_CALL, // Function call
    EXPR_VAR,
    EXPR_LOGICAL
} expr_type_t;

typedef struct expr_t
{
    expr_type_t type;
} expr_t;

typedef struct
{
    expr_t e;

    token_t name;
    expr_t* val;
} expr_assign_t;

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

    expr_t* left;
    token_t operator;
    expr_t* right;
} expr_logical_t;

typedef struct
{
    expr_t e;

    token_t operator;
    expr_t* right;
} expr_unary_t;

typedef struct
{
    expr_t e;

    expr_t* callee;
    token_t closing_paren;
    list_expr_t args;
} expr_call_t;

typedef struct
{
    expr_t e;

    // technically an object and a literal are not the same thing
    // and probably shouldn't share a type enum
    // but we don't care for now
    object_type_t type;
    union
    {
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
    expr_t e;

    token_t name;
} expr_var_t;

void expr_free(expr_t* expr);
void expr_print(const expr_t* expr);

char* expr_to_str(const expr_t* expr);

#endif