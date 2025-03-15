cmake_minimum_required(VERSION 3.20)

# PSvita specifics
if(CMAKE_BUILD_PLATFORM MATCHES "VITA")
    # VitaSDK is required
    if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
        if(DEFINED ENV{VITASDK})
            set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "toolchain file")
        else()
            message(FATAL_ERROR "Please define VITASDK to point to your SDK path!")
        endif()
    endif()
else()
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ../bin/)
endif()

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

target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic -Werror)

set_property(
    TARGET ${PROJECT_NAME}
    PROPERTY C_STANDARD 17
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
target_link_libraries(
    ${PROJECT_NAME}
    PUBLIC
        auxum
        cchip8
        SDL3::SDL3
)

if(CMAKE_BUILD_PLATFORM MATCHES "VITA")
    target_link_libraries(
        ${PROJECT_NAME}
        PUBLIC
            libGL_stub
    )

    ## Configuration options for this app
    set(VITA_APP_NAME "Edra")                                           # Display name.
    set(VITA_TITLEID  "SLMN00000")                                      # Unique ID must be exactly 9 characters. Recommended: XXXXYYYYY
    set(VITA_VERSION  "01.00")                                          # Shown in LiveArea's more info screen.
    # set(VITA_MKSFOEX_FLAGS "${VITA_MKSFOEX_FLAGS} -d ATTRIBUTE2=12")    # +109 MB of RAM, system apps are not allowed to run alongside.

    include("$ENV{VITASDK}/share/vita.cmake" REQUIRED)

    ## Create Vita files
    vita_create_self(${PROJECT_NAME}.self ${PROJECT_NAME} UNSAFE)

    # The FILE directive lets you add additional files to the VPK, the syntax is
    # FILE src_path dst_path_in_vpk. In this case, we add the LiveArea paths.
    vita_create_vpk(
        ${PROJECT_NAME}.vpk
        ${VITA_TITLEID}
        ${PROJECT_NAME}.self
        VERSION ${VITA_VERSION}
        NAME ${VITA_APP_NAME}
        FILE vita-metadata sce_sys
        FILE vita-module module
    )
elseif(CMAKE_BUILD_PLATFORM MATCHES "WINDOWS")
    target_link_libraries(
        ${PROJECT_NAME}
        PUBLIC
            opengl32
    )
    install(TARGETS ${PROJECT_NAME})
else()
    target_link_libraries(
        ${PROJECT_NAME}
        PUBLIC
            GL
    )
    install(TARGETS ${PROJECT_NAME})
endif()