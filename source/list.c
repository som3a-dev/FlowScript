/*
 * File: list.c
 * Created on Sun Apr 12 2026
 *
 * Copyright (c) 2026 Omar Eltayeb
 */

#include "list.h"
#include "statement.h"

#include <assert.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void list_stmt_push(list_stmt_t* list, const stmt_t* stmt)
{
    assert(list);
    if (!stmt)
    {
        return;
    }

    list->len++;

    if (list->stmts)
    {
        list->stmts = realloc(list->stmts, sizeof(stmt_t*) * list->len);
    }
    else
    {
        list->stmts = malloc(sizeof(stmt_t*) * list->len);
    }

    list->stmts[list->len - 1] = (stmt_t*)stmt;
}

void list_stmt_print(const list_stmt_t* list)
{
    assert(list);

    for (int i = 0; i < list->len; i++)
    {
        char* str = stmt_to_str(list->stmts[i]);
        printf("%s\n", str);
        free(str);
    }
}

void list_stmt_free(list_stmt_t* list)
{
    assert(list);

    for (int i = 0; i < list->len; i++)
    {
        stmt_free(list->stmts[i]);
    }

    free(list->stmts);
}

void list_expr_push(list_expr_t* list, const expr_t* expr)
{
    assert(list);
    if (!expr)
    {
        return;
    }

    list->len++;

    if (list->exprs)
    {
        list->exprs = realloc(list->exprs, sizeof(expr_t*) * list->len);
    }
    else
    {
        list->exprs = malloc(sizeof(expr_t*) * list->len);
    }

    list->exprs[list->len - 1] = (expr_t*)expr;
}

void list_expr_print(const list_expr_t* list)
{
    assert(list);

    for (int i = 0; i < list->len; i++)
    {
        char* str = expr_to_str(list->exprs[i]);
        printf("%s\n", str);
        free(str);
    }
}

void list_expr_free(list_expr_t* list)
{
    assert(list);

    for (int i = 0; i < list->len; i++)
    {
        expr_free(list->exprs[i]);
    }

    free(list->exprs);
}

void list_object_push(list_object_t* list, object_t obj)
{
    assert(list);

    list->len++;

    if (list->objects)
    {
        list->objects = realloc(list->objects, sizeof(object_t) * list->len);
    }
    else
    {
        list->objects = malloc(sizeof(object_t) * list->len);
    }

    list->objects[list->len - 1] = obj;
}

void list_object_print(const list_object_t* list)
{
    assert(list);

    for (int i = 0; i < list->len; i++)
    {
        object_print(&(list->objects[i]));
    }
}

void list_object_free(list_object_t* list, bool free_objects)
{
    assert(list);

    if (free_objects)
    {
        for (int i = 0; i < list->len; i++)
        {
            object_free(&(list->objects[i]));
        }
    }

    free(list->objects);
}