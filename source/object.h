/*
 * File: object.h
 * Created on Wed Mar 25 2026
 *
 * Copyright (c) 2026 Omar Eltayeb
 */

#ifndef _OBJECT_H
#define _OBJECT_H

#include "list.h"

#include <stdbool.h>

typedef enum
{
    _OBJECT_INVALID, // Don't ever make this not 0, a lot of zeroing out initialization depends on it
    OBJECT_BOOL,
    OBJECT_STRING,
    OBJECT_NUMBER,
    OBJECT_NIL,
    OBJECT_CALLABLE
} object_type_t;

typedef enum
{
    _CALLABLE_INVALID,
    CALLABLE_USER_FUN,
    CALLABLE_NATIVE_FUN
} callable_type_t;

typedef struct stmt_function_t stmt_function_t;

// This is for just functions for now (we don't have classes and methods)
typedef struct
{
    int arity;
    object_t* (*call)(list_object_t* args);
} callable_data_t;

typedef struct object_t
{
    object_type_t type;
    union
    {
        float num;
        char* str;
        bool boolean;
        callable_data_t call;
    } val;
    bool marked; // for the GC
} object_t;

void object_free(object_t* obj);
void object_print(const object_t* obj);

// creates a seperate deep copy of an object
object_t object_copy(const object_t* obj);

bool object_is_truthy(const object_t* obj);
bool object_is_equal(const object_t* left, const object_t* right);

#endif