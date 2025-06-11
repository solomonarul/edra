cmake_minimum_required(VERSION 3.20)

project(clay_sdl3)

file(
    GLOB_RECURSE
    ${PROJECT_NAME}_SRC
        "src/*.c"
)

add_library(
    ${PROJECT_NAME}
    STATIC
        ${${PROJECT_NAME}_SRC}
)

target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic -Werror)

if(CMAKE_BUILD_PLATFORM MATCHES "VITA")
    target_compile_options(${PROJECT_NAME} PRIVATE -Wno-unknown-pragmas -Wno-missing-field-initializers)
endif()

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC
        "inc/"
)

set_property(
    TARGET ${PROJECT_NAME}
    PROPERTY C_STANDARD 99
)

if(NOT "${SDL3_PATH}" STREQUAL "")
find_package(SDL3 REQUIRED PATHS ${SDL3_PATH})
else()
find_package(SDL3 REQUIRED)
endif()
if(NOT "${SDL3_IMAGE_PATH}" STREQUAL "")
find_package(SDL3_image REQUIRED PATHS ${SDL3_TTF_PATH})
else()
find_package(SDL3_image REQUIRED)
endif()
if(NOT "${SDL3_TTF_PATH}" STREQUAL "")
find_package(SDL3_ttf REQUIRED PATHS ${SDL3_TTF_PATH})
else()
find_package(SDL3_ttf REQUIRED)
endif()
target_link_libraries(
    ${PROJECT_NAME}
    PUBLIC
        SDL3::SDL3
        SDL3_ttf::SDL3_ttf
        SDL3_image::SDL3_image
)
