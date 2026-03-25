#ifndef _OBJECT_H
#define _OBJECT_H

#include <stdbool.h>

typedef enum
{
    _OBJECT_INVALID,
    OBJECT_BOOL,
    OBJECT_STRING,
    OBJECT_NUMBER,
    OBJECT_NIL
} object_type_t;

typedef struct
{
    object_type_t type;
    union {
        float num;
        char* str;
        bool boolean;
    } val;
} object_t;

void free_object(object_t* obj);
void print_object(const object_t *obj);


#endif