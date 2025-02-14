cmake_minimum_required(VERSION 3.19)

project(cchip8)

file(
    GLOB_RECURSE
    ${PROJECT_NAME}_SRC
        "src/*.c"
)

file(
    GLOB_RECURSE
    ${PROJECT_NAME}_TEST_SRC
        "test/*.c"
)

add_library(
    ${PROJECT_NAME}
    ${${PROJECT_NAME}_SRC}
)

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC
        "inc/"
    PRIVATE
        "inc/cchip8"
)

set_property(
    TARGET ${PROJECT_NAME}
    PROPERTY C_STANDARD 23
)

if(CMAKE_BUILD_TYPE MATCHES "Debug")
add_executable(
    ${PROJECT_NAME}_test
    ${${PROJECT_NAME}_TEST_SRC}
)

if(MSVC)
    target_compile_options(${PROJECT_NAME}_test PRIVATE /W4 /WX)
else()
    target_compile_options(${PROJECT_NAME}_test PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

target_link_libraries(
    ${PROJECT_NAME}_test
    PUBLIC
        ${PROJECT_NAME}
)

set_property(
    TARGET ${PROJECT_NAME}_test
    PROPERTY C_STANDARD 23
)
endif()

