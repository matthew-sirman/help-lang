cmake_minimum_required(VERSION 3.17)
project(help-lang C)

set(CMAKE_C_STANDARD 11)

#set(CMAKE_C_FLAGS_DEBUG "-g -fsanitize=address")

add_executable(${PROJECT_NAME} main.c include/interpreter/interpreter.h src/interpreter/interpreter.c include/lang/type.h include/lang/func.h include/lang/expr.h include/lang/context.h src/lang/context.c include/compiler/parser.h src/compiler/parser.c include/util.h src/util.c include/lang/builtin.h src/lang/builtin.c include/lang/constant.h include/lang/var.h src/lang/func.c src/lang/var.c src/lang/expr.c include/compiler/compiler.h "src/compiler/compiler.c" src/lang/constant.c include/compiler/preprocessor.h src/compiler/preprocessor.c include/interpreter/call_stack.h src/interpreter/call_stack.c include/interpreter/string_buffer.h src/interpreter/string_buffer.c include/base.h)