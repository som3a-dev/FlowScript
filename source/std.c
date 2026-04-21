/*
 * File: std.c
 * Created on Thu Apr 16 2026
 *
 * Copyright (c) 2026 Omar Eltayeb
 */

#include "std.h"
#include "environment.h"
#include "gc.h"

#include <assert.h>
#include <stdio.h>
#include <time.h>

#ifdef _WIN32
#include <Windows.h>
#endif

#ifdef _WIN32
static bool freq_initialized = false;
static LARGE_INTEGER freq;
#endif

object_t* fsstd_clock(interpreter_state_t* interpreter, object_t* callee, list_object_t* args)
{
    (void)args;
    (void)interpreter;
    (void)callee;

    object_t* ret = gc_new_object();
    ret->type = OBJECT_NUMBER;

#ifdef _WIN32
    if (!freq_initialized)
    {
        QueryPerformanceFrequency(&freq);
    }

    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);

    ret->val.num = (float)(now.QuadPart) / freq.QuadPart;
#else
    struct timespec _t;
    clock_gettime(CLOCK_MONOTONIC, &_t);
    ret->val.num = _t.tv_sec * 1000 + lround(_t.tv_nsec / 1e6);
#endif

    return ret;
}

object_t* fsstd_call_user_fun(interpreter_state_t* interpreter, object_t* callee, list_object_t* args)
{
    (void)args;
    (void)interpreter;
    (void)callee;

    object_t* ret = gc_new_object();
    ret->type = OBJECT_NIL;

    stmt_function_t* declaration = callee->val.call.declaration;
    assert(declaration);

/*    environment_t env = { 0 };
    environment_init(&env, &(interpreter->globals));

    for (int i = 0; i < declaration->params.len; i++)
    {
        environment_define(&env, declaration->params.tokens[i].lexeme, args->objects[i]);
    }*/

    return ret;
}