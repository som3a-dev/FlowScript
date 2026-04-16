/*
 * File: gc.c
 * Created on Thu Apr 16 2026
 *
 * Copyright (c) 2026 Omar Eltayeb
 */

#include "gc.h"

#include <stdio.h>
#include <stdlib.h>

static gc_object_t* gc_obj_list = NULL;

static void gc_reset_object_marks();
static void gc_sweep();
static void gc_mark_objects(environment_t* e);

object_t* gc_new_object()
{
    object_t* obj = calloc(1, sizeof(object_t));

    // Add to GC list of objects
    if (gc_obj_list)
    {
        gc_object_t* prev_gc_obj = NULL;
        gc_object_t* gc_obj = gc_obj_list;
        while (true)
        {
            if (!gc_obj)
            {
                break;
            }

            if (gc_obj->obj == NULL)
            {
                // Found empty spot, return early
                gc_obj->obj = obj;
                return obj;
            }

            prev_gc_obj = gc_obj;
            gc_obj = gc_obj->next;
        }

        // Didn't find an empty spot, push to the end
        gc_object_t* new_gc_obj = calloc(1, sizeof(gc_object_t));
        new_gc_obj->obj = obj;
        new_gc_obj->next = NULL;
        prev_gc_obj->next = new_gc_obj;
    }
    else
    {
        gc_object_t* new_gc_obj = calloc(1, sizeof(gc_object_t));
        new_gc_obj->obj = obj;
        new_gc_obj->next = NULL;
        gc_obj_list = new_gc_obj;
    }

    return obj;
}

void gc_run(environment_t* env)
{
    gc_reset_object_marks();
    gc_mark_objects(env);
    gc_sweep();
}

void gc_free_objects()
{
    gc_object_t* gc_obj = gc_obj_list;
    while (gc_obj)
    {
        gc_object_t* next = gc_obj->next;

        object_free(gc_obj->obj);
        free(gc_obj->obj);
        free(gc_obj);
        gc_obj = next;
    }
}

void gc_mark_objects(environment_t* e)
{
    if (!e)
    {
        return;
    }

    for (int i = 0; i < e->vals_count; i++)
    {
        environment_entry_t* entry = e->vals + i;
        if (entry->val)
        {
            entry->val->marked = true;
        }
    }

    gc_mark_objects(e->enclosing);
}

void gc_reset_object_marks()
{
    gc_object_t* gc_obj = gc_obj_list;
    while (gc_obj)
    {
        if (gc_obj->obj)
        {
            gc_obj->obj->marked = false;
        }
        gc_obj = gc_obj->next;
    }
}

void gc_sweep()
{
    gc_object_t* gc_obj = gc_obj_list;
    while (gc_obj)
    {
        if (gc_obj->obj)
        {
            if (gc_obj->obj->marked == false)
            {
                object_free(gc_obj->obj);
                free(gc_obj->obj);
                gc_obj->obj = NULL;
            }
        }

        gc_obj = gc_obj->next;
    }
}
