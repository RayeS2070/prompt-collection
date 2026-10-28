#include <SDL3/SDL.h>
// Once, here: SDL's own entry point, which hands main UTF-8 arguments on
// Windows too.
#include <SDL3/SDL_main.h>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlrenderer3.h>

#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <expected>
#include <format>
#include <memory>
#include <print>
#include <span>
#include <string>
#include <utility>
#include <vector>

#include "app/action.hpp"
#include "app/controller.hpp"
#include "model/prompt_store.hpp"
#include "view.hpp"

namespace {

constexpr int kWidth = 1100;
constexpr int kHeight = 700;

// A unique_ptr deleter that calls an SDL destroy function.
template <auto destroy>
struct Destroy {
  auto operator()(auto* handle) const -> void { destroy(handle); }
};
using Window = std::unique_ptr<SDL_Window, Destroy<SDL_DestroyWindow>>;
using Renderer = std::unique_ptr<SDL_Renderer, Destroy<SDL_DestroyRenderer>>;

// SDL_Quit on every way out of main once SDL_Init was called, failed or not:
// SDL_Quit is safe after a failed SDL_Init.
class SdlQuit {
 public:
  SdlQuit() = default;
  SdlQuit(const SdlQuit&) = delete;
  auto operator=(const SdlQuit&) -> SdlQuit& = delete;
  ~SdlQuit() { SDL_Quit(); }
};

// The ImGui context with its SDL3 backends, shut down in reverse order.
// The window and the renderer must outlive it.
class ImGuiSession {
 public:
  ImGuiSession(SDL_Window* window, SDL_Renderer* renderer, float scale) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(scale);
    style.FontScaleDpi = scale;
    // Both always return true in ImGui 1.93.
    ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer3_Init(renderer);
  }
  ImGuiSession(const ImGuiSession&) = delete;
  auto operator=(const ImGuiSession&) -> ImGuiSession& = delete;
  ~ImGuiSession() {
    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
  }
};

// prompts.db in the per-user data directory of the platform, which SDL
// creates: ~/.local/share/pcollect on Linux, %APPDATA%\pcollect on Windows,
// ~/Library/Application Support/pcollect on macOS. UTF-8, as SQLite wants
// it; std::filesystem::path would go through the ANSI code page on Windows.
auto defaultDatabasePath() -> std::expected<std::string, std::string> {
  const std::unique_ptr<char, Destroy<SDL_free>> directory(
      SDL_GetPrefPath(nullptr, "pcollect"));
  if (!directory) {
    return std::unexpected(std::format("SDL_GetPrefPath: {}", SDL_GetError()));
  }
  return std::string(directory.get()) + "prompts.db";
}

// Draws one frame and returns what the user asked for in it.
auto drawFrame(SDL_Renderer* renderer, const app::AppState& state,
               view::Drafts& drafts) -> std::vector<app::Action> {
  ImGui_ImplSDLRenderer3_NewFrame();
  ImGui_ImplSDL3_NewFrame();
  ImGui::NewFrame();
  std::vector<app::Action> actions;
  view::drawMainWindow(state, drafts, actions);
  ImGui::Render();

  const ImGuiIO& io = ImGui::GetIO();
  SDL_SetRenderScale(renderer, io.DisplayFramebufferScale.x,
                     io.DisplayFramebufferScale.y);
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
  SDL_RenderClear(renderer);
  ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);
  SDL_RenderPresent(renderer);
  return actions;
}

}  // namespace

// Usage: prompt_collection [DATABASE]
auto main(int argc, char** argv) -> int {
  const std::span args(argv, static_cast<std::size_t>(argc));
  const auto path = args.size() > 1
                        ? std::expected<std::string, std::string>(args[1])
                        : defaultDatabasePath();
  if (!path) {
    std::println(stderr, "{}", path.error());
    return EXIT_FAILURE;
  }
  auto store = model::PromptStore::open(*path);
  if (!store) {
    std::println(stderr, "{}", store.error());
    return EXIT_FAILURE;
  }
  app::Controller controller(std::move(*store));

  const SdlQuit sdlQuit;
  if (!SDL_Init(SDL_INIT_VIDEO)) {
    std::println(stderr, "SDL_Init: {}", SDL_GetError());
    return EXIT_FAILURE;
  }
  const float scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
  const Window window(
      SDL_CreateWindow("Prompt collection", static_cast<int>(kWidth * scale),
                       static_cast<int>(kHeight * scale),
                       SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY));
  if (!window) {
    std::println(stderr, "SDL_CreateWindow: {}", SDL_GetError());
    return EXIT_FAILURE;
  }
  const Renderer renderer(SDL_CreateRenderer(window.get(), nullptr));
  if (!renderer) {
    std::println(stderr, "SDL_CreateRenderer: {}", SDL_GetError());
    return EXIT_FAILURE;
  }
  SDL_SetRenderVSync(renderer.get(), 1);
  const ImGuiSession imgui(window.get(), renderer.get(), scale);

  view::Drafts drafts;
  bool done = false;
  while (!done) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      ImGui_ImplSDL3_ProcessEvent(&event);
      if (event.type == SDL_EVENT_QUIT) done = true;
    }
    if ((SDL_GetWindowFlags(window.get()) & SDL_WINDOW_MINIMIZED) != 0) {
      SDL_Delay(10);
      continue;
    }
    // After the frame: the view no longer holds references into the state.
    for (const app::Action& action :
         drawFrame(renderer.get(), controller.state(), drafts)) {
      controller.apply(action);
    }
  }
  return EXIT_SUCCESS;
}
