/*
 * File: scanner.c
 * Created on Mon Mar 09 2026
 *
 * Copyright (c) 2026 Omar Eltayeb
 */

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "scanner.h"

static inline bool IS_DIGIT(char c) {
    return ('0' <= c) && (c <= '9');
}

static inline bool IS_IDENTIFIER_CHAR(char c) {
	return (
		('a' <= c && c <= 'z') ||
		('A' <= c && c <= 'Z') ||
		(c == '_')
	);
}

typedef struct
{
	char* str;
	size_t str_len; 

	token_t* tokens;
	int tokens_count;

	// index of the starting character of the token being scanned
	int start;
	// index of the current character of the token being scanned
	int current;
	int line;	

	bool has_error;
} token_scanner_t;

static void scan_token(token_scanner_t* scan);

// In the case of an identifier type token, 
// add_token checks the lexeme first, if it matches a reserved keyword
// the token will be added with said keyword's token type
// if not, the type will be an identifier
static void add_token(token_scanner_t* scan, token_type_t type);

static void add_double_token(
	token_scanner_t* scan, char expected_next,
	token_type_t single_type, token_type_t double_type);

static bool at_end(const token_scanner_t* scan);

static token_type_t get_type_from_keyword(const char* lexeme);

token_list_t token_scanner_scan(const char* input)
{
	token_scanner_t scan = {0};
	scan.line = 1;
	scan.str = malloc(sizeof(char) * (strlen(input) + 1));
	strcpy(scan.str, input);
	scan.str_len = strlen(scan.str);

	while (scan.current < scan.str_len)
	{
		scan_token(&scan);
	}

	token_list_t list;
	list.tokens = scan.tokens;
	list.len = scan.tokens_count;
	list.has_error = scan.has_error;

	free(scan.str);

	return list;
}

void token_print(token_t token)
{
    printf("Type: %d lexeme: '%s' Line: %d\n", token.type, token.lexeme, token.line);
}

void token_list_delete(token_list_t* list)
{
	for (int i = 0; i < list->len; i++)
	{
		free(list->tokens[i].lexeme);
	}

	free(list->tokens);
	list->tokens = NULL;
	list->len = 0;
}

static void scan_token(token_scanner_t* scan)
{
	char c = scan->str[scan->current];
	scan->start =scan->current;
	scan->current++;

	token_t token = {0};
    token.line = 1;
	switch (c)
	{
		case '(': {
			add_token(scan, TOKEN_LEFT_PAREN);
		} break;
		case ')': {
			add_token(scan, TOKEN_RIGHT_PAREN);
		} break;
		case '{': {
			add_token(scan, TOKEN_LEFT_BRACE);
		} break;
		case '}': {
			add_token(scan, TOKEN_RIGHT_BRACE);
		} break;

		case ',': {
			add_token(scan, TOKEN_COMMA);
		} break;
		case '.': {
			add_token(scan, TOKEN_DOT);
		} break;

		case '-': {
			add_token(scan, TOKEN_MINUS);
		} break;
		case '+': {
			add_token(scan, TOKEN_PLUS);
		} break;
		case '*': {
			add_token(scan, TOKEN_STAR);
		} break;

		case ';': {
			add_token(scan, TOKEN_SEMICOLON);
		} break;

		case '!': {
			add_double_token(scan, '=', TOKEN_BANG, TOKEN_BANG_EQUAL);
		} break;

		case '=': {
			add_double_token(scan, '=', TOKEN_EQUAL, TOKEN_EQUAL_EQUAL);
		} break;

		case '<': {
			add_double_token(scan, '=', TOKEN_LESS, TOKEN_LESS_EQUAL);
		} break;

		case '>': {
			add_double_token(scan, '=', TOKEN_GREATER, TOKEN_GREATER_EQUAL);
		} break;

		case '/': {
			if (scan->str[scan->current] == '/') {
				while ((scan->str[scan->current] != '\n') && (!at_end(scan)))
				{
					scan->current++;
				}
			}
			else {
				add_token(scan, TOKEN_SLASH);
			}
		} break;

		case ' ':
		case '\r':
		case '\t': {
		} break;

		case '\n': {
			scan->line++;
		} break;

		case '"': {
			scan->start = scan->current; // start from the character after the first '"'
			while (scan->str[scan->current] != '"' && (!at_end(scan)))
			{
				scan->current++;
			}

			if (!at_end(scan)) {
				add_token(scan, TOKEN_STRING);
				scan->current++; // skip the last '"'
			}
			else {
				printf("ERROR | Line: %d | Unterminated string literal\n", scan->line);
				scan->has_error = true;
			}
		} break;

		default: {
			if (IS_DIGIT(c)) {
				while (IS_DIGIT(scan->str[scan->current]))
				{
					scan->current++;
				}
				if (scan->str[scan->current] == '.' && (IS_DIGIT(scan->str[scan->current + 1]))) {
					scan->current++;
					while (IS_DIGIT(scan->str[scan->current]))
					{
						scan->current++;
					}				
				}

				add_token(scan, TOKEN_NUMBER);
			}
			else if (IS_IDENTIFIER_CHAR(c)) {
				while (IS_IDENTIFIER_CHAR(scan->str[scan->current])) {
					scan->current++;
				}

				add_token(scan, TOKEN_IDENTIFIER);
			}
			else {
				printf("ERROR | Line: %d | Unexpected Character '%c'\n", scan->line, c);
				scan->has_error = true;
			}
		} break;
	}
}

