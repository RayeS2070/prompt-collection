#include <SDL3/SDL.h>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlrenderer3.h>

#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <print>
#include <system_error>
#include <vector>

#include "app/action.hpp"
#include "app/controller.hpp"
#include "model/prompt_store.hpp"
#include "view.hpp"

namespace {

constexpr int kWidth = 1100;
constexpr int kHeight = 700;

// $XDG_DATA_HOME/pcollect/prompts.db, by default under ~/.local/share.
auto defaultDatabasePath() -> std::filesystem::path {
  std::filesystem::path base;
  if (const char* data = std::getenv("XDG_DATA_HOME"); data && *data) {
    base = data;
  } else if (const char* home = std::getenv("HOME"); home && *home) {
    base = std::filesystem::path(home) / ".local" / "share";
  } else {
    base = std::filesystem::current_path();
  }
  return base / "pcollect" / "prompts.db";
}

}  // namespace

// Usage: prompt_collection [DATABASE]
auto main(int argc, char** argv) -> int {
  const std::filesystem::path path =
      argc > 1 ? std::filesystem::path(argv[1]) : defaultDatabasePath();
  std::error_code error;
  if (path.has_parent_path()) {
    std::filesystem::create_directories(path.parent_path(), error);
  }
  if (error) {
    std::println(stderr, "cannot create {}: {}", path.parent_path().string(),
                 error.message());
    return EXIT_FAILURE;
  }
  auto store = model::PromptStore::open(path.string());
  if (!store) {
    std::println(stderr, "{}", store.error());
    return EXIT_FAILURE;
  }
  app::Controller controller(std::move(*store));

  if (!SDL_Init(SDL_INIT_VIDEO)) {
    std::println(stderr, "SDL_Init: {}", SDL_GetError());
    return EXIT_FAILURE;
  }
  const float scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
  SDL_Window* window =
      SDL_CreateWindow("Prompt collection", static_cast<int>(kWidth * scale),
                       static_cast<int>(kHeight * scale),
                       SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY);
  if (window == nullptr) {
    std::println(stderr, "SDL_CreateWindow: {}", SDL_GetError());
    return EXIT_FAILURE;
  }
  SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr);
  if (renderer == nullptr) {
    std::println(stderr, "SDL_CreateRenderer: {}", SDL_GetError());
    return EXIT_FAILURE;
  }
  SDL_SetRenderVSync(renderer, 1);

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  ImGui::StyleColorsDark();
  ImGuiStyle& style = ImGui::GetStyle();
  style.ScaleAllSizes(scale);
  style.FontScaleDpi = scale;
  ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
  ImGui_ImplSDLRenderer3_Init(renderer);

  view::Drafts drafts;
  std::vector<app::Action> actions;
  bool done = false;
  while (!done) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      ImGui_ImplSDL3_ProcessEvent(&event);
      if (event.type == SDL_EVENT_QUIT) done = true;
    }
    if ((SDL_GetWindowFlags(window) & SDL_WINDOW_MINIMIZED) != 0) {
      SDL_Delay(10);
      continue;
    }

    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();
    view::drawMainWindow(controller.state(), drafts, actions);
    ImGui::Render();

    SDL_SetRenderScale(renderer, io.DisplayFramebufferScale.x,
                       io.DisplayFramebufferScale.y);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(renderer);
    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);
    SDL_RenderPresent(renderer);

    // After the frame: the view no longer holds references into the state.
    for (const app::Action& action : actions) controller.apply(action);
    actions.clear();
  }

  ImGui_ImplSDLRenderer3_Shutdown();
  ImGui_ImplSDL3_Shutdown();
  ImGui::DestroyContext();
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
  return EXIT_SUCCESS;
}
