/*
 * File: interpreter.h
 * Created on Tue Mar 24 2026
 *
 * Copyright (c) 2026 Omar Eltayeb
 */

#ifndef _INTERPRETER_H
#define _INTERPRETER_H

#include "object.h"
#include "parser.h"

void init_interpreter();
void destroy_interpreter();

void interpret(const stmt_list_t* stmts);

#endif