cmake_minimum_required(VERSION 3.15)

project(TimeBomb VERSION 1.1 LANGUAGES CXX)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 如果未指定构建类型，默认为Release
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# 编译器检测和配置选项
option(FORCE_MINGW "Force use MinGW compiler" ON)
option(ENABLE_STATIC_LINKING "Enable static linking" ON)
option(ENABLE_MODULES "Enable C++ modules support" OFF)
option(ENABLE_WARNINGS "Enable extra warnings" ON)

# 编译器检测函数
function(detect_compiler)
    if(WIN32)
        message(STATUS "Detecting available compilers on Windows...")

        # 优先查找MinGW编译器
        if(FORCE_MINGW OR NOT CMAKE_CXX_COMPILER_ID)
            # 查找MinGW-w64编译器
            find_program(MINGW_GXX NAMES
                x86_64-w64-mingw32-g++
                mingw32-g++
                g++
                PATHS
                "C:/mingw64/bin"
                "C:/msys64/mingw64/bin"
                "C:/Program Files/mingw-w64/x86_64-8.1.0-posix-seh-rt_v6-rev0/mingw64/bin"
                ENV PATH
            )

            if(MINGW_GXX)
                set(CMAKE_CXX_COMPILER ${MINGW_GXX} PARENT_SCOPE)
                message(STATUS "Found MinGW compiler: ${MINGW_GXX}")
                set(COMPILER_TYPE "MinGW" PARENT_SCOPE)
                return()
            endif()
        endif()

        # 查找LLVM/Clang编译器
        find_program(CLANG_CXX NAMES
            clang++
            clang
            PATHS
            "C:/Program Files/LLVM/bin"
            "C:/LLVM/bin"
            ENV PATH
        )

        if(CLANG_CXX AND NOT FORCE_MINGW)
            set(CMAKE_CXX_COMPILER ${CLANG_CXX} PARENT_SCOPE)
            message(STATUS "Found Clang compiler: ${CLANG_CXX}")
            set(COMPILER_TYPE "Clang" PARENT_SCOPE)
            return()
        endif()

        # 最后尝试MSVC (如果其他都不可用)
        if(MSVC AND NOT FORCE_MINGW)
            message(STATUS "Using MSVC compiler")
            set(COMPILER_TYPE "MSVC" PARENT_SCOPE)
            return()
        endif()

        # 如果都没找到，使用默认
        if(NOT MINGW_GXX AND NOT CLANG_CXX)
            message(WARNING "No preferred compiler found, using default")
            set(COMPILER_TYPE "Default" PARENT_SCOPE)
        endif()

    else()
        # Linux系统
        set(COMPILER_TYPE "GCC" PARENT_SCOPE)
        message(STATUS "Using GCC on Linux")
    endif()
endfunction()

# 检测编译器
detect_compiler()

# 编译器特定的配置
if(WIN32)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU" OR COMPILER_TYPE STREQUAL "MinGW")
        message(STATUS "Configuring for MinGW/GCC")
        set(COMPILER_IS_MINGW TRUE)

        # MinGW 编译选项
        set(BASE_CXX_FLAGS "-Wall -Wextra")
        if(ENABLE_WARNINGS)
            set(BASE_CXX_FLAGS "${BASE_CXX_FLAGS} -Wpedantic -Wno-unused-parameter")
        endif()

        if(ENABLE_STATIC_LINKING)
            set(BASE_CXX_FLAGS "${BASE_CXX_FLAGS} -static-libgcc -static-libstdc++")
        endif()

        if(ENABLE_MODULES)
            set(BASE_CXX_FLAGS "${BASE_CXX_FLAGS} -fmodules-ts")
        endif()

        # 链接库
        set(PLATFORM_LIBS kernel32 ws2_32)

    elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        message(STATUS "Configuring for Clang")
        set(COMPILER_IS_CLANG TRUE)

        # Clang 编译选项
        set(BASE_CXX_FLAGS "-Wall -Wextra")
        if(ENABLE_WARNINGS)
            set(BASE_CXX_FLAGS "${BASE_CXX_FLAGS} -Wpedantic")
        endif()

        # 链接库
        set(PLATFORM_LIBS kernel32 ws2_32)

    elseif(MSVC)
        message(STATUS "Configuring for MSVC")
        set(COMPILER_IS_MSVC TRUE)

        # MSVC 编译选项
        set(BASE_CXX_FLAGS "/W4 /EHsc")
        if(ENABLE_STATIC_LINKING)
            set(BASE_CXX_FLAGS "${BASE_CXX_FLAGS} /MT")
        endif()

        # 链接库
        set(PLATFORM_LIBS kernel32 ws2_32)

        # MSVC特定定义
        add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    endif()

else()
    # Linux 配置
    message(STATUS "Configuring for Linux GCC")
    set(BASE_CXX_FLAGS "-Wall -Wextra")
    if(ENABLE_WARNINGS)
        set(BASE_CXX_FLAGS "${BASE_CXX_FLAGS} -Wpedantic")
    endif()

    if(ENABLE_STATIC_LINKING)
        set(BASE_CXX_FLAGS "${BASE_CXX_FLAGS} -static-libgcc -static-libstdc++")
    endif()

    # Linux 链接库
    set(PLATFORM_LIBS dl pthread)
