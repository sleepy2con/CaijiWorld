cmake_minimum_required(VERSION 3.15)

# 只启用cpp编译器,不去用C编译器,配置阶段更快
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

# ==================== 💡 核心修改：显式列出所有源文件与头文件 ====================
# 按照你最新的目录结构，将 .h 和 .cpp 成双成对地写在这里。
# 只要手写在这里，Visual Studio 就会100%强制建立跳转和切换索引，再也不会失效！
set(CAIJIWORLD_SRC_FILES
    # --- core 模块 ---
    include/caijiworld/core/game.h
    src/core/game.cpp
    
    # --- world 模块 ---
    include/caijiworld/world/tile.h
    src/world/tile.cpp
    include/caijiworld/world/world.h
    src/world/world.cpp
    
    # --- ui 模块 ---
    include/caijiworld/ui/ui_manager.h
    src/ui/ui_manager.cpp

    # --- entity 模块 (根据你截图中的实体目录，后续新增文件可以按这种格式往里加) ---
    # include/caijiworld/entity/xxx.h
    # src/entity/xxx.cpp

    # --- utils 模块 ---
    # include/caijiworld/utils/xxx.h
    # src/utils/xxx.cpp

    # --- constant 模块 ---
    include/caijiworld/constant/constant.h

    # --- 根目录下的主工作流文件 ---
    src/caiji_workflow.cpp

    # --- 第三方库 ImGui 的源码 (归为该构建目标的资产) ---
    third/imgui/imgui.cpp
    third/imgui/imgui_draw.cpp
    third/imgui/imgui_tables.cpp
    third/imgui/imgui_widgets.cpp
    third/imgui/imgui_impl_sdl3.cpp
    third/imgui/imgui_impl_sdlrenderer3.cpp
)

# 将CAIJIWORLD_SRC_FILES中的所有文件编译链接成可执行文件,一个构建目标
# 生成CaijiWorld这个可执行程序(作为一个target,
# 也用于后面编译时target_include_directories的private修复符号,表示仅用于此target(构建目标))
add_executable(${PROJECT_NAME}
    ${CAIJIWORLD_SRC_FILES}
)
set(SPDLOG_HEADER_ONLY OFF)

# 头文件不需要编译,告诉编译器编译源文件时,头文件在哪,用于CaijiWorld这个目标程序
# 💡 核心修改：将自己写的代码与第三方库隔离，用 SYSTEM 关键字让编译器闭嘴
target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
)

target_include_directories(${PROJECT_NAME}
    SYSTEM PRIVATE
    ${PROJECT_SOURCE_DIR}/include/imgui
    ${PROJECT_SOURCE_DIR}/include/SDL3
    ${PROJECT_SOURCE_DIR}/include/SDL3_image
    ${PROJECT_SOURCE_DIR}/include/spdlog
)

# 💡 专门针对微软 VS 代码分析器的终极补丁：强行让静态分析器忽略第三方库目录
# 直接在生成的 VS 后台工程配置文件里注入一条禁令，告诉 VS 静态分析器：“这几个路径是法外之地，永远不要进去扫描警告！
if(MSVC)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        VS_GLOBAL_CAExcludePath "${PROJECT_SOURCE_DIR}/include/spdlog;${PROJECT_SOURCE_DIR}/include/imgui;${PROJECT_SOURCE_DIR}/include/SDL3"
    )
endif()
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