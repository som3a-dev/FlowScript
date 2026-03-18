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

#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#endif
#include <crtdbg.h>

#include "scanner.h"

// Maximum size of user's input string
#define MAX_INPUT_SIZE 512

static bool running = true;

static bool exec_command(const char* input);

static void clear_stdout();

static void init_crtdbg();
static void dump_crtdbg();

int main(void)
{
	#ifdef _DEBUG
	init_crtdbg();
	#endif

	char buf[MAX_INPUT_SIZE];
	while (running)
	{
		printf("> ");
		fgets(buf, MAX_INPUT_SIZE, stdin);

		buf[strlen(buf)-1] = '\0'; // strip the newline

		if (exec_command(buf)) {
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

	#ifdef _DEBUG
	dump_crtdbg();
	#endif

	return 0;
}

static bool exec_command(const char* input)
{
	bool result = true;

	if (strcmp(input, ".cls") == 0) {
		clear_stdout();
	}
	else if (strcmp(input, ".q") == 0) {
		running = false;
	}
	else {
		result = false;
	}

	return result;
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

static void init_crtdbg()
{
    _CrtSetReportMode( _CRT_WARN, _CRTDBG_MODE_FILE );
    _CrtSetReportFile( _CRT_WARN, _CRTDBG_FILE_STDOUT );
    _CrtSetReportMode( _CRT_ERROR, _CRTDBG_MODE_FILE );
    _CrtSetReportFile( _CRT_ERROR, _CRTDBG_FILE_STDOUT );
    _CrtSetReportMode( _CRT_ASSERT, _CRTDBG_MODE_FILE );
    _CrtSetReportFile( _CRT_ASSERT, _CRTDBG_FILE_STDOUT );
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
}

static void dump_crtdbg()
{
	_CrtDumpMemoryLeaks();
}