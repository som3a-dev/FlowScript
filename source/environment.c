#include "environment.h"

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define ENVIRONMENT_DEFAULT_SIZE 10

static void environment_resize(environment_t* env, int new_size);

static uint32_t hash(const char* str);

void environment_init(environment_t* env)
{
    env->vals = NULL;
    env->vals_count = 0;

    environment_resize(env, ENVIRONMENT_DEFAULT_SIZE);
}

void environment_destroy(environment_t* env)
{
    if (env->vals)
    {
        for (int i = 0; i < env->vals_count; i++)
        {
            free(env->vals[i].name);
        }

        free(env->vals);
        env->vals = NULL;
    }

    env->vals_count = 0;
}

object_t environment_get(const environment_t* env, const char* name)
{
    uint32_t index = hash(name) % env->vals_count;

    const environment_entry_t* e = env->vals + index;
    if (strcmp(e->name, name) == 0)
    {
        return e->val;
    }
    else
    {
        // LINEAR PROBING
    }

    object_t obj = {0};
    obj.type = _OBJECT_INVALID;
    return obj;
}

void environment_define(environment_t* env, const char* name, object_t val)
{
    uint32_t index = hash(name) % env->vals_count;

    environment_entry_t* e = env->vals + index;

    if (e->name)
    {
        // LINEAR PROBING
        return;
    }

    e->name = malloc(sizeof(char) * (strlen(name) + 1));
    strcpy(e->name, name);
    e->val = val;
}

static void environment_resize(environment_t* env, int new_size)
{
    if (new_size < 1)
    {
        return;
    }

    if (env->vals) 
    {
        free(env->vals);
        env->vals_count = 0;
    }

    env->vals_count = new_size;
    env->vals = calloc(new_size, sizeof(*(env->vals)));

    for (int i = 0; i < env->vals_count; i++)
    {
        env->vals[i].val.type = _OBJECT_INVALID;
    }
}

static uint32_t hash(const char* str)
{
    uint32_t hash = 2166136261u;
    for (int i = 0; i < strlen(str); i++)
    { 
        hash = (hash ^ str[i]) * 16777619u;
    }

    return hash;
}