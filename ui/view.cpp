#include "view.hpp"

#include <imgui.h>
#include <imgui_stdlib.h>

#include <cfloat>
#include <format>
#include <vector>

#include "app/action.hpp"
#include "app/state.hpp"
#include "model/prompt.hpp"

namespace view {

namespace {

// Sizes in font heights, so they follow the DPI scale.
constexpr float kListWidth = 18.0f;
constexpr float kPartLines = 8.0f;

constexpr ImVec4 kErrorColor{1.0f, 0.45f, 0.45f, 1.0f};

auto drawList(const app::AppState& state, Drafts& drafts,
              std::vector<app::Action>& actions) -> void {
  ImGui::SetNextItemWidth(-FLT_MIN);
  if (ImGui::InputTextWithHint("##filter", "Filter", &drafts.filter)) {
    actions.push_back(app::action::SetFilter{drafts.filter});
  }
  if (ImGui::Button("New prompt", ImVec2(-FLT_MIN, 0))) {
    actions.push_back(app::action::NewPrompt{});
  }
  ImGui::Separator();

  ImGui::BeginChild("prompts");
  for (const model::Prompt& prompt : state.prompts) {
    const bool selected = state.editing && state.editing->id == prompt.id;
    // After ##: part of the ImGui id, not shown. Keeps equal titles apart.
    const std::string label = std::format("{}##{}", prompt.title, prompt.id);
    if (ImGui::Selectable(label.c_str(), selected)) {
      actions.push_back(app::action::Select{prompt.id});
    }
  }
  ImGui::EndChild();
}

// A multiline field for a prompt part, with a button that copies it.
auto drawPart(const char* name, std::string& text) -> void {
  ImGui::SeparatorText(name);
  ImGui::PushID(name);
  ImGui::InputTextMultiline(
      "##text", &text,
      ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * kPartLines));
  if (ImGui::SmallButton("Copy")) ImGui::SetClipboardText(text.c_str());
  ImGui::PopID();
}

auto drawEditor(const app::AppState& state, Drafts& drafts,
                std::vector<app::Action>& actions) -> void {
  if (!state.editing) {
    ImGui::TextDisabled("Select a prompt or create a new one.");
    return;
  }
  // The controller opened another prompt or saved this one: what was typed
  // gives way to it.
  if (drafts.revision != state.editorRevision) {
    drafts.prompt = *state.editing;
    drafts.revision = state.editorRevision;
  }

  ImGui::SetNextItemWidth(-FLT_MIN);
  ImGui::InputTextWithHint("##title", "Title", &drafts.prompt.title);
  drawPart("Positive", drafts.prompt.positive);
  drawPart("Negative", drafts.prompt.negative);
  ImGui::Separator();

  const bool changed = drafts.prompt != *state.editing;
  ImGui::BeginDisabled(!changed);
  if (ImGui::Button("Save") || ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_S,
                                               ImGuiInputFlags_RouteGlobal)) {
    actions.push_back(app::action::Save{drafts.prompt});
  }
  ImGui::EndDisabled();
  if (drafts.prompt.id != 0) {
    ImGui::SameLine();
    if (ImGui::Button("Delete")) {
      actions.push_back(app::action::Delete{drafts.prompt.id});
    }
  }
  if (changed) {
    ImGui::SameLine();
    ImGui::TextDisabled("unsaved");
  }
}

}  // namespace

auto drawMainWindow(const app::AppState& state, Drafts& drafts,
                    std::vector<app::Action>& actions) -> void {
  // One window over the whole SDL window, without a title bar of its own.
  const ImGuiViewport* viewport = ImGui::GetMainViewport();
  ImGui::SetNextWindowPos(viewport->WorkPos);
  ImGui::SetNextWindowSize(viewport->WorkSize);
  ImGui::Begin("Prompts", nullptr,
               ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
                   ImGuiWindowFlags_NoSavedSettings |
                   ImGuiWindowFlags_NoBringToFrontOnFocus);

  ImGui::BeginChild("list", ImVec2(ImGui::GetFontSize() * kListWidth, 0),
                    ImGuiChildFlags_ResizeX | ImGuiChildFlags_Borders);
  drawList(state, drafts, actions);
  ImGui::EndChild();

  ImGui::SameLine();
  ImGui::BeginChild("editor");
  if (!state.error.empty()) {
    ImGui::TextColored(kErrorColor, "%s", state.error.c_str());
  }
  drawEditor(state, drafts, actions);
  ImGui::EndChild();

  ImGui::End();
}

}  // namespace view
