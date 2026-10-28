# SQLite from its amalgamation: one C file, built with the options we pick
# rather than a distribution's. sqlite.org publishes no hash for the zip; this
# one was taken after checking sqlite3.c against the SHA3-256 in the release
# log of 3.53.4.
set(prompt_collection_sqlite_sha3
        628a44cfe82c66aed1ccbbe85a562d2e33ebe64b3288981ed76285612227934e
)
CPMAddPackage(
        NAME sqlite3
        VERSION 3.53.4
        URL "https://www.sqlite.org/2026/sqlite-amalgamation-3530400.zip"
        URL_HASH SHA3_256=${prompt_collection_sqlite_sha3}
        DOWNLOAD_ONLY YES
)
find_package(Threads REQUIRED)
add_library(sqlite3)
target_sources(sqlite3
        PUBLIC
        FILE_SET HEADERS
        BASE_DIRS ${sqlite3_SOURCE_DIR}
        FILES
        ${sqlite3_SOURCE_DIR}/sqlite3.h
        PRIVATE
        ${sqlite3_SOURCE_DIR}/sqlite3.c
)
target_compile_definitions(sqlite3
        PRIVATE
        # Full-text search, for searching prompts and tags.
        SQLITE_ENABLE_FTS5
        # Foreign keys enforced without a PRAGMA on every connection.
        SQLITE_DEFAULT_FOREIGN_KEYS=1
        # "text" is an identifier only, never a string by mistake.
        SQLITE_DQS=0
        # No extensions loaded from files: nothing needs them, and no libdl.
        SQLITE_OMIT_LOAD_EXTENSION
)
target_link_libraries(sqlite3
        PRIVATE
        Threads::Threads
        # FTS5 ranking calls log().
        $<$<PLATFORM_ID:Linux>:m>
)

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
        sqlite3
)
