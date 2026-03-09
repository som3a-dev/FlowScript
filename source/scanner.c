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

typedef struct
{
	char* str;
	size_t str_len; 

	token_t* tokens;
	int tokens_count;

	int current;
	int line;	
} token_scanner_t;

static void add_token(token_scanner_t* scan, token_t token);
static void scan_token(token_scanner_t* scan);

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

	free(scan.str);

	return list;
}

void token_print(token_t token)
{
    printf("Type: %d Lexem: '%s' Line: %d\n", token.type, token.lexem, token.line);
}

void token_list_delete(token_list_t* list)
{
	for (int i = 0; i < list->len; i++)
	{
		free(list->tokens[i].lexem);
	}

	free(list->tokens);
	list->tokens = NULL;
	list->len = 0;
}

static void scan_token(token_scanner_t* scan)
{
	char c = scan->str[scan->current];

	token_t token = {0};
    token.line = 1;
	switch (c)
	{
		case '(': {
			token.type = TOKEN_LEFT_PAREN;
			token.lexem = calloc(2, sizeof(char));
			token.lexem[0] = c;
			scan->current++;
		} break;
		case ')': {
			token.type = TOKEN_RIGHT_PAREN;
			token.lexem = calloc(2, sizeof(char));
			token.lexem[0] = c;
			scan->current++;
		} break;
		case '{': {
			token.type = TOKEN_LEFT_BRACE;
			token.lexem = calloc(2, sizeof(char));
			token.lexem[0] = c;
			scan->current++;
		} break;
		case '}': {
			token.type = TOKEN_RIGHT_BRACE;
			token.lexem = calloc(2, sizeof(char));
			token.lexem[0] = c;
			scan->current++;
		} break;

		case ',': {
			token.type = TOKEN_COMMA;
			token.lexem = calloc(2, sizeof(char));
			token.lexem[0] = c;
			scan->current++;
		} break;
		case '.': {
			token.type = TOKEN_DOT;
			token.lexem = calloc(2, sizeof(char));
			token.lexem[0] = c;
			scan->current++;
		} break;

		case '-': {
			token.type = TOKEN_MINUS;
			token.lexem = calloc(2, sizeof(char));
			token.lexem[0] = c;
			scan->current++;
		} break;
		case '+': {
			token.type = TOKEN_PLUS;
			token.lexem = calloc(2, sizeof(char));
			token.lexem[0] = c;
			scan->current++;
		} break;
		case '*': {
			token.type = TOKEN_STAR;
			token.lexem = calloc(2, sizeof(char));
			token.lexem[0] = c;
			scan->current++;
		} break;

		case ';': {
			token.type = TOKEN_SEMICOLON;
			token.lexem = calloc(2, sizeof(char));
			token.lexem[0] = c;
			scan->current++;
		} break;
	}

	if (token.type != TOKEN_NONE) {
		add_token(scan, token);
	}
	else {
		scan->current++;
	}
}

static void add_token(token_scanner_t* scan, token_t token)
{
	scan->tokens_count++;

	if (scan->tokens == NULL) {
		scan->tokens = malloc(sizeof(token_t) * scan->tokens_count);
	}
	else {
		scan->tokens = realloc(scan->tokens, sizeof(token_t) * scan->tokens_count);
	}

	scan->tokens[scan->tokens_count-1] = token;
}