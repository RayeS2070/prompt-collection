#ifndef PROMPT_COLLECTION_UI_VIEW_HPP
#define PROMPT_COLLECTION_UI_VIEW_HPP

#include <string>
#include <vector>

#include "app/action.hpp"
#include "app/state.hpp"
#include "model/prompt.hpp"

namespace view {

// What the user is typing and has not turned into an action yet.
// ImGui::InputText writes into it every keystroke, so it cannot live in the
// const AppState.
struct Drafts {
  std::string filter;
  model::Prompt prompt;
  // The AppState::editorRevision that `prompt` was loaded from.
  int revision = -1;
};

// Draws the whole window for one frame and adds what the user asked for to
// the actions.
auto drawMainWindow(const app::AppState& state, Drafts& drafts,
                    std::vector<app::Action>& actions) -> void;

}  // namespace view

#endif  // PROMPT_COLLECTION_UI_VIEW_HPP
