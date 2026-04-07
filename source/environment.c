#include "environment.h"

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define ENVIRONMENT_DEFAULT_SIZE 20

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
    if (e->name == NULL)
    {
        goto invalid_object_ret;
    }

    if (strcmp(e->name, name) == 0)
    {
        return e->val;
    }
    else
    {
        for (int i = index + 1; i < env->vals_count; i++)
        {
            e++;
            if (strcmp(e->name, name) == 0)
            {
                return e->val;
            }
        }
    }

    invalid_object_ret:
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
        for (int i = index + 1; i < env->vals_count; i++)
        {
            e++;
            if (e->name == NULL)
            {
                goto define_entry;
            }
        }

        // No empty spot found, resize and retry
        environment_resize(env, env->vals_count * 2) ;
        environment_define(env, name, val);
        return;
    }

    define_entry:
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
    environment_t old_env = *env;

    env->vals_count = new_size;
    env->vals = calloc(new_size, sizeof(*(env->vals)));

    for (int i = 0; i < env->vals_count; i++)
    {
        env->vals[i].val.type = _OBJECT_INVALID;
    }

    if (old_env.vals)
    {
        // Move all the old entries
        environment_entry_t* e = old_env.vals;

        for (int i = 0; i < old_env.vals_count; i++)
        {
            if (e->name)
            {
                environment_define(env, e->name, e->val);
            }

            e++;
        }

        environment_destroy(&old_env);
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