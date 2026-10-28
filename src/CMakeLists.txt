# SQLite from the system: one C library that every distribution packages.
find_package(SQLite3 REQUIRED)
# Newer CMake renamed the target and warns on the old name, which is all
# that CMake 3.30 has.
if(TARGET SQLite3::SQLite3)
        set(prompt_collection_sqlite SQLite3::SQLite3)
else()
        set(prompt_collection_sqlite SQLite::SQLite3)
endif()

# Model: the prompts and their storage. Knows nothing of the UI.
add_library(prompt_collection_model STATIC
        model/prompt.hpp
        model/prompt_store.hpp
        model/prompt_store.cpp
)
target_include_directories(prompt_collection_model PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)
target_compile_features(prompt_collection_model PUBLIC cxx_std_26)
target_link_libraries(prompt_collection_model
        PRIVATE
        prompt_collection_warnings
        ${prompt_collection_sqlite}
)

# Controller: the application state and the actions that change it.
add_library(prompt_collection_app STATIC
        app/action.hpp
        app/state.hpp
        app/controller.hpp
        app/controller.cpp
)
target_link_libraries(prompt_collection_app
        PUBLIC
        prompt_collection_model
        PRIVATE
        prompt_collection_warnings
)
