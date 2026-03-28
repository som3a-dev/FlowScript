#ifndef _INTERPRETER_H
#define _INTERPRETER_H

#include "parser.h"
#include "object.h"

void interpret(const declaration_list_t* stmts);

#endif