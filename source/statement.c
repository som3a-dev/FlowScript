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
