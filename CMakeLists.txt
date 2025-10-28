cmake_minimum_required(VERSION 3.20)

add_subdirectory(lib)

project(edra)

file(
    GLOB_RECURSE
    ${PROJECT_NAME}_SRC
        "src/*.c"
)

if(CMAKE_BUILD_PLATFORM MATCHES "WINDOWS")
    add_executable(
        ${PROJECT_NAME}
        WIN32
            ${${PROJECT_NAME}_SRC}
    )
else()
    add_executable(
        ${PROJECT_NAME}
        ${${PROJECT_NAME}_SRC}
    )
endif()

target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic -Werror)

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

if(NOT "${SDL3_PATH}" STREQUAL "")
find_package(SDL3 REQUIRED PATHS ${SDL3_PATH})
else()
find_package(SDL3 REQUIRED)
endif()
if(NOT "${SDL3_TTF_PATH}" STREQUAL "")
find_package(SDL3_ttf REQUIRED PATHS ${SDL3_TTF_PATH})
else()
find_package(SDL3_ttf REQUIRED)
endif()
if(NOT "${SDL3_IMAGE_PATH}" STREQUAL "")
find_package(SDL3_image REQUIRED PATHS ${SDL3_TTF_PATH})
else()
find_package(SDL3_image REQUIRED)
endif()
target_link_libraries(
    ${PROJECT_NAME}
    PUBLIC
        auxum
        cbf
        cchip8
        SDL3::SDL3
)

if(NOT BUILD_TESTS)
install(
    TARGETS
    ${PROJECT_NAME}
)
endif()