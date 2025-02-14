cmake_minimum_required(VERSION 3.19)

add_subdirectory(lib)

project(app)

file(
    GLOB_RECURSE
    ${PROJECT_NAME}_SRC
        "src/*.c"
)

add_executable(
    ${PROJECT_NAME}
    ${${PROJECT_NAME}_SRC}
)

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

set_property(
    TARGET ${PROJECT_NAME}
    PROPERTY C_STANDARD 23
)

find_package(SDL3 REQUIRED PATHS ${SDL3_PATH})
target_link_libraries(
    ${PROJECT_NAME}
    PUBLIC
        auxum
        cchip8
        SDL3::SDL3
)