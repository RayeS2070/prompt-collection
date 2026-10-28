# The controller: the application state and the actions that change it. No
# UI: the view only reads the state and adds actions. Headers are included
# as "app/<name>.hpp".
add_library(prompt_collection_app)
target_sources(prompt_collection_app
        PUBLIC
        FILE_SET HEADERS
        BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
        FILES
        include/app/action.hpp
        include/app/state.hpp
        include/app/controller.hpp
        PRIVATE
        src/controller.cpp
)
target_compile_features(prompt_collection_app PUBLIC cxx_std_26)
target_link_libraries(prompt_collection_app
        PUBLIC
        prompt_collection_model
        PRIVATE
        prompt_collection_warnings
)
