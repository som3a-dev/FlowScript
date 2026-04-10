/*
 * File: interpreter.c
 * Created on Tue Mar 24 2026
 *
 * Copyright (c) 2026 Omar Eltayeb
 */

#include "interpreter.h"
#include "environment.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void interpret_stmt(const stmt_t* stmt, const char** out_err);
static object_t interpret_expr(const expr_t* expr, const char** out_err);

static environment_t env = { 0 };

void init_interpreter()
{
    environment_init(&env, NULL);
}

void destroy_interpreter()
{
    environment_destroy(&env);
}

void interpret(const stmt_list_t* stmts)
{
    const char* err = NULL;
    for (int i = 0; i < stmts->len; i++)
    {
        interpret_stmt(stmts->stmts[i], &err);
        if (err)
        {
            printf("RUNTIME ERROR: %s\n", err);
            break;
        }
    }
}

static void interpret_stmt(const stmt_t* stmt, const char** out_err)
{
    if (!stmt)
    {
        if (out_err)
        {
            *out_err = "Invalid Statement";
        }

        return;
    }

    const char* err = NULL;

    switch (stmt->type)
    {
    case STMT_PRINT:
    {
        const stmt_print_t* s = (stmt_print_t*)stmt;
        object_t obj = interpret_expr(s->expr, &err);
        if (obj.type != _OBJECT_INVALID)
        {
            object_print(&obj);

            // TODO(omar): maybe interpret_expr() should return a copy of the object in the case of a var expression
            // so we don't have to do this
            if (s->expr->type != EXPR_VAR)
            {
                object_free(&obj);
            }
        }
    }
    break;

    case STMT_EXPR:
    {
        const stmt_expr_t* s = (stmt_expr_t*)stmt;
        object_t obj = interpret_expr(s->expr, &err);
        if (obj.type != _OBJECT_INVALID)
        {
            object_free(&obj);
        }
    }
    break;

    case STMT_VAR:
    {
        const stmt_var_t* decl = (stmt_var_t*)stmt;
        object_t val = { 0 };
        val.type = OBJECT_NIL;
        if (decl->expr)
        {
            val = interpret_expr(decl->expr, &err);
            if (err)
            {
                // TODO(omar): why does an invalid object not give us an error
                object_free(&val);
                return;
            }
        }

        environment_define(&env, decl->name.lexeme, &val);
        object_free(&val);
    }
    break;

    case STMT_BLOCK:
    {
        const stmt_block_t* block = (stmt_block_t*)stmt;
        environment_t prev_env = env;
        environment_init(&env, &prev_env);

        for (int i = 0; i < block->stmts.len; i++)
        {
            stmt_t* local_stmt = block->stmts.stmts[i];
            interpret_stmt(local_stmt, &err);
            if (err)
            {
                *out_err = err;
                return;
            }
        }

        environment_destroy(&env);
        env = prev_env;
    }
    break;

    case STMT_IF:
    {
        const stmt_if_t* s = (stmt_if_t*)stmt;
        object_t condition = interpret_expr(s->condition, &err);
        if (err)
        {
            *out_err = err;
            return;
        }

        if (object_is_truthy(&condition))
        {
            interpret_stmt(s->then_branch, &err);
            if (err)
            {
                *out_err = err;
                return;
            }
        }
        else if (s->else_branch)
        {
            interpret_stmt(s->else_branch, &err);
            if (err)
            {
                *out_err = err;
                return;
            }
        }
    }
    break;
    }
}

