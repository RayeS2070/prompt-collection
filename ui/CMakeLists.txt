# SDL3 for the window and input, drawn with its own 2D renderer: no OpenGL
# loader and no shaders to ship.
CPMAddPackage(URI "gh:libsdl-org/SDL#release-3.4.18"
        OPTIONS
        "SDL_SHARED OFF"
        "SDL_STATIC ON"
        "SDL_TEST_LIBRARY OFF"
        "SDL_EXAMPLES OFF"
)

# Dear ImGui has no CMake build of its own: its sources are compiled here,
# with the SDL3 backends and the std::string overloads of InputText.
CPMAddPackage(URI "gh:ocornut/imgui#v1.93.0" DOWNLOAD_ONLY YES)
add_library(imgui STATIC
        ${imgui_SOURCE_DIR}/imgui.cpp
        ${imgui_SOURCE_DIR}/imgui_draw.cpp
        ${imgui_SOURCE_DIR}/imgui_tables.cpp
        ${imgui_SOURCE_DIR}/imgui_widgets.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl3.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_sdlrenderer3.cpp
        ${imgui_SOURCE_DIR}/misc/cpp/imgui_stdlib.cpp
)
# SYSTEM: our warnings do not reach into the ImGui headers.
target_include_directories(imgui SYSTEM PUBLIC
        ${imgui_SOURCE_DIR}
        ${imgui_SOURCE_DIR}/backends
        ${imgui_SOURCE_DIR}/misc/cpp
)
target_link_libraries(imgui PUBLIC SDL3::SDL3)

# View and the main loop.
add_executable(prompt_collection
        main.cpp
        view.hpp
        view.cpp
)
target_link_libraries(prompt_collection
        PRIVATE
        prompt_collection_warnings
        prompt_collection_app
        imgui
)
install(TARGETS prompt_collection
        RUNTIME DESTINATION
        ${CMAKE_INSTALL_BINDIR}
)
