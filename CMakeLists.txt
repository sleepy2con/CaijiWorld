cmake_minimum_required(VERSION 3.20)
# LANGUAGES CXX:这个工程只使用 C++ 语言，让 CMake 只初始化 C++ 编译器和相关规则
project(CaijiWorld LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(CaijiWorld
    src/main.cpp
)

target_include_directories(CaijiWorld PRIVATE
    include
)