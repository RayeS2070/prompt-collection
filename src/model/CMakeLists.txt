# SQLite from the system: one C library that every distribution packages.
find_package(SQLite3 REQUIRED)
# Newer CMake renamed the target and warns on the old name, which is all
# that CMake 3.30 has.
if(TARGET SQLite3::SQLite3)
        set(prompt_collection_sqlite SQLite3::SQLite3)
else()
        set(prompt_collection_sqlite SQLite::SQLite3)
endif()

# The prompts and their storage: no UI. Headers are included as
# "model/<name>.hpp".
add_library(prompt_collection_model)
target_sources(prompt_collection_model
        PUBLIC
        FILE_SET HEADERS
        BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
        FILES
        include/model/prompt.hpp
        include/model/prompt_store.hpp
        PRIVATE
        src/prompt_store.cpp
)
target_compile_features(prompt_collection_model PUBLIC cxx_std_26)
target_link_libraries(prompt_collection_model
        PRIVATE
        prompt_collection_warnings
        ${prompt_collection_sqlite}
)
