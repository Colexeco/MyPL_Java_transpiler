
project(CPSC_326_MyPL_to_Java_Transpiler)

cmake_minimum_required(VERSION 3.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "-O0") 
set(CMAKE_BUILD_TYPE Debug)

include_directories("src")


# locate gtest
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

# create unit test executables
add_executable(MyPL_to_Java_Transpiler_Tests tests/MyPL_to_Java_Transpiler_Tests.cpp
  src/token.cpp src/mypl_exception.cpp src/lexer.cpp src/ast_parser.cpp
  src/symbol_table.cpp src/semantic_checker.cpp src/print_visitor.cpp src/mypl_to_java_transpiler.cpp 
  src/java_ast_parser.cpp src/java_lexer.cpp src/code_generator.cpp src/var_table.cpp src/vm_instr.cpp
  src/vm.cpp)
target_link_libraries(MyPL_to_Java_Transpiler_Tests ${GTEST_LIBRARIES} pthread)

# create mypl target
add_executable(mypl src/token.cpp src/mypl_exception.cpp src/lexer.cpp
  src/simple_parser.cpp src/ast_parser.cpp src/print_visitor.cpp
  src/symbol_table.cpp src/semantic_checker.cpp src/mypl.cpp src/mypl_to_java_transpiler.cpp 
  src/java_ast_parser.cpp src/java_lexer.cpp src/code_generator.cpp src/var_table.cpp src/vm_instr.cpp
  src/vm.cpp)
  
 