endif()

# 构建类型特定的编译选项
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    if(COMPILER_IS_MSVC)
        set(BUILD_CXX_FLAGS "/Od /Zi /RTC1")
    else()
        set(BUILD_CXX_FLAGS "-O0 -g -DDEBUG")
    endif()
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    if(COMPILER_IS_MSVC)
        set(BUILD_CXX_FLAGS "/O2 /DNDEBUG")
    else()
        set(BUILD_CXX_FLAGS "-O2 -DNDEBUG")
    endif()
elseif(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    if(COMPILER_IS_MSVC)
        set(BUILD_CXX_FLAGS "/O2 /Zi /DNDEBUG")
    else()
        set(BUILD_CXX_FLAGS "-O2 -g -DNDEBUG")
    endif()
endif()

# 组合所有编译选项
set(CMAKE_CXX_FLAGS "${BASE_CXX_FLAGS} ${BUILD_CXX_FLAGS}")

# 显示配置信息
message(STATUS "=== TimeBomb Build Configuration ===")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "Compiler ID: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "CXX Flags: ${CMAKE_CXX_FLAGS}")
message(STATUS "Platform Libraries: ${PLATFORM_LIBS}")
message(STATUS "Static Linking: ${ENABLE_STATIC_LINKING}")
message(STATUS "Modules Support: ${ENABLE_MODULES}")
message(STATUS "=====================================")

# 设置包含目录
include_directories(
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/TimeBomb
)

# 输出目录配置
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# 可执行文件名称
set(OUTPUT_NAME tmb)
set(TEST_NAME test_timebomb)

# 添加子目录
add_subdirectory(TimeBomb)

# 主程序源文件
set(MAIN_SRC_FILES main.cpp)

# ================================
# 主程序目标
# ================================
add_executable(${OUTPUT_NAME} ${MAIN_SRC_FILES})

# 设置目标属性
set_target_properties(${OUTPUT_NAME} PROPERTIES
    OUTPUT_NAME ${OUTPUT_NAME}
    CXX_STANDARD ${CMAKE_CXX_STANDARD}
    CXX_STANDARD_REQUIRED ON
)

# 链接库
target_link_libraries(${OUTPUT_NAME}
    PRIVATE timebomb
    PRIVATE ${PLATFORM_LIBS}
)

# 编译器特定的目标设置
if(COMPILER_IS_MINGW)
    target_compile_options(${OUTPUT_NAME} PRIVATE -Wno-unused-function)
elseif(COMPILER_IS_MSVC)
    target_compile_definitions(${OUTPUT_NAME} PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()

# ================================
# 测试程序目标
# ================================
set(TEST_SRC_FILES test_timebomb.cpp)
add_executable(${TEST_NAME} ${TEST_SRC_FILES})

# 设置测试目标属性
set_target_properties(${TEST_NAME} PROPERTIES
    OUTPUT_NAME ${TEST_NAME}
    CXX_STANDARD ${CMAKE_CXX_STANDARD}
    CXX_STANDARD_REQUIRED ON
)

# 链接库
target_link_libraries(${TEST_NAME}
    PRIVATE timebomb
    PRIVATE ${PLATFORM_LIBS}
)

# 编译器特定的测试目标设置
if(COMPILER_IS_MINGW)
    target_compile_options(${TEST_NAME} PRIVATE -Wno-unused-function)
elseif(COMPILER_IS_MSVC)
    target_compile_definitions(${TEST_NAME} PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()

# ================================
# 自定义目标和安装规则
# ================================

# 清理目标
add_custom_target(clean_all
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target clean
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/bin
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/lib
    COMMENT "Deep cleaning all generated files and directories"
)

# 运行目标
add_custom_target(run
    COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${OUTPUT_NAME}
    DEPENDS ${OUTPUT_NAME}
    COMMENT "Running TimeBomb application"
    WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)

# 测试目标
add_custom_target(test_run
    COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TEST_NAME}
    DEPENDS ${TEST_NAME}
    COMMENT "Running TimeBomb tests"
    WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)

# 显示编译器信息目标
add_custom_target(info
    COMMAND ${CMAKE_COMMAND} -E echo "Compiler: ${CMAKE_CXX_COMPILER}"
    COMMAND ${CMAKE_COMMAND} -E echo "Compiler ID: ${CMAKE_CXX_COMPILER_ID}"
    COMMAND ${CMAKE_COMMAND} -E echo "Build Type: ${CMAKE_BUILD_TYPE}"
    COMMAND ${CMAKE_COMMAND} -E echo "CXX Flags: ${CMAKE_CXX_FLAGS}"
    COMMENT "Displaying build information"
)

# 安装规则
install(TARGETS ${OUTPUT_NAME} ${TEST_NAME}
    RUNTIME DESTINATION bin
    COMPONENT applications
)

install(FILES README.md
    DESTINATION .
    COMPONENT documentation
)

# CPack配置
set(CPACK_PACKAGE_NAME "TimeBomb")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "TimeBomb Software Protection System")
set(CPACK_PACKAGE_VENDOR "Lily King")

if(WIN32)
    set(CPACK_GENERATOR "ZIP;NSIS")
else()
    set(CPACK_GENERATOR "TGZ;DEB")
endif()

include(CPack)
