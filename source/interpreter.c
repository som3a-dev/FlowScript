#include "interpreter.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

static object_t interpret_expr(const expr_t* expr, const char** out_err);
static bool object_is_truthy(const object_t* obj);
static bool object_is_equal(const object_t* left, const object_t* right);

void interpret(const expr_t* expr)
{
    const char* err = NULL;
    object_t obj = interpret_expr(expr, &err);
    if (err) {
        printf("RUNTIME ERROR: %s\n", err);
    }
    else {
        print_object(&obj);
    }

    free_object(&obj);
}

static object_t interpret_expr(const expr_t* expr, const char** out_err)
{
    object_t obj = {0};

    if (!expr) {
        if (out_err) {
            *out_err = "Invalid Expression";
        }

        return obj;
    }

    const char* err = NULL;

    switch (expr->type)
    {
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
                } break;

                case OBJECT_BOOL:
                {
                    obj.val.boolean = e->val.boolean;
                } break;

                case OBJECT_NUMBER:
                {
                    obj.val.num = e->val.num;
                } break;

                case OBJECT_NIL:
                {
                    // TODO(omar): implement nil (everywhere)
                } break;

                default: assert(false);
            }

        } break;

        case EXPR_UNARY:
        {
            expr_unary_t* e = (expr_unary_t*)expr;
            object_t right = interpret_expr(e->right, &err);
            if (err) {
                break;
            }

            switch (e->operator.type)
            {
                case TOKEN_MINUS:
                {
                    if (right.type == OBJECT_NUMBER) {
                        obj.type = OBJECT_NUMBER;
                        obj.val.num = -(right.val.num);
                    }
                    else {
                        err = "Operand must be a number";
                        break;
                    }
                } break;

                case TOKEN_BANG:
                {
                    obj.type = OBJECT_BOOL;
                    obj.val.boolean = !(object_is_truthy(&right));
                } break;

                default: assert(false);
            }
        } break;

        case EXPR_BINARY:
        {
            expr_binary_t* e = (expr_binary_t*)expr;
            object_t left = interpret_expr(e->left, NULL);
            object_t right = interpret_expr(e->right, NULL);
            if (err) {
                break;
            }

            switch (e->operator.type)
            {
                case TOKEN_STAR:
                {
                    if ((left.type != right.type) || (left.type != OBJECT_NUMBER)) {
                        err = "Operands must be numbers";
                        break;
                    }

                    obj.type = OBJECT_NUMBER;
                    obj.val.num = left.val.num * right.val.num;
                }  break;

                case TOKEN_MINUS:
                {
                    if ((left.type != right.type) || (left.type != OBJECT_NUMBER)) {
                        err = "Operands must be numbers";
                        break;
                    }

                    obj.type = OBJECT_NUMBER;
                    obj.val.num = left.val.num - right.val.num;
                } break;

                case TOKEN_SLASH:
                {
                    if ((left.type != right.type) || (left.type != OBJECT_NUMBER)) {
                        err = "Operands must be numbers";
                        break;
                    }

                    obj.type = OBJECT_NUMBER;
                    obj.val.num = left.val.num / right.val.num;
                } break;

                case TOKEN_PLUS:
                {
                    if ((left.type == OBJECT_NUMBER) && (right.type == OBJECT_NUMBER)) {
                        obj.type = OBJECT_NUMBER;
                        obj.val.num = left.val.num + right.val.num;
                    }
                    else if ((left.type == OBJECT_STRING) && (right.type == OBJECT_STRING)) {
                        size_t str_len = strlen(left.val.str) + strlen(left.val.str);
                        char* str = malloc(sizeof(char) * (str_len + 1));

                        snprintf(str, str_len + 1, "%s%s", left.val.str, right.val.str);

                        obj.type = OBJECT_STRING;
                        obj.val.str = str;
                    }
                    else {
                        err = "Operands must be two numbers or two strings";
                        break;
                    }
                } break;

                case TOKEN_GREATER:
                {
                    if ((left.type != right.type) || (left.type != OBJECT_NUMBER)) {
                        err = "Operands must be numbers";
                        break;
                    }

                    obj.type = OBJECT_BOOL;
                    obj.val.boolean = left.val.num > right.val.num;
                } break;

                case TOKEN_LESS:
                {
                    if ((left.type != right.type) || (left.type != OBJECT_NUMBER)) {
                        err = "Operands must be numbers";
                        break;
                    }

                    obj.type = OBJECT_BOOL;
                    obj.val.boolean = left.val.num < right.val.num;
                } break;

                case TOKEN_GREATER_EQUAL:
                {
                    if ((left.type != right.type) || (left.type != OBJECT_NUMBER)) {
                        err = "Operands must be numbers";
                        break;
                    }

                    obj.type = OBJECT_BOOL;
                    obj.val.boolean = left.val.num >= right.val.num;
                } break;

                case TOKEN_LESS_EQUAL:
                {
                    if ((left.type != right.type) || (left.type != OBJECT_NUMBER)) {
                        err = "Operands must be numbers";
                        break;
                    }

                    obj.type = OBJECT_BOOL;
                    obj.val.boolean = left.val.num <= right.val.num;
                } break;

                case TOKEN_BANG_EQUAL:
                {
                    obj.type = OBJECT_BOOL;
                    obj.val.boolean = !object_is_equal(&left, &right) ;
                } break;

                case TOKEN_EQUAL_EQUAL:
                {
                    obj.type = OBJECT_BOOL;
                    obj.val.boolean = object_is_equal(&left, &right) ;
                } break;
            }

            free_object(&left);
            free_object(&right);
        } break;

        default: assert(false);
    }

    if (out_err)
    {
        *out_err = err;
    }
    return obj;
}

static bool object_is_truthy(const object_t* obj)
{
    switch (obj->type)
    {
        case OBJECT_BOOL:
        {
            return (obj->val.boolean);
        } break;

        case OBJECT_NIL:
        {
            return false;
        } break;

        default:
        {
            return true;
        } break;
    }
}

static bool object_is_equal(const object_t* left, const object_t* right)
{
    if (left->type != right->type)
    {
        return false;
    }

    switch (left->type)
    {
        case OBJECT_STRING:
        {
            return (strcmp(left->val.str, right->val.str) == 0);
        } break;

        case OBJECT_NIL:
        {
            return true;
        } break;

        // this will work for any object type where the value is encoded in val
        default:
        {
            return left->val.num == right->val.num;
        } break;
    }
}
