cmake_minimum_required(VERSION 3.10)
project(ai_glasses VERSION 1.0.0 LANGUAGES CXX)

# C++ 标准设置
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# 构建选项
option(BUILD_SHARED_LIBS "Build shared libraries" ON)
option(BUILD_ANDROID_JNI "Build Android JNI library" ON)
option(BUILD_TESTS "Build tests" OFF)

# 输出目录设置
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# 源文件列表
set(LIB_SOURCES
    src/embedding_matcher.cpp
    src/embedding_model.cpp
    src/jieba_segmenter.cpp
    src/json_parser.cpp
)

# 如果需要 JNI 支持
if(BUILD_ANDROID_JNI AND NOT ANDROID)
    message(WARNING "BUILD_ANDROID_JNI is ON but ANDROID is OFF, ignoring")
endif()

if(BUILD_ANDROID_JNI AND ANDROID)
    list(APPEND LIB_SOURCES
        src/embedding_matcher_jni.cpp
    )
endif()

# 创建库
if(BUILD_SHARED_LIBS)
    add_library(ai_glasses SHARED ${LIB_SOURCES})
else()
    add_library(ai_glasses STATIC ${LIB_SOURCES})
endif()

# 包含目录
target_include_directories(ai_glasses
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/cppjieba/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# 编译选项
if(ANDROID)
    target_compile_options(ai_glasses PRIVATE
        -Wall
        -Wextra
        -O2
        -fexceptions
        -frtti
        -std=c++14
    )
    
    # Android 特定设置
    if(BUILD_ANDROID_JNI)
        find_library(log-lib log)
        target_link_libraries(ai_glasses ${log-lib})
    endif()
else()
    # 桌面平台设置
    target_compile_options(ai_glasses PRIVATE
        -Wall
        -Wextra
        -O2
    )
endif()

# 链接选项
if(NOT ANDROID)
    if(UNIX)
        target_link_libraries(ai_glasses PRIVATE pthread)
    endif()
endif()

# 安装设置
include(GNUInstallDirs)

install(TARGETS ai_glasses
    EXPORT ai_glasses-targets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# 安装头文件
install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.h"
)

# 导出配置
install(EXPORT ai_glasses-targets
    FILE ai_glasses-targets.cmake
    NAMESPACE ai_glasses::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ai_glasses
)

# 创建配置文件
include(CMakePackageConfigHelpers)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/ai_glasses-config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/ai_glasses-config.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ai_glasses
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/ai_glasses-config-version.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/ai_glasses-config.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/ai_glasses-config-version.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ai_glasses
)

# 测试（可选）
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()

# 打印配置信息
message(STATUS "=================================")
message(STATUS "AI Glasses 配置")
message(STATUS "=================================")
message(STATUS "版本：${PROJECT_VERSION}")
message(STATUS "构建类型：${CMAKE_BUILD_TYPE}")
message(STATUS "库类型：$<IF:$<BOOL:${BUILD_SHARED_LIBS}>,SHARED,STATIC>")
message(STATUS "Android: $<IF:$<BOOL:${ANDROID}>,Yes,No>")
message(STATUS "JNI 支持：$<IF:$<BOOL:${BUILD_ANDROID_JNI}>,Yes,No>")
message(STATUS "=================================")
