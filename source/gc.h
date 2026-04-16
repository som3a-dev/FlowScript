/*
 * File: gc.h
 * Created on Thu Apr 16 2026
 *
 * the garbage collector, duh
 *
 * Copyright (c) 2026 Omar Eltayeb
 */

#include "environment.h"
#include "object.h"

// Linked list of objects the GC tracks, So all objects (hopefully)
typedef struct gc_object_t
{
    object_t* obj;
    struct gc_object_t* next;
} gc_object_t;

// yes, you create objects by asking the GC
object_t* gc_new_object();
void gc_free_objects();

void gc_run(environment_t* env);