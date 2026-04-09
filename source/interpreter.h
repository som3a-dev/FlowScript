#ifndef _INTERPRETER_H
#define _INTERPRETER_H

#include "parser.h"
#include "object.h"

void init_interpreter();
void destroy_interpreter();

void interpret(const declaration_list_t* stmts);

#endif