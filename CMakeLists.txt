cmake_minimum_required(VERSION 3.30 FATAL_ERROR)

project(prompts-collector LANGUAGES C CXX)

set(CMAKE_CXX_EXTENSIONS OFF)
# No C++ modules: clangd crashes on them. Without scanning, no dependency
# scanner runs (emscan-deps for the browser).
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

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

include(GNUInstallDirs)
include(cmake/CPM.cmake)

# Only the Boost libraries we use are configured:
# the full superproject has dozens.
# json reads data/asteroids.json and writes the --json answers.
set(BOOST_INCLUDE_LIBRARIES program_options json)
# Boost.Container (under Boost.JSON) header-only: its compiled part has pool
# resources JSON does not use, and one of them does not build for the
# browser. Without threads (wasm without -pthread) Boost.Config gives no
# BOOST_HAS_PTHREADS and container/detail/thread_mutex.hpp falls to its Win32
# branch.
CPMAddPackage(URI "gh:boostorg/boost#boost-1.92.0"
        OPTIONS
        "BOOST_CONTAINER_HEADER_ONLY ON"
)
CPMAddPackage(URI "gh:gabime/spdlog#v1.17.0"
        OPTIONS
        "SPDLOG_USE_STD_FORMAT ON"
        "SPDLOG_NO_TLS ON"
)

# Warnings for our own targets only; the dependencies keep their own flags.
add_library(prompt_collection_warnings INTERFACE)
target_compile_options(prompt_collection_warnings INTERFACE
        $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -pedantic>
)

# The command line tool needs a terminal: not for the browser.
if(NOT EMSCRIPTEN)
        add_executable(prompt_collection src/main.cpp src/json.hpp src/json.cpp)
        target_compile_features(prompt_collection PRIVATE cxx_std_26)
        target_link_libraries(prompt_collection
                PRIVATE
                prompt_collection_warnings
                prompt_collection_reader
                prompt_collection_report
                Boost::program_options
                Boost::json
                spdlog::spdlog
        )
        install(TARGETS prompt_collection
                RUNTIME DESTINATION
                ${CMAKE_INSTALL_BINDIR}
        )
endif()

add_subdirectory(ui)

# The tests run natively: the browser build has no test runner yet.
option(prompt_collection_BUILD_TESTS "Build the unit tests"
        ${PROJECT_IS_TOP_LEVEL}
)
if(prompt_collection_BUILD_TESTS AND NOT EMSCRIPTEN)
        enable_testing()
        add_subdirectory(tests)
endif()

# The data is embedded in the programs: nothing to install besides them and
# the documents. The license is the repository's, one level up.
install(FILES README.md ${PROJECT_SOURCE_DIR}/../LICENSE
        DESTINATION ${CMAKE_INSTALL_DOCDIR}
)