static object_t interpret_expr(const expr_t* expr, const char** out_err)
{
    object_t obj = { 0 };
    obj.type = _OBJECT_INVALID;

    if (!expr)
    {
        if (out_err)
        {
            *out_err = "Invalid Expression";
        }

        return obj;
    }

    const char* err = NULL;

    switch (expr->type)
    {
    case EXPR_ASSIGN:
    {
        expr_assign_t* e = (expr_assign_t*)expr;

        object_t val = interpret_expr(e->val, &err);
        if (err)
        {
            *out_err = err;
        }
        else
        {
            environment_assign(&env, e->name.lexeme, &val);
            obj = val;
        }
    }
    break;

    case EXPR_LITERAL:
    {
        expr_literal_t* e = (expr_literal_t*)expr;
        obj.type = e->type;

        switch (obj.type)
        {
        case OBJECT_STRING:
        {
            obj.val.str = malloc(sizeof(char) * (strlen(e->val.str) + 1));
            strcpy(obj.val.str, e->val.str);
        }
        break;

        case OBJECT_BOOL:
        {
            obj.val.boolean = e->val.boolean;
        }
        break;

        case OBJECT_NUMBER:
        {
            obj.val.num = e->val.num;
        }
        break;

        case OBJECT_NIL:
        {
            // TODO(omar): implement nil (everywhere)
        }
        break;

        default:
            assert(false);
        }
    }
    break;

    case EXPR_UNARY:
    {
        expr_unary_t* e = (expr_unary_t*)expr;
        object_t right = interpret_expr(e->right, &err);
        if (err)
        {
            break;
        }

        switch (e->operator.type)
        {
        case TOKEN_MINUS:
        {
            if (right.type == OBJECT_NUMBER)
            {
                obj.type = OBJECT_NUMBER;
                obj.val.num = -(right.val.num);
            }
            else
            {
                err = "Operand must be a number";
                break;
            }
        }
        break;

        case TOKEN_BANG:
        {
            obj.type = OBJECT_BOOL;
            obj.val.boolean = !(object_is_truthy(&right));
        }
        break;

        default:
            assert(false);
        }
    }
    break;

    case EXPR_BINARY:
    {
        expr_binary_t* e = (expr_binary_t*)expr;
        object_t left = interpret_expr(e->left, NULL);
        object_t right = interpret_expr(e->right, NULL);
        if (err)
        {
            break;
        }

        switch (e->operator.type)
        {
        case TOKEN_STAR:
        {
            if ((left.type != right.type) || (left.type != OBJECT_NUMBER))
            {
                err = "Operands must be numbers";
                break;
            }

            obj.type = OBJECT_NUMBER;
            obj.val.num = left.val.num * right.val.num;
        }
        break;

        case TOKEN_MINUS:
        {
            if ((left.type != right.type) || (left.type != OBJECT_NUMBER))
            {
                err = "Operands must be numbers";
                break;
            }

            obj.type = OBJECT_NUMBER;
            obj.val.num = left.val.num - right.val.num;
        }
        break;

        case TOKEN_SLASH:
        {
            if ((left.type != right.type) || (left.type != OBJECT_NUMBER))
            {
                err = "Operands must be numbers";
                break;
            }

            obj.type = OBJECT_NUMBER;
            obj.val.num = left.val.num / right.val.num;
        }
        break;

        case TOKEN_PLUS:
        {
            if ((left.type == OBJECT_NUMBER) && (right.type == OBJECT_NUMBER))
            {
                obj.type = OBJECT_NUMBER;
                obj.val.num = left.val.num + right.val.num;
            }
            else if (
                (left.type == OBJECT_STRING) && (right.type == OBJECT_STRING))
            {
                size_t str_len = strlen(left.val.str) + strlen(left.val.str);
                char* str = malloc(sizeof(char) * (str_len + 1));

                snprintf(str, str_len + 1, "%s%s", left.val.str, right.val.str);

                obj.type = OBJECT_STRING;
                obj.val.str = str;
            }
            else
            {
                err = "Operands must be two numbers or two strings";
                break;
            }
        }
        break;

        case TOKEN_GREATER:
        {
            if ((left.type != right.type) || (left.type != OBJECT_NUMBER))
            {
                err = "Operands must be numbers";
                break;
            }

            obj.type = OBJECT_BOOL;
            obj.val.boolean = left.val.num > right.val.num;
        }
        break;

        case TOKEN_LESS:
        {
            if ((left.type != right.type) || (left.type != OBJECT_NUMBER))
            {
                err = "Operands must be numbers";
                break;
            }

            obj.type = OBJECT_BOOL;
            obj.val.boolean = left.val.num < right.val.num;
        }
        break;

        case TOKEN_GREATER_EQUAL:
        {
            if ((left.type != right.type) || (left.type != OBJECT_NUMBER))
            {
                err = "Operands must be numbers";
                break;
            }

            obj.type = OBJECT_BOOL;
            obj.val.boolean = left.val.num >= right.val.num;
        }
        break;

        case TOKEN_LESS_EQUAL:
        {
            if ((left.type != right.type) || (left.type != OBJECT_NUMBER))
            {
                err = "Operands must be numbers";
                break;
            }

            obj.type = OBJECT_BOOL;
            obj.val.boolean = left.val.num <= right.val.num;
        }
        break;

        case TOKEN_BANG_EQUAL:
        {
            obj.type = OBJECT_BOOL;
            obj.val.boolean = !object_is_equal(&left, &right);
        }
        break;

        case TOKEN_EQUAL_EQUAL:
        {
            obj.type = OBJECT_BOOL;
            obj.val.boolean = object_is_equal(&left, &right);
        }
        break;
        }

        if (e->left->type != EXPR_VAR)
        {
            object_free(&left);
        }
        if (e->right->type != EXPR_VAR)
        {
            object_free(&right);
        }
    }
    break;

    case EXPR_VAR:
    {
        expr_var_t* e = (expr_var_t*)expr;
        obj = environment_get(&env, e->name.lexeme);
    }
    break;

    default:
    {
        assert(false);
    }
    break;
    }

    if (out_err)
    {
        *out_err = err;
    }
    return obj;
}