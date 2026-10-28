CPMAddPackage(URI "gh:google/googletest#v1.17.0"
        OPTIONS
        "INSTALL_GTEST OFF"
        "BUILD_GMOCK OFF"
)

# The model and the controller, without the UI.
add_executable(prompt_collection_tests)
target_sources(prompt_collection_tests
        PRIVATE
        prompt_store_test.cpp
        controller_test.cpp
)
target_compile_features(prompt_collection_tests PRIVATE cxx_std_26)
target_link_libraries(prompt_collection_tests
        PRIVATE
        prompt_collection_warnings
        prompt_collection_app
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(prompt_collection_tests)
