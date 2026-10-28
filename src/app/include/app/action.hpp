#ifndef PROMPT_COLLECTION_APP_ACTION_HPP
#define PROMPT_COLLECTION_APP_ACTION_HPP

#include <string>
#include <variant>

#include "model/prompt.hpp"

// What the user asked for during a frame. The view collects them and the
// controller applies them after the frame, never while the view is still
// iterating over the state.

namespace app {

namespace action {

struct SetFilter {
  std::string text;
};

// Opens the editor on an empty prompt.
struct NewPrompt {};

// Opens the editor on a stored prompt.
struct Select {
  model::PromptId id = 0;
};

// Stores the prompt: a new one when its id is 0.
struct Save {
  model::Prompt prompt;
};

struct Delete {
  model::PromptId id = 0;
};

}  // namespace action

using Action = std::variant<action::SetFilter, action::NewPrompt,
                            action::Select, action::Save, action::Delete>;

}  // namespace app

#endif  // PROMPT_COLLECTION_APP_ACTION_HPP
