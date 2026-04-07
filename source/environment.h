#ifndef _ENVIRONMENT_H
#define _ENVIRONMENT_H

#include "object.h"

typedef struct
{
    char* name;
    object_t val;
} environment_entry_t;

/*
* entry names strings lifetimes are owned by the environment
* object_t strings lifetimes are not, they are not owned by the environment and will be freed somewhere else (the expression tree)
* TODO(omar): decide if the environment should make copies of objects instead
*/
typedef struct
{
    environment_entry_t* vals;
    int vals_count;
} environment_t;

void environment_init(environment_t* env);
void environment_destroy(environment_t* env);

object_t environment_get(const environment_t* env, const char* name);
void environment_define(environment_t* env, const char* name, object_t val);

#endif