static void add_token(token_scanner_t* scan, token_type_t type)
{
	token_t token = {0};
	token.type = type;
	token.line = 1;

	size_t lexeme_len = scan->current - scan->start;
	token.lexeme = calloc(lexeme_len + 1, sizeof(char));
	memcpy(token.lexeme, scan->str + scan->start, lexeme_len);

	scan->tokens_count++;
	if (scan->tokens == NULL) {
		scan->tokens = malloc(sizeof(token_t) * scan->tokens_count);
	}
	else {
		scan->tokens = realloc(scan->tokens, sizeof(token_t) * scan->tokens_count);
	}

	if (token.type == TOKEN_IDENTIFIER) {
		token_type_t keyword = get_type_from_keyword(token.lexeme);
		if (keyword != TOKEN_NONE) {
			token.type = keyword;
		}
	}

	scan->tokens[scan->tokens_count-1] = token;
}

static void add_double_token(
	token_scanner_t* scan, char expected_next,
	token_type_t single_type, token_type_t double_type)
{
	if (scan->str[scan->current] == expected_next) {
		scan->current++;
		add_token(scan, double_type);
	}
	else {
		add_token(scan, single_type);
	}
}

static bool at_end(const token_scanner_t* scan)
{
	return !(scan->current < scan->str_len);
}

token_type_t get_type_from_keyword(const char* lexeme)
{
	if (strcmp(lexeme, "and") == 0) {
		return TOKEN_AND;
	}
	if (strcmp(lexeme, "class") == 0) {
		return TOKEN_CLASS;
	}
	if (strcmp(lexeme, "else") == 0) {
		return TOKEN_ELSE;
	}
	if (strcmp(lexeme, "false") == 0) {
		return TOKEN_FALSE;
	}
	if (strcmp(lexeme, "fun") == 0) {
		return TOKEN_FUN;
	}
	if (strcmp(lexeme, "for") == 0) {
		return TOKEN_FOR;
	}
	if (strcmp(lexeme, "if") == 0) {
		return TOKEN_IF;
	}
	if (strcmp(lexeme, "nil") == 0) {
		return TOKEN_NIL;
	}
	if (strcmp(lexeme, "or") == 0) {
		return TOKEN_OR;
	}
	if (strcmp(lexeme, "print") == 0) {
		return TOKEN_PRINT;
	}
	if (strcmp(lexeme, "return") == 0) {
		return TOKEN_RETURN;
	}
	if (strcmp(lexeme, "super") == 0) {
		return TOKEN_SUPER;
	}
	if (strcmp(lexeme, "this") == 0) {
		return TOKEN_THIS;
	}
	if (strcmp(lexeme, "true") == 0) {
		return TOKEN_TRUE;
	}
	if (strcmp(lexeme, "var") == 0) {
		return TOKEN_VAR;
	}
	if (strcmp(lexeme, "while") == 0) {
		return TOKEN_WHILE;
	}

	return TOKEN_NONE;
}
