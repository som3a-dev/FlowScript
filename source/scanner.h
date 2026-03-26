/*
 * File: scanner.h
 * Created on Mon Mar 09 2026
 *
 * This module defines and implements the token scanner/lexer
 * 
 * Copyright (c) 2026 Omar Eltayeb
 */

#ifndef _SCANNER_H
#define _SCANNER_H

#include <stdbool.h>

typedef enum
{
	TOKEN_NONE,

	// Single character
	TOKEN_LEFT_PAREN, TOKEN_RIGHT_PAREN, TOKEN_LEFT_BRACE, TOKEN_RIGHT_BRACE,
	TOKEN_COMMA, TOKEN_DOT,
	TOKEN_MINUS, TOKEN_PLUS, TOKEN_STAR,
	TOKEN_SEMICOLON, TOKEN_SLASH,

	// Single or double character
	TOKEN_BANG, TOKEN_BANG_EQUAL,
	TOKEN_EQUAL, TOKEN_EQUAL_EQUAL,
	TOKEN_GREATER, TOKEN_GREATER_EQUAL,
	TOKEN_LESS, TOKEN_LESS_EQUAL,

	// Literals
	TOKEN_IDENTIFIER, TOKEN_STRING, TOKEN_NUMBER,

	// Keywords
	TOKEN_AND, TOKEN_CLASS, TOKEN_ELSE, TOKEN_FALSE,
	TOKEN_FUN, TOKEN_FOR, TOKEN_IF,
	TOKEN_NIL, TOKEN_OR, TOKEN_PRINT,
	TOKEN_RETURN, TOKEN_SUPER, TOKEN_THIS,
	TOKEN_TRUE, TOKEN_VAR, TOKEN_WHILE,

	TOKEN_EOF
} token_type_t;

typedef struct
{
	token_type_t type;
	char* lexeme;
	int line;
} token_t;

typedef struct
{
	token_t* tokens;
	int len;
	bool has_error;
} token_list_t;

token_list_t scan(const char* input);

void token_print(token_t token);
void token_list_delete(token_list_t* list);

#endif