cmake_minimum_required(VERSION 3.18)
project(AIGlasses VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

find_package(JNI REQUIRED)

include_directories(${JNI_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

file(GLOB_RECURSE SOURCES
    "src/*.cpp"
    "src/*.c"
)

message(STATUS "Found source files: ${SOURCES}")

add_library(ai_glasses SHARED ${SOURCES})

target_include_directories(ai_glasses PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${JNI_INCLUDE_DIRS}
)

if(ANDROID)
    target_link_libraries(ai_glasses
        android
        log
    )
endif()

if(WIN32)
    set_target_properties(ai_glasses PROPERTIES
        SUFFIX ".dll"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    )
elseif(APPLE)
    set_target_properties(ai_glasses PROPERTIES
        SUFFIX ".dylib"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    )
else()
    set_target_properties(ai_glasses PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    )
endif()
