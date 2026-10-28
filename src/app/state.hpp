#ifndef PROMPT_COLLECTION_SRC_APP_STATE_HPP
#define PROMPT_COLLECTION_SRC_APP_STATE_HPP

#include <optional>
#include <string>
#include <vector>

#include "model/prompt.hpp"

namespace app {

// Everything the view draws. Only the controller changes it; the view reads
// it every frame, so it holds the results of queries, not the queries.
struct AppState {
  std::string filter;
  // The prompts matching the filter, by title.
  std::vector<model::Prompt> prompts;
  // The prompt in the editor, id 0 when not stored yet; none when the editor
  // is closed.
  std::optional<model::Prompt> editing;
  // Grows whenever `editing` is replaced, so the view knows to reload what
  // the user types into from it.
  int editorRevision = 0;
  // Why the last action failed; empty when it succeeded.
  std::string error;
};

}  // namespace app

#endif  // PROMPT_COLLECTION_SRC_APP_STATE_HPP
