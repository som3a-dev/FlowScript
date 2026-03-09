/*
 * File: main.c
 * Created on Mon Mar 09 2026
 *
 * Copyright (c) 2026 Omar Eltayeb
 */

#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#ifdef _WIN32
#include <Windows.h>
#endif

#include "scanner.h"

// Maximum size of user's input string
#define MAX_INPUT_SIZE 512

static bool running = true;

static void clear_stdout();

int main(void)
{
	char buf[MAX_INPUT_SIZE];
	while (running)
	{
		printf("> ");
		fgets(buf, MAX_INPUT_SIZE, stdin);

		buf[strlen(buf)-1] = '\0'; // strip the newline

		if (strcmp(buf, "cls") == 0) {
			clear_stdout();
			continue;
		}
		if (strcmp(buf, "q") == 0) {
			running = false;
			continue;
		}

		token_list_t tokens = token_scanner_scan(buf);

		for (int i = 0; i < tokens.len; i++)
		{
			token_print(tokens.tokens[i]);
		}

		token_list_delete(&tokens);

		buf[0] = '\0'; // clear the buffer
	}

	return 0;
}

static void clear_stdout()
{
	#ifdef _WIN32

	HANDLE console_handle = GetStdHandle(STD_OUTPUT_HANDLE);

	CONSOLE_SCREEN_BUFFER_INFO console_buf_info;
	GetConsoleScreenBufferInfo(console_handle, &console_buf_info);

	DWORD console_size = console_buf_info.dwSize.X * console_buf_info.dwSize.Y;
	COORD cursor = {0, 0};
	DWORD chars_written;
	FillConsoleOutputCharacter(console_handle, (TCHAR)' ', console_size, cursor, &chars_written);

	GetConsoleScreenBufferInfo(console_handle, &console_buf_info);
	FillConsoleOutputAttribute(console_handle,
	console_buf_info.wAttributes,
	console_size, cursor, &chars_written);

	SetConsoleCursorPosition(console_handle, cursor);

	#endif
}