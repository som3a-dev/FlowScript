#ifndef _INTERPRETER_H
#define _INTERPRETER_H

#include "parser.h"
#include "object.h"

void interpret(const stmt_list_t* stmts);

#endif