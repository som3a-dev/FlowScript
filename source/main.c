/*
 * File: main.c
 * Created on Mon Mar 09 2026
 *
 * Copyright (c) 2026 Omar Eltayeb
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <Windows.h>
#endif

#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#endif
#include <crtdbg.h>

#include "interpreter.h"
#include "parser.h"
#include "scanner.h"

// Maximum size of user's input string
#define MAX_INPUT_SIZE 512

static bool running = true;

static bool exec_command(const char* input);

static void read_input_file(const char* filename);

static void clear_stdout();

/*
 * gets the Xth substr out of a string
 * caller must free
 */
static char* get_substr_delim(const char* str, char delim, int i);

/*
 * returns the amount of delimiter split substrings in the string
 */
static int get_substr_delim_count(const char* str, char delim);

static void init_crtdbg();

static void dump_crtdbg();

static char buf[MAX_INPUT_SIZE];

int main(void)
{
#ifdef _DEBUG
    init_crtdbg();
#endif

    init_interpreter();
    while (running)
    {
        printf("> ");
        fgets(buf, MAX_INPUT_SIZE, stdin);

        buf[strlen(buf) - 1] = '\0'; // strip the newline

        if (exec_command(buf))
        {
            continue;
        }

        bool error = false;
        token_list_t tokens = scan(buf, &error);
        if (error)
        {
            goto done;
        }
        declaration_list_t stmts = parse(&tokens, &error);
        if (error)
        {
            goto done;
        }

        interpret(&stmts);

    done:
        declaration_list_free(&stmts);
        token_list_delete(&tokens);

        buf[0] = '\0'; // clear the buffer
    }

    destroy_interpreter();

    return 0;
}

static bool exec_command(const char* input)
{
    char* cmd = get_substr_delim(input, ' ', 0);
    int count = get_substr_delim_count(input, ' ');

    if (!cmd)
    {
        return false;
    }

    if (cmd[0] != '.')
    {
        free(cmd);
        return false;
    }

    bool result = true;

    if ((strcmp(cmd, ".cls") == 0) && count == 1)
    {
        clear_stdout();
    }
    else if ((strcmp(cmd, ".q") == 0) && count == 1)
    {
        running = false;
    }
    else if ((strcmp(cmd, ".run") == 0) && count == 2)
    {
        char* filename = get_substr_delim(input, ' ', 1);
        printf("Running file: '%s'\n", filename);
        read_input_file(filename);

        free(filename);

        // To execute the input code we read
        result = false;
    }
    else
    {
        printf("REPL ERROR: Invalid Command\n");
    }

    free(cmd);
    return result;
}

void read_input_file(const char* filename)
{
    FILE* fp = fopen(filename, "r");
    if (!fp)
    {
        printf("IO ERROR: File could not be opened.\n");
        return;
    }

    fread(buf, sizeof(char), MAX_INPUT_SIZE, fp);

    fclose(fp);
}

static void clear_stdout()
{
#ifdef _WIN32

    HANDLE console_handle = GetStdHandle(STD_OUTPUT_HANDLE);

    CONSOLE_SCREEN_BUFFER_INFO console_buf_info;
    GetConsoleScreenBufferInfo(console_handle, &console_buf_info);

    DWORD console_size = console_buf_info.dwSize.X * console_buf_info.dwSize.Y;
    COORD cursor = { 0, 0 };
    DWORD chars_written;
    FillConsoleOutputCharacter(
        console_handle, (TCHAR)' ', console_size, cursor, &chars_written);

    GetConsoleScreenBufferInfo(console_handle, &console_buf_info);
    FillConsoleOutputAttribute(
        console_handle,
        console_buf_info.wAttributes,
        console_size,
        cursor,
        &chars_written);

    SetConsoleCursorPosition(console_handle, cursor);

#endif
}

static char* get_substr_delim(const char* str, char delim, int index)
{
    if (index < 0)
    {
        return NULL;
    }

    char* result = NULL;

    int current_index = -1;
    int result_len = 0;
    for (int i = 0; i < strlen(str); i++)
    {
        if (str[i] == delim)
        {
            current_index++;

            if (current_index != index)
            {
                result_len = 0;
                continue;
            }
            else if (result_len != 0)
            { // avoid returning a string consisting of
                // the delimiter as the result
                result = calloc(result_len + 1, sizeof(char));

                memcpy(
                    result, str + (i - result_len), result_len * sizeof(char));
                result[result_len] = '\0';

                break;
            }
        }

        result_len++;
    }

    if ((current_index == index - 1))
    {
        result = calloc(result_len + 1, sizeof(char));
        strcpy(result, str + strlen(str) - result_len);
    }

    return result;
}

int get_substr_delim_count(const char* str, char delim)
{
    int count = 0;

    int sub_len = 0;
    for (int i = 0; i < strlen(str); i++)
    {
        if (str[i] == delim)
        {
            if (sub_len != 0)
            {
                count++;
            }

            sub_len = 0;
            continue;
        }

        sub_len++;
    }

    if (sub_len != 0)
    {
        count++;
    }

    return count;
}

static void init_crtdbg()
{
    _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDOUT);
    _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDOUT);
    _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDOUT);
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
}

static void dump_crtdbg()
{
    _CrtDumpMemoryLeaks();
}