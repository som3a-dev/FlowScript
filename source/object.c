#include "object.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

void free_object(object_t* obj)
{
    if (obj->type == OBJECT_STRING) {
        free(obj->val.str);
    }
}

void print_object(const object_t *obj)
{
    switch (obj->type)
    {
        case OBJECT_BOOL:
        {
            if (obj->val.boolean) {
                printf("true\n");
            }
            else {
                printf("false\n");
            }
        } break;

        case OBJECT_NUMBER:
        {
            printf("%f\n", obj->val.num);
        } break;

        case OBJECT_STRING:
        {
            printf("%s\n", obj->val.str);
        } break;

        case OBJECT_NIL:
        {
            printf("nil\n");
        } break;

        default: assert(false);
    }
}
