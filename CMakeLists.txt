# CMakeList.txt : CMake project for FlowScript, include source and define
# project specific logic here.
#
cmake_minimum_required(VERSION 3.8)

# Enable Hot Reload for MSVC compilers if supported.
if(POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF) # Optional: ensures strict C99 without GNU extensions

project("FlowScript")

set(SOURCES
  source/main.c
  source/scanner.c
  source/parser.c
  source/interpreter.c
  source/object.c
  source/environment.c
  source/expression.c
  source/statement.c
  source/list.c
)

add_compile_definitions(USE_NEW_CACHE _CRT_SECURE_NO_WARNINGS)

# Add source to this project's executable.
add_executable(FlowScript ${SOURCES})

if(MSVC)
  target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX)
else()
  target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

# TODO: Add tests and install targets if needed.
