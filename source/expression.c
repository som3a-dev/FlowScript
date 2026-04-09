/*
 * File: expression.c
 * Created on Thu Apr 09 2026
 *
 * Copyright (c) 2026 Omar Eltayeb
 */

#include "expression.h"

#include <assert.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void expr_free(expr_t* expr)
{
    if (!expr)
    {
        return;
    }

    switch (expr->type)
    {
    case EXPR_BINARY:
    {
        expr_binary_t* e = (expr_binary_t*)expr;

        expr_free(e->left);
        expr_free(e->right);
    }
    break;

    case EXPR_GROUPING:
    {
        expr_grouping_t* e = (expr_grouping_t*)expr;

        expr_free(e->inner);
    }
    break;

    case EXPR_LITERAL:
    {
        expr_literal_t* e = (expr_literal_t*)expr;
        (void)e;
    }
    break;

    case EXPR_UNARY:
    {
        expr_unary_t* e = (expr_unary_t*)expr;

        expr_free(e->right);
    }
    break;

    case EXPR_VAR:
    {
        expr_var_t* e = (expr_var_t*)expr;

        (void)e;
    }
    break;

    case EXPR_ASSIGN:
    {
        expr_assign_t* e = (expr_assign_t*)expr;
        expr_free(e->val);
    }
    break;

    default:
        assert(false);
    }

    free(expr);
}

void expr_print(const expr_t* expr)
{
    char* str = expr_to_str(expr);
    printf("%s\n", str);
    free(str);
}

char* expr_to_str(const expr_t* expr)
{
    if (!expr)
    {
        return NULL;
    }

    switch (expr->type)
    {
    case EXPR_BINARY:
    {
        const expr_binary_t* e = (expr_binary_t*)expr;
        char* left = expr_to_str(e->left);
        char* right = expr_to_str(e->right);

        size_t str_size = sizeof(char) * (strlen(left) + strlen(right) + strlen(e->operator.lexeme) + 1 + 4); // accounting for null terminator and spaces
        char* str = malloc(str_size);
        snprintf(str, str_size, "(%s %s %s)", e->operator.lexeme, left, right);

        free(left);
        free(right);

        return str;
    }
    break;

    case EXPR_UNARY:
    {
        const expr_unary_t* e = (expr_unary_t*)expr;
        char* right = expr_to_str(e->right);

        size_t str_size = sizeof(char) + (strlen(right) + strlen(e->operator.lexeme) + 1);
        char* str = malloc(str_size);
        snprintf(str, str_size, "%s%s", e->operator.lexeme, right);

        free(right);

        return str;
    }
    break;

    case EXPR_GROUPING:
    {
        const expr_grouping_t* e = (expr_grouping_t*)expr;
        char* inner = expr_to_str(e->inner);

        size_t str_size = sizeof(char) * (strlen(inner) + strlen("()") + 1);
        char* str = malloc(str_size);
        snprintf(str, str_size, "(%s)", inner);

        free(inner);

        return str;
    }
    break;

    case EXPR_LITERAL:
    {
        const expr_literal_t* e = (expr_literal_t*)expr;

        char* str = NULL;
        switch (e->type)
        {
        case OBJECT_BOOL:
        {
            const char* bool_str = "false";
            if (e->val.boolean)
            {
                bool_str = "true";
            }

            str = malloc(sizeof(char) * (strlen(bool_str) + 1));
            strcpy(str, bool_str);
        }
        break;

        case OBJECT_NUMBER:
        {
            int str_size = snprintf(NULL, 0, "%f", e->val.num) + 1;

            str = malloc(str_size);

            snprintf(str, str_size, "%f", e->val.num);
        }
        break;

        case OBJECT_STRING:
        {
            str = malloc(strlen(e->val.str) + 1);
            strcpy(str, e->val.str);
        }
        break;

        case OBJECT_NIL:
        {
        }
        break;
        }

        assert(str);
        return str;
    }
    break;

    case EXPR_VAR:
    {
        const expr_var_t* e = (expr_var_t*)expr;

        size_t str_size = sizeof(char) * (strlen(e->name.lexeme) + 1);
        char* str = malloc(str_size);
        snprintf(str, str_size, "%s", e->name.lexeme);

        return str;
    }
    break;
    }

    return NULL;
}