/*
 * File: declaration.c
 * Created on Thu Apr 09 2026
 *
 * Copyright (c) 2026 Omar Eltayeb
 */

#include "declaration.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void declaration_free(declaration_t* d)
{
    switch (d->type)
    {
    case DECLARATION_STMT:
    {
        declaration_stmt_t* decl = (declaration_stmt_t*)d;
        stmt_free(decl->stmt);
    }
    break;

    case DECLARATION_VAR:
    {
        declaration_var_t* decl = (declaration_var_t*)d;
        expr_free(decl->expr);
    }
    break;
    }

    free(d);
}

void declaration_print(const declaration_t* d)
{
    char* str = declaration_to_str(d);
    printf("%s\n", str);
    free(str);
}

char* declaration_to_str(const declaration_t* d)
{
    if (!d)
    {
        return NULL;
    }

    switch (d->type)
    {
    case DECLARATION_STMT:
    {
        const declaration_stmt_t* decl = (declaration_stmt_t*)d;
        return stmt_to_str(decl->stmt);
    }
    break;

    case DECLARATION_VAR:
    {
        const declaration_var_t* decl = (declaration_var_t*)d;
        const char* format;
        size_t str_size;

        char* expr_str = expr_to_str(decl->expr);
        if (expr_str)
        {
            format = "%s declared to be %s";
            str_size = sizeof(char) * (strlen(expr_str) + strlen(format) + strlen(decl->name.lexeme) + 1);
            char* str = malloc(str_size);
            snprintf(str, str_size, format, decl->name.lexeme, expr_str);

            free(expr_str);
            return str;
        }
        else
        {
            format = "%s declared to be nil";
            str_size = sizeof(char) * (strlen(format) + strlen(decl->name.lexeme) + 1);
            char* str = malloc(str_size);
            snprintf(str, str_size, format, decl->name.lexeme);

            return str;
        }
    }
    break;
    }

    return NULL;
}

void declaration_list_push(declaration_list_t* list, declaration_t* stmt)
{
    assert(list);
    if (!stmt)
    {
        return;
    }

    list->len++;

    if (list->stmts)
    {
        list->stmts = realloc(list->stmts, sizeof(declaration_t*) * list->len);
    }
    else
    {
        list->stmts = malloc(sizeof(declaration_t*) * list->len);
    }

    list->stmts[list->len - 1] = stmt;
}

void declaration_list_print(const declaration_list_t* list)
{
    assert(list);

    for (int i = 0; i < list->len; i++)
    {
        char* str = declaration_to_str(list->stmts[i]);
        printf("%s\n", str);
        free(str);
    }
}

void declaration_list_free(declaration_list_t* list)
{
    assert(list);

    for (int i = 0; i < list->len; i++)
    {
        declaration_free(list->stmts[i]);
    }

    free(list->stmts);
}