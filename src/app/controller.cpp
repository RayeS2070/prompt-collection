#include "app/controller.hpp"

#include <algorithm>
#include <format>
#include <optional>
#include <utility>
#include <variant>

#include "app/action.hpp"
#include "model/prompt.hpp"
#include "model/prompt_store.hpp"

namespace app {

Controller::Controller(model::PromptStore store) : store_(std::move(store)) {
  reload();
}

auto Controller::apply(const Action& action) -> void {
  state_.error.clear();
  std::visit([this](const auto& a) { handle(a); }, action);
}

auto Controller::handle(const action::SetFilter& action) -> void {
  state_.filter = action.text;
  reload();
}

auto Controller::handle(const action::NewPrompt& /*action*/) -> void {
  openEditor(model::Prompt{});
}

auto Controller::handle(const action::Select& action) -> void {
  const auto it =
      std::ranges::find(state_.prompts, action.id, &model::Prompt::id);
  if (it == state_.prompts.end()) {
    state_.error = std::format("no prompt {}", action.id);
    return;
  }
  openEditor(*it);
}

auto Controller::handle(const action::Save& action) -> void {
  model::Prompt prompt = action.prompt;
  if (prompt.title.empty()) {
    state_.error = "the title is empty";
    return;
  }
  if (prompt.id == 0) {
    const auto id = store_.insert(prompt);
    if (!id) {
      state_.error = id.error();
      return;
    }
    prompt.id = *id;
  } else if (const auto done = store_.update(prompt); !done) {
    state_.error = done.error();
    return;
  }
  openEditor(std::move(prompt));
  reload();
}

auto Controller::handle(const action::Delete& action) -> void {
  if (const auto done = store_.remove(action.id); !done) {
    state_.error = done.error();
    return;
  }
  if (state_.editing && state_.editing->id == action.id) {
    openEditor(std::nullopt);
  }
  reload();
}

auto Controller::reload() -> void {
  auto prompts = store_.find(state_.filter);
  if (!prompts) {
    state_.error = prompts.error();
    return;
  }
  state_.prompts = std::move(*prompts);
}

auto Controller::openEditor(std::optional<model::Prompt> prompt) -> void {
  state_.editing = std::move(prompt);
  ++state_.editorRevision;
}

}  // namespace app
