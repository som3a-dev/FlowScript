#include "object.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void free_object(object_t* obj)
{
    if (obj->type == OBJECT_STRING)
    {
        free(obj->val.str);
    }
}

void print_object(const object_t* obj)
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
