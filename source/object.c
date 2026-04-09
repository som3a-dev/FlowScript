/*
 * File: object.c
 * Created on Wed Mar 25 2026
 *
 * Copyright (c) 2026 Omar Eltayeb
 */

#include "object.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void object_free(object_t* obj)
{
    if (obj->type == OBJECT_STRING)
    {
        free(obj->val.str);
    }
}

void object_print(const object_t* obj)
{
    switch (obj->type)
    {
    case OBJECT_BOOL:
    {
        if (obj->val.boolean)
        {
            printf("true\n");
        }
        else
        {
            printf("false\n");
        }
    }
    break;

    case OBJECT_NUMBER:
    {
        printf("%f\n", obj->val.num);
    }
    break;

    case OBJECT_STRING:
    {
        printf("%s\n", obj->val.str);
    }
    break;

    case OBJECT_NIL:
    {
        printf("nil\n");
    }
    break;

    case _OBJECT_INVALID:
    {
        printf("(INVALID OBJECT)\n");
    }
    break;

    default:
        assert(false);
    }
}

object_t object_copy(const object_t* obj)
{
    object_t cpy = *obj;
    if (cpy.type == OBJECT_STRING)
    {
        cpy.val.str = malloc(sizeof(char) * (strlen(obj->val.str) + 1));
        strcpy(cpy.val.str, obj->val.str);
    }

    return cpy;
}

bool object_is_truthy(const object_t* obj)
{
    switch (obj->type)
    {
    case OBJECT_BOOL:
    {
        return (obj->val.boolean);
    }
    break;

    case OBJECT_NIL:
    {
        return false;
    }
    break;

    default:
    {
        return true;
    }
    break;
    }
}

bool object_is_equal(const object_t* left, const object_t* right)
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
    }
    break;

    case OBJECT_NIL:
    {
        return true;
    }
    break;

    // this will work for any object type where the value is encoded in val
    default:
    {
        return left->val.num == right->val.num;
    }
    break;
    }
}