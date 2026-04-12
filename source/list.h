/*
 * File: list.h
 * Created on Sun Apr 12 2026
 *
 * Copyright (c) 2026 Omar Eltayeb
 */

#ifndef _LIST_H
#define _LIST_H

typedef struct stmt_t stmt_t;

/*
 * This is a list of statements (like token_list_t), not a list
 * statement
 */
typedef struct
{
    stmt_t** stmts;
    int len;
} list_stmt_t;

void list_stmt_push(list_stmt_t* list, stmt_t* stmt);
void list_stmt_print(const list_stmt_t* list);
void list_stmt_free(list_stmt_t* list);

#endif