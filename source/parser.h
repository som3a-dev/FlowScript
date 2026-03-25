/*
 * File: parser.h
 * Created on Sun Mar 22 2026
 *
 * Copyright (c) 2026 Omar Eltayeb
 */

#ifndef _PARSER_H
#define _PARSER_H

#include "scanner.h"
#include "object.h"

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

expr_t* parse(token_list_t* _tokens);
void free_expr(expr_t* expr);

void print_expr(const expr_t* expr);

#endif