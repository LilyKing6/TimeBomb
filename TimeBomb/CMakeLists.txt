# TimeBomb静态库配置

# 设置源文件
set(TMB_SRC_FILES
    TimeBomb.cpp
    utils.cpp
)

set(TMB_HEADER_FILES
    TimeBomb.h
    utils.h
    Itm.h
)

# 创建静态库
add_library(timebomb STATIC ${TMB_SRC_FILES})

# 设置库的属性
set_target_properties(timebomb PROPERTIES
    CXX_STANDARD ${CMAKE_CXX_STANDARD}
    CXX_STANDARD_REQUIRED ON
    POSITION_INDEPENDENT_CODE ON
)

# 设置包含目录
target_include_directories(timebomb
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
)

# 编译器特定设置
if(COMPILER_IS_MINGW)
    target_compile_options(timebomb PRIVATE
        -Wno-unused-parameter
        -Wno-unused-variable
    )
elseif(COMPILER_IS_MSVC)
    target_compile_definitions(timebomb PRIVATE
        _CRT_SECURE_NO_WARNINGS
        WIN32_LEAN_AND_MEAN
    )
elseif(COMPILER_IS_CLANG)
    target_compile_options(timebomb PRIVATE
        -Wno-unused-parameter
    )
endif()

# 平台特定的链接库
if(WIN32)
    target_link_libraries(timebomb PRIVATE ${PLATFORM_LIBS})
else()
    target_link_libraries(timebomb PRIVATE ${PLATFORM_LIBS})
endif()

# 安装头文件
install(FILES ${TMB_HEADER_FILES}
    DESTINATION include/timebomb
    COMPONENT development
)

# 安装库文件
install(TARGETS timebomb
    ARCHIVE DESTINATION lib
    COMPONENT libraries
)
