cmake_minimum_required(VERSION 3.15)

#只启用cpp编译器,不去用C编译器,配置阶段更快
project(CaijiWorld LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
#要么用我指定的版本,要么编译失败
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 源文件
file(GLOB_RECURSE SRC_FILES
    src/*.cpp
)

add_executable(CaijiWorld
    ${SRC_FILES}
)

target_include_directories(CaijiWorld
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
)