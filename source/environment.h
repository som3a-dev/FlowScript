/*
 * File: environment.h
 * Created on Sat Apr 04 2026
 *
 * Copyright (c) 2026 Omar Eltayeb
 */

#ifndef _ENVIRONMENT_H
#define _ENVIRONMENT_H

#include "object.h"

typedef struct
{
    char* name;
    object_t* val;
} environment_entry_t;

typedef struct _environment_t
{
    struct _environment_t* enclosing;
    environment_entry_t* vals;
    int vals_count;
} environment_t;

void environment_init(environment_t* env, environment_t* enclosing);
void environment_destroy(environment_t* env);

object_t* environment_get(const environment_t* env, const char* name);
bool environment_define(environment_t* env, const char* name, object_t* val);
void environment_assign(environment_t* env, const char* name, object_t* val);

#endif