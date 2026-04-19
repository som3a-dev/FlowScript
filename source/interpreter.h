/*
 * File: interpreter.h
 * Created on Tue Mar 24 2026
 *
 * Copyright (c) 2026 Omar Eltayeb
 */

#ifndef _INTERPRETER_H
#define _INTERPRETER_H

#include "environment.h"
#include "object.h"
#include "parser.h"

typedef struct interpreter_state_t
{
    environment_t env;
    environment_t globals;
} interpreter_state_t;

void init_interpreter();
void destroy_interpreter();

void interpret(const list_stmt_t* stmts);

#endif