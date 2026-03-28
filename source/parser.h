/*
 * File: parser.h
 * Created on Sun Mar 22 2026
 *
 * Copyright (c) 2026 Omar Eltayeb
 */

#ifndef _PARSER_H
#define _PARSER_H

#include "object.h"
#include "scanner.h"

typedef enum
{
    EXPR_BINARY,
    EXPR_GROUPING,
    EXPR_LITERAL,
    EXPR_UNARY,
    EXPR_VAR
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

// STATEMENTS

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

typedef enum
{
    DECLARATION_VAR, // variable declaration
    DECLARATION_STMT // just a normal statement that doesn't declare anything
} declaration_type_t;

// A declaration is a statement that, may or may not declare a name.
// A declaration can have a type of DECLARATION_STMT, which means it just falls
// through to a normal statement and doesn't declare anything,
// its a bit confusing, but this is how we ensure some types of statements
// (like conditionals) are not allowed to declare names
typedef struct
{
    declaration_type_t type;
} declaration_t;

typedef struct
{
    declaration_t d;
    token_t name;
    expr_t* expr;
} declaration_var_t;

typedef struct
{
    declaration_t d;
    stmt_t* stmt;
} declaration_stmt_t;

/*
 * This is a list of declaration statements (like token_list_t), not a list
 * declaration statement
 */
typedef struct
{
    declaration_t** stmts;
    int len;
} declaration_list_t;

declaration_list_t parse(token_list_t* _tokens, bool* has_error);

void free_declaration(declaration_t* d);
void print_declaration(const declaration_t* d);

void free_expr(expr_t* expr);
void print_expr(const expr_t* expr);

void free_stmt(stmt_t* stmt);
void print_stmt(const stmt_t* stmt);

void declaration_list_push(declaration_list_t* list, declaration_t* stmt);
void declaration_list_print(const declaration_list_t* list);
void declaration_list_free(declaration_list_t* list);

#endif