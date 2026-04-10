/*
 * File: statement.c
 * Created on Thu Apr 09 2026
 *
 * Copyright (c) 2026 Omar Eltayeb
 */

#include "statement.h"

#include <assert.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void stmt_free(stmt_t* stmt)
{
    if (!stmt)
    {
        return;
    }

    switch (stmt->type)
    {
    case STMT_EXPR:
    {
        stmt_expr_t* s = (stmt_expr_t*)stmt;
        expr_free(s->expr);
    }
    break;

    case STMT_PRINT:
    {
        stmt_print_t* s = (stmt_print_t*)stmt;
        expr_free(s->expr);
    }
    break;

    case STMT_VAR:
    {
        stmt_var_t* s = (stmt_var_t*)stmt;
        expr_free(s->expr);
    }
    break;

    case STMT_BLOCK:
    {
        stmt_block_t* s = (stmt_block_t*)stmt;
        stmt_list_free(&(s->stmts));
    }
    break;

    default:
        assert(false);
    }

    free(stmt);
}

void stmt_print(const stmt_t* stmt)
{
    char* str = stmt_to_str(stmt);
    printf("%s\n", str);
    free(str);
}

char* stmt_to_str(const stmt_t* stmt)
{
    if (!stmt)
    {
        return NULL;
    }

    switch (stmt->type)
    {
    case STMT_EXPR:
    {
        stmt_expr_t* s = (stmt_expr_t*)stmt;
        const char* prefix = "EXPRESSION STATEMENT: ";

        char* exp_str = expr_to_str(s->expr);
        size_t str_size = sizeof(char) * (strlen(exp_str) + strlen(prefix) + 1);

        char* str = malloc(str_size);
        snprintf(str, str_size, "%s%s", prefix, exp_str);
        free(exp_str);

        return str;
    }
    break;

    case STMT_PRINT:
    {
        stmt_print_t* s = (stmt_print_t*)stmt;
        const char* prefix = "PRINT STATEMENT: ";

        char* exp_str = expr_to_str(s->expr);
        size_t str_size = sizeof(char) * (strlen(exp_str) + strlen(prefix) + 1);

        char* str = malloc(str_size);
        snprintf(str, str_size, "%s%s", prefix, exp_str);
        free(exp_str);

        return str;
    }
    break;
    }

    return NULL;
}

void stmt_list_push(stmt_list_t* list, stmt_t* stmt)
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

    list->stmts[list->len - 1] = stmt;
}

void stmt_list_print(const stmt_list_t* list)
{
    assert(list);

    for (int i = 0; i < list->len; i++)
    {
        char* str = stmt_to_str(list->stmts[i]);
        printf("%s\n", str);
        free(str);
    }
}

void stmt_list_free(stmt_list_t* list)
{
    assert(list);

    for (int i = 0; i < list->len; i++)
    {
        stmt_free(list->stmts[i]);
    }

    free(list->stmts);
}