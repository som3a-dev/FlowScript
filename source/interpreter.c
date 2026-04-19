/*
 * File: interpreter.c
 * Created on Tue Mar 24 2026
 *
 * Copyright (c) 2026 Omar Eltayeb
 */

#include "interpreter.h"
#include "environment.h"
#include "gc.h"
#include "std.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void interpret_stmt(const stmt_t* stmt, const char** out_err);
static object_t* interpret_expr(const expr_t* expr, const char** out_err);
static object_t* interpret_function_call(object_t* callee, list_object_t* args, const char** out_err);

static environment_t env = { 0 };
static environment_t globals = { 0 };

void init_interpreter()
{
    environment_destroy(&env);
    environment_destroy(&globals);

    environment_init(&globals, NULL);
    {
        callable_data_t func = { 0 };
        func.type = CALLABLE_NATIVE_FUN;
        func.arity = 0;
        func.call = fsstd_clock;

        object_t* obj = gc_new_object();
        obj->type = OBJECT_CALLABLE;
        obj->val.call = func;

        environment_define(&globals, "clock", obj);
    }

    env = globals;
}

void destroy_interpreter()
{
    gc_free_objects();
    environment_destroy(&env);
}

void interpret(const list_stmt_t* stmts)
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

        gc_run(&env);
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
        object_t* obj = interpret_expr(s->expr, &err);
        object_print(obj);
    }
    break;

    case STMT_EXPR:
    {
        const stmt_expr_t* s = (stmt_expr_t*)stmt;
        interpret_expr(s->expr, &err);
    }
    break;

    case STMT_VAR:
    {
        const stmt_var_t* decl = (stmt_var_t*)stmt;
        object_t* val = NULL;
        if (decl->expr)
        {
            val = interpret_expr(decl->expr, &err);
            if (err)
            {
                // TODO(omar): why does an invalid object not give us an error
                return;
            }
        }

        if (!val)
        {
            val = gc_new_object();
            val->type = OBJECT_NIL;
        }

        if (!environment_define(&env, decl->name.lexeme, val))
        {
            err = "Variable with this name already exists.";
        }
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

        // TODO(omar): remove this when we have a GC
        environment_destroy(&env);
        env = prev_env;
    }
    break;

    case STMT_IF:
    {
        const stmt_if_t* s = (stmt_if_t*)stmt;
        object_t* condition = interpret_expr(s->condition, &err);
        if (err)
        {
            *out_err = err;
            return;
        }

        if (object_is_truthy(condition))
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

    case STMT_WHILE:
    {
        stmt_while_t* s = (stmt_while_t*)stmt;

        object_t* condition = NULL;
        do
        {
            condition = interpret_expr(s->condition, &err);
            if (err)
            {
                *out_err = err;
                return;
            }

            interpret_stmt(s->body, &err);
            if (err)
            {
                *out_err = err;
                return;
            }
        } while (object_is_truthy(condition));
    }
    break;

    case STMT_FUNCTION:
    {
        stmt_function_t* s = (stmt_function_t*)stmt;

        object_t* fun = gc_new_object();
        fun->type = OBJECT_CALLABLE;

        callable_data_t* callable = &(fun->val.call);
        callable->declaration = s;
        callable->arity = s->params.len;
        callable->call = fsstd_call_user_fun;

        environment_define(&globals, s->name.lexeme, fun);
    }
    break;
    }

    if (err && out_err)
    {
        *out_err = err;
    }
}

