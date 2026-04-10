/*
 * File: parser.h
 * Created on Sun Mar 22 2026
 *
 * Copyright (c) 2026 Omar Eltayeb
 */

#ifndef _PARSER_H
#define _PARSER_H

#include "expression.h"
#include "statement.h"

stmt_list_t parse(token_list_t* _tokens, bool* has_error);

#endif