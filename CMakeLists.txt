cmake_minimum_required(VERSION 3.19)

add_subdirectory(lib)

project(edra)

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

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC
        "inc"
)

target_compile_definitions(
    ${PROJECT_NAME}
    PRIVATE
        BUILD_TYPE_${CMAKE_BUILD_PLATFORM}
)

if(CMAKE_BUILD_PLATFORM MATCHES "WINDOWS")
find_package(SDL3 REQUIRED PATHS ${SDL3_PATH})
else()
find_package(SDL3 REQUIRED)
endif()
target_link_libraries(
    ${PROJECT_NAME}
    PUBLIC
        auxum
        cchip8
        SDL3::SDL3
)