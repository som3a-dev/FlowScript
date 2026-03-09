/*
 * File: scanner.h
 * Created on Mon Mar 09 2026
 *
 * This module defines and implements the token scanner/lexer
 * 
 * Copyright (c) 2026 Omar Eltayeb
 */

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

	TOKEN_EOF
} token_type_t;

typedef struct
{
	token_type_t type;
	char* lexem;
	int line;
} token_t;

typedef struct
{
	token_t* tokens;
	int len;
} token_list_t;

token_list_t token_scanner_scan(const char* input);

void token_print(token_t token);
void token_list_delete(token_list_t* list);