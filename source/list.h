/*
 * File: list.h
 * Created on Sun Apr 12 2026
 *
 * Copyright (c) 2026 Omar Eltayeb
 */

#ifndef _LIST_H
#define _LIST_H

#include <stdbool.h>

typedef struct stmt_t stmt_t;
typedef struct expr_t expr_t;
typedef struct object_t object_t;
typedef struct token_t token_t;

/*
 * This is a list of statements (like token_list_t), not a list
 * statement
 */
typedef struct
{
    stmt_t** stmts;
    int len;
} list_stmt_t;

typedef struct
{
    expr_t** exprs;
    int len;
} list_expr_t;

// Holds references to objects, doesn't own lifetime
typedef struct
{
    const object_t** objects;
    int len;
} list_object_t;

// Holds shallow by value copies, doesn't own lifetime
typedef struct
{
    token_t* tokens;
    int len;
} list_token_t;

void list_stmt_push(list_stmt_t* list, const stmt_t* stmt);
void list_stmt_print(const list_stmt_t* list);
void list_stmt_free(list_stmt_t* list);

void list_expr_push(list_expr_t* list, const expr_t* expr);
void list_expr_print(const list_expr_t* list);
void list_expr_free(list_expr_t* list);

void list_object_push(list_object_t* list, object_t* obj);
void list_object_print(const list_object_t* list);
void list_object_free(list_object_t* list);

void list_token_push(list_token_t* list, token_t tok);
void list_token_free(list_token_t* list);

#endif