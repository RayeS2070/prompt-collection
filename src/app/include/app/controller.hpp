#ifndef PROMPT_COLLECTION_APP_CONTROLLER_HPP
#define PROMPT_COLLECTION_APP_CONTROLLER_HPP

#include <optional>

#include "app/action.hpp"
#include "app/state.hpp"
#include "model/prompt_store.hpp"

namespace app {

// Applies the actions to the store and keeps the state in step with it.
class Controller {
 public:
  // Loads the prompts at once; a failure shows up in state().error.
  explicit Controller(model::PromptStore store);

  auto state() const -> const AppState& { return state_; }

  auto apply(const Action& action) -> void;

 private:
  auto handle(const action::SetFilter& action) -> void;
  auto handle(const action::NewPrompt& action) -> void;
  auto handle(const action::Select& action) -> void;
  auto handle(const action::Save& action) -> void;
  auto handle(const action::Delete& action) -> void;

  // Queries the prompts matching the filter again.
  auto reload() -> void;
  auto openEditor(std::optional<model::Prompt> prompt) -> void;

  model::PromptStore store_;
  AppState state_;
};

}  // namespace app

#endif  // PROMPT_COLLECTION_APP_CONTROLLER_HPP
