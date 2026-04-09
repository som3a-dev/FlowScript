/*
 * File: object.h
 * Created on Wed Mar 25 2026
 *
 * Copyright (c) 2026 Omar Eltayeb
 */

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
    union
    {
        float num;
        char* str;
        bool boolean;
    } val;
} object_t;

void object_free(object_t* obj);
void object_print(const object_t* obj);

// creates a seperate deep copy of an object
object_t object_copy(const object_t* obj);

#endif