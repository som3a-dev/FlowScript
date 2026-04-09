/*
 * File: declaration.h
 * Created on Thu Apr 09 2026
 *
 * Copyright (c) 2026 Omar Eltayeb
 */

#ifndef _DECLARATION_H
#define _DECLARATION_H

#include "statement.h"

typedef enum
{
    DECLARATION_VAR, // variable declaration
    DECLARATION_BLOCK, // Block/Scope
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

/*
 * This is a list of declaration statements (like token_list_t), not a list
 * declaration statement
 */
typedef struct
{
    declaration_t** stmts;
    int len;
} declaration_list_t;

typedef struct
{
    declaration_t d;
    token_t name;
    expr_t* expr;
} declaration_var_t;

typedef struct
{
    declaration_t d;
    declaration_list_t stmts;
} declaration_block_t;

typedef struct
{
    declaration_t d;
    stmt_t* stmt;
} declaration_stmt_t;

void declaration_free(declaration_t* d);
void declaration_print(const declaration_t* d);
char* declaration_to_str(const declaration_t* d);

void declaration_list_push(declaration_list_t* list, declaration_t* stmt);
void declaration_list_print(const declaration_list_t* list);
void declaration_list_free(declaration_list_t* list);

#endif