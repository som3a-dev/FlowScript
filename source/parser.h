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


//STATEMENTS

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

/*
* This is a list of statements (like token_list_t), not a list statement
*/
typedef struct
{
    stmt_t** stmts;
    int len;
} stmt_list_t;

stmt_list_t parse(token_list_t* _tokens);

void free_expr(expr_t* expr);
void print_expr(const expr_t* expr);

void free_stmt(stmt_t* stmt);
void print_stmt(const stmt_t* stmt);

void stmt_list_push(stmt_list_t* list, stmt_t* stmt);
void stmt_list_print(const stmt_list_t* list);
void stmt_list_free(stmt_list_t* list);

#endif