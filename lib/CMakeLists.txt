include(ExternalProject)

# MSYS2 note:
# set up ACLOCAL_PATH=/usr/share/aclocal before running GNU Lightning install as otherwise it will try to use Windows paths and break.

# Build master branch of GNU Lightning locally.
if(DEFINED ENABLE_JIT_LIGHTNING)
if(DEFINED ENABLE_BUILD_LIGHTNING)
    if(NOT EXISTS ${CMAKE_BINARY_DIR}/lightning-install)
        ExternalProject_Add(
            lightning
            GIT_REPOSITORY "git://git.savannah.gnu.org/lightning.git"
            CONFIGURE_COMMAND cd ../lightning && ./bootstrap && ./configure --prefix=${CMAKE_BINARY_DIR}/lightning-install
            BUILD_COMMAND cd ../lightning && make -j$(nproc)
            INSTALL_COMMAND cd ../lightning && make install
        )
    endif()
    set(LIGHTNING_INCLUDE_DIR ${CMAKE_BINARY_DIR}/lightning-install/include)
    set(LIGHTNING_LIBRARY ${CMAKE_BINARY_DIR}/lightning-install/lib/liblightning.a)
endif()
endif()

add_subdirectory(auxum)
add_subdirectory(cbf)
add_subdirectory(cchip8)