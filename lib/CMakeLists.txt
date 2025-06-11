include(ExternalProject)

# MSYS2 note:
# set up ACLOCAL_PATH=/usr/share/aclocal before running GNU Lightning install as otherwise it will try to use Windows paths and break.

# Build master branch of GNU Lightning locally.
if(DEFINED ENABLE_JIT_LIGHTNING)
if(DEFINED ENABLE_BUILD_LIGHTNING)
    # Note for other platforms (must have pthread so PSVita is out):
    # CONFIGURE_COMMAND <SOURCE_DIR>/configure --host=<gcc_host_here> --disable-shared --disable-disassembler
    ExternalProject_Add(
        lightning_external
        URL https://ftp.gnu.org/gnu/lightning/lightning-2.2.3.tar.gz
        URL_HASH SHA256=c045c7a33a00affbfeb11066fa502c03992e474a62ba95977aad06dbc14c6829
        CONFIGURE_COMMAND <SOURCE_DIR>/configure --disable-shared --disable-disassembler
        BUILD_COMMAND make
        BUILD_IN_SOURCE 1
        INSTALL_COMMAND ""  # Skip installation
        BUILD_BYPRODUCTS "<SOURCE_DIR>/liblightning.a"
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )

    # Set paths for imported target
    ExternalProject_Get_Property(lightning_external SOURCE_DIR)
    set(LIGHTNING_INCLUDE_DIR "${SOURCE_DIR}")
    set(LIGHTNING_LIBRARY "${SOURCE_DIR}/lib/.libs/liblightning.a")

    # Create imported static library target
    add_library(lightning STATIC IMPORTED)
    set_target_properties(lightning PROPERTIES
        IMPORTED_LOCATION "${LIGHTNING_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "$<BUILD_INTERFACE:${LIGHTNING_INCLUDE_DIR}>"
    )
    add_dependencies(lightning lightning_external)
endif()
endif()

add_subdirectory(auxum)
add_subdirectory(cbf)
add_subdirectory(cchip8)
add_subdirectory(clay_sdl3)