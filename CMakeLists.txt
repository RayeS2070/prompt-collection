cmake_minimum_required(VERSION 3.30 FATAL_ERROR)

project(prompts-collector LANGUAGES C CXX)

set(CMAKE_CXX_EXTENSIONS OFF)
# No C++ modules: clangd crashes on them.
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

# ccache when installed, for us and the dependencies alike. Optional: CI
# machines and fresh installs go without.
find_program(PROMPT_COLLECTION_CCACHE ccache)
if(PROMPT_COLLECTION_CCACHE)
        set(CMAKE_C_COMPILER_LAUNCHER ${PROMPT_COLLECTION_CCACHE})
        set(CMAKE_CXX_COMPILER_LAUNCHER ${PROMPT_COLLECTION_CCACHE})
endif()

# Checks below go before the dependencies, so they get them too.

# Bounds and precondition checks in the standard library for Debug builds.
# Both keep the ABI, unlike _GLIBCXX_DEBUG, so code built with and without
# them still links together.
add_compile_definitions(
        $<$<CONFIG:Debug>:_GLIBCXX_ASSERTIONS>
        $<$<CONFIG:Debug>:_LIBCPP_HARDENING_MODE=_LIBCPP_HARDENING_MODE_DEBUG>
)

option(PROMPT_COLLECTION_SANITIZE "Build with AddressSanitizer and UBSan" OFF)
if(PROMPT_COLLECTION_SANITIZE)
        add_compile_options(
                -fsanitize=address,undefined
                -fno-omit-frame-pointer
        )
        add_link_options(-fsanitize=address,undefined)
        # Lets ASan see accesses past size() but within
        # capacity() of std::vector.
        add_compile_definitions(_GLIBCXX_SANITIZE_VECTOR)
endif()

# MinGW: the C++ runtime and winpthread linked in, so the .exe runs without
# the compiler's DLLs next to it (under wine too). libstdc++ on Windows
# keeps what std::print needs for the console in libstdc++exp.
if(MINGW)
        add_link_options(-static)
        link_libraries("$<$<CXX_COMPILER_ID:GNU>:stdc++exp>")
endif()

include(GNUInstallDirs)
include(cmake/CPM.cmake)

# Warnings for our own targets only; the dependencies keep their own flags.
add_library(prompt_collection_warnings INTERFACE)
target_compile_options(prompt_collection_warnings INTERFACE
        $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -pedantic>
)

add_subdirectory(src)
add_subdirectory(ui)

option(prompt_collection_BUILD_TESTS "Build the unit tests"
        ${PROJECT_IS_TOP_LEVEL}
)
if(prompt_collection_BUILD_TESTS)
        enable_testing()
        add_subdirectory(tests)
endif()

install(FILES README.md LICENSE
        DESTINATION ${CMAKE_INSTALL_DOCDIR}
)
