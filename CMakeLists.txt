cmake_minimum_required(VERSION 3.10)
project(TestCompiler LANGUAGES C CXX)  # 同时支持 C 和 C++

# 设置输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# Flex 配置
find_package(FLEX REQUIRED)
FLEX_TARGET(Ilexer src/lex.l ${CMAKE_CURRENT_BINARY_DIR}/lex.yy.c)

# 包含头文件路径
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

# 源文件
set(SOURCES
    src/main.c
    src/util.c
    src/parse.c
    src/codegen.c
    src/print.c
    ${FLEX_Ilexer_OUTPUTS}
)

# C 目标
add_executable(test_compiler ${SOURCES})

# C++ 目标（明确语言）
add_executable(machine machine.cpp)
set_target_properties(machine PROPERTIES LINKER_LANGUAGE CXX)  # 可选，但更明确