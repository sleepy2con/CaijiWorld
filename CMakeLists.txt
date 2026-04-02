cmake_minimum_required(VERSION 3.15)

#只启用cpp编译器,不去用C编译器,配置阶段更快
# CaijiWorld项目名字,作为IDE工程名
project(CaijiWorld LANGUAGES CXX)

if(MSVC)
    set(CMAKE_GENERATOR_PLATFORM x64)
    # 加上下面这一行，强制编译器使用 UTF-8 编码
    add_compile_options(/utf-8)
endif()

set(CMAKE_CXX_STANDARD 20)
# 要么用我指定的版本,要么编译失败
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 源文件
file(GLOB_RECURSE CAIJIWORLD_SRC_FILES
    src/*.cpp
    third/imgui/*.cpp
)

# 将CAIJIWORLD_SRC_FILES中的所有cpp文件编译链接成可执行文件,一个构建目标
# 生成CaijiWorld这个可执行程序(作为一个target,
# 也用于后面编译时target_include_directories的private修复符号,表示仅用于此target(构建目标))
add_executable(${PROJECT_NAME}
    ${CAIJIWORLD_SRC_FILES}
)
set(SPDLOG_HEADER_ONLY OFF)

# 头文件不需要编译,告诉编译器编译源文件时,头文件在哪,用于CaijiWorld这个目标程序
target_include_directories(CaijiWorld
    PRIVATE
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/include/imgui
)


#link_directories(${PROJECT_SOURCE_DIR}/third/SDL3/lib)

# 需要配置环境变量可能？或者用一些定义路径的方式
#find_package(SDL3 REQUIRED)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
    ${PROJECT_SOURCE_DIR}/third/SDL3-3.4.0/SDL3.lib
    ${PROJECT_SOURCE_DIR}/third/SDL3_image-3.4.0/SDL3_image.lib
    ${PROJECT_SOURCE_DIR}/third/spdlog/spdlogd.lib
    opengl32.lib
)

if(WIN32)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${PROJECT_SOURCE_DIR}/third/SDL3-3.4.0/SDL3.dll"      # 源路径
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>" # 目标路径：可执行文件所在目录
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${PROJECT_SOURCE_DIR}/third/SDL3_image-3.4.0/SDL3_image.dll"      # 源路径
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>" # 目标路径：可执行文件所在目录
    )
endif()