static object_t* interpret_expr(const expr_t* expr, const char** out_err)
{

    if (!expr)
    {
        if (out_err)
        {
            *out_err = "Invalid Expression";
        }

        return NULL;
    }

    const char* err = NULL;
    object_t* obj = NULL;

    switch (expr->type)
    {

    case EXPR_CALL:
    {
        expr_call_t* call = (expr_call_t*)expr;
        object_t* callee = interpret_expr(call->callee, &err);
        if (err)
        {
            *out_err = err;
            break;
        }

        list_object_t args = { 0 };
        for (int i = 0; i < call->args.len; i++)
        {
            expr_t* arg = call->args.exprs[i];
            object_t* val = interpret_expr(arg, &err);
            if (err)
            {
                *out_err = err;
                list_object_free(&args);
                goto exit_expr_call;
            }

            list_object_push(&args, val);
        }

        list_object_print(&args);

        obj = interpret_function_call(callee, &args, &err);

        list_object_free(&args);
    }
    exit_expr_call:
        break;

    case EXPR_ASSIGN:
    {
        expr_assign_t* e = (expr_assign_t*)expr;

        object_t* val = interpret_expr(e->val, &err);
        if (err)
        {
            *out_err = err;
            break;
        }

        environment_assign(&env, e->name.lexeme, val);
        obj = val;
    }
    break;

    case EXPR_LOGICAL:
    {
        expr_logical_t* e = (expr_logical_t*)expr;
        obj = gc_new_object();
        obj->type = OBJECT_BOOL;

        object_t* left = interpret_expr(e->left, &err);
        if (err)
        {
            *out_err = err;
            break;
        }

        /*        if (e->operator->type == TOKEN_OR)
                {
                    if (object_is_truthy(&left))
                    {
                        obj->val.boolean = true;
                    }
                }
                else if (e->operator->type == TOKEN_AND)
                {
                    if (!object_is_truthy(&left))
                    {
                        obj->val.boolean = false;
                    }
                } */

        object_t* right = interpret_expr(e->right, &err);
        if (err)
        {
            *out_err = err;
            break;
        }

        if (e->operator.type == TOKEN_OR)
        {
            if (object_is_truthy(left) || object_is_truthy(right))
            {
                obj->val.boolean = true;
            }
        }
        else if (e->operator.type == TOKEN_AND)
        {
            if (object_is_truthy(left) && object_is_truthy(right))
            {
                obj->val.boolean = true;
            }
        }
    }
    break;

    case EXPR_LITERAL:
    {
        expr_literal_t* e = (expr_literal_t*)expr;
        obj = gc_new_object();
        obj->type = e->type;

        switch (obj->type)
        {
        case OBJECT_STRING:
        {
            obj->val.str = malloc(sizeof(char) * (strlen(e->val.str) + 1));
            strcpy(obj->val.str, e->val.str);
        }
        break;

        case OBJECT_BOOL:
        {
            obj->val.boolean = e->val.boolean;
        }
        break;

        case OBJECT_NUMBER:
        {
            obj->val.num = e->val.num;
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
        obj = gc_new_object();
        object_t* right = interpret_expr(e->right, &err);
        if (err)
        {
            break;
        }

        switch (e->operator.type)
        {
        case TOKEN_MINUS:
        {
            if (right->type == OBJECT_NUMBER)
            {
                obj->type = OBJECT_NUMBER;
                obj->val.num = -(right->val.num);
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
            obj->type = OBJECT_BOOL;
            obj->val.boolean = !(object_is_truthy(right));
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
        obj = gc_new_object();
        object_t* left = interpret_expr(e->left, NULL);
        object_t* right = interpret_expr(e->right, NULL);
        if (err)
        {
            break;
        }

        switch (e->operator.type)
        {
        case TOKEN_STAR:
        {
            if ((left->type != right->type) || (left->type != OBJECT_NUMBER))
            {
                err = "Operands must be numbers";
                break;
            }

            obj->type = OBJECT_NUMBER;
            obj->val.num = left->val.num * right->val.num;
        }
        break;

        case TOKEN_MINUS:
        {
            if ((left->type != right->type) || (left->type != OBJECT_NUMBER))
            {
                err = "Operands must be numbers";
                break;
            }

            obj->type = OBJECT_NUMBER;
            obj->val.num = left->val.num - right->val.num;
        }
        break;

        case TOKEN_SLASH:
        {
            if ((left->type != right->type) || (left->type != OBJECT_NUMBER))
            {
                err = "Operands must be numbers";
                break;
            }

            obj->type = OBJECT_NUMBER;
            obj->val.num = left->val.num / right->val.num;
        }
        break;

        case TOKEN_PLUS:
        {
            if ((left->type == OBJECT_NUMBER) && (right->type == OBJECT_NUMBER))
            {
                obj->type = OBJECT_NUMBER;
                obj->val.num = left->val.num + right->val.num;
            }
            else if (
                (left->type == OBJECT_STRING) && (right->type == OBJECT_STRING))
            {
                size_t str_len = strlen(left->val.str) + strlen(left->val.str);
                char* str = malloc(sizeof(char) * (str_len + 1));

                snprintf(str, str_len + 1, "%s%s", left->val.str, right->val.str);

                obj->type = OBJECT_STRING;
                obj->val.str = str;
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
            if ((left->type != right->type) || (left->type != OBJECT_NUMBER))
            {
                err = "Operands must be numbers";
                break;
            }

            obj->type = OBJECT_BOOL;
            obj->val.boolean = left->val.num > right->val.num;
        }
        break;

        case TOKEN_LESS:
        {
            if ((left->type != right->type) || (left->type != OBJECT_NUMBER))
            {
                err = "Operands must be numbers";
                break;
            }

            obj->type = OBJECT_BOOL;
            obj->val.boolean = left->val.num < right->val.num;
        }
        break;

        case TOKEN_GREATER_EQUAL:
        {
            if ((left->type != right->type) || (left->type != OBJECT_NUMBER))
            {
                err = "Operands must be numbers";
                break;
            }

            obj->type = OBJECT_BOOL;
            obj->val.boolean = left->val.num >= right->val.num;
        }
        break;

        case TOKEN_LESS_EQUAL:
        {
            if ((left->type != right->type) || (left->type != OBJECT_NUMBER))
            {
                err = "Operands must be numbers";
                break;
            }

            obj->type = OBJECT_BOOL;
            obj->val.boolean = left->val.num <= right->val.num;
        }
        break;

        case TOKEN_BANG_EQUAL:
        {
            obj->type = OBJECT_BOOL;
            obj->val.boolean = !object_is_equal(left, right);
        }
        break;

        case TOKEN_EQUAL_EQUAL:
        {
            obj->type = OBJECT_BOOL;
            obj->val.boolean = object_is_equal(left, right);
        }
        break;
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

    if (err && out_err)
    {
        *out_err = err;

        // OBJECT CREATE SHOULD FREE ?
        //        object_free(obj);
        return NULL;
    }

    return obj;
}

static object_t* interpret_function_call(object_t* callee, list_object_t* args, const char** out_err)
{
    assert(callee);
    assert(args);
    assert(out_err);

    if (callee->type != OBJECT_CALLABLE)
    {
        *out_err = "RUNTIME ERROR: Only functions and classes can be called.";
        return NULL;
    }

    const callable_data_t* callable = (const callable_data_t*)(&(callee->val));
    if (callable->arity != args->len)
    {
        *out_err = "RUNTIME ERROR: Incorrect number of arguments provided.";
        return NULL;
    }

    assert(callable->call);
    return callable->call(args);
}