CPMAddPackage(URI "gh:google/googletest#v1.17.0"
        OPTIONS
        "INSTALL_GTEST OFF"
        "BUILD_GMOCK OFF"
)

add_executable(prompt_collection_tests
        prompt_store_test.cpp
        controller_test.cpp
)
target_link_libraries(prompt_collection_tests
        PRIVATE
        prompt_collection_warnings
        prompt_collection_app
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(prompt_collection_tests)
