/*
 * File: std.h
 * Created on Thu Apr 16 2026
 *
 * Copyright (c) 2026 Omar Eltayeb
 */

#include "list.h"
#include "object.h"

// fsstd stands for FlowScript standard

object_t* fsstd_clock(interpreter_state_t* interpreter, list_object_t* args);
object_t* fsstd_call_user_fun(interpreter_state_t* interpreter, list_object_t* args);
