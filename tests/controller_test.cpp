#include "app/controller.hpp"

#include <gtest/gtest.h>

#include "app/action.hpp"
#include "model/prompt.hpp"
#include "model/prompt_store.hpp"

namespace {

namespace action = app::action;

auto makeController() -> app::Controller {
  auto store = model::PromptStore::open(":memory:");
  EXPECT_TRUE(store) << store.error();
  return app::Controller(std::move(*store));
}

TEST(Controller, SavesNewPromptAndKeepsItOpen) {
  auto controller = makeController();
  controller.apply(action::NewPrompt{});
  ASSERT_TRUE(controller.state().editing);
  EXPECT_EQ(controller.state().editing->id, 0);

  controller.apply(action::Save{{.title = "cat", .positive = "1girl"}});
  const app::AppState& state = controller.state();
  EXPECT_TRUE(state.error.empty()) << state.error;
  ASSERT_EQ(state.prompts.size(), 1);
  EXPECT_NE(state.prompts.front().id, 0);
  ASSERT_TRUE(state.editing);
  EXPECT_EQ(*state.editing, state.prompts.front());
}

TEST(Controller, RefusesEmptyTitle) {
  auto controller = makeController();
  controller.apply(action::Save{{.title = ""}});
  EXPECT_FALSE(controller.state().error.empty());
  EXPECT_TRUE(controller.state().prompts.empty());
}

TEST(Controller, ErrorLastsUntilNextAction) {
  auto controller = makeController();
  controller.apply(action::Save{{.title = ""}});
  ASSERT_FALSE(controller.state().error.empty());
  controller.apply(action::NewPrompt{});
  EXPECT_TRUE(controller.state().error.empty());
}

TEST(Controller, UpdatesExistingPrompt) {
  auto controller = makeController();
  controller.apply(action::Save{{.title = "cat"}});
  model::Prompt prompt = controller.state().prompts.front();
  prompt.negative = "lowres";
  controller.apply(action::Save{prompt});

  ASSERT_EQ(controller.state().prompts.size(), 1);
  EXPECT_EQ(controller.state().prompts.front(), prompt);
}

TEST(Controller, FilterNarrowsTheList) {
  auto controller = makeController();
  controller.apply(action::Save{{.title = "cat"}});
  controller.apply(action::Save{{.title = "dog"}});
  controller.apply(action::SetFilter{"do"});

  ASSERT_EQ(controller.state().prompts.size(), 1);
  EXPECT_EQ(controller.state().prompts.front().title, "dog");
}

TEST(Controller, SelectOpensPromptAndBumpsRevision) {
  auto controller = makeController();
  controller.apply(action::Save{{.title = "cat"}});
  controller.apply(action::Save{{.title = "dog"}});
  const int revision = controller.state().editorRevision;
  const model::Prompt cat = controller.state().prompts.front();

  controller.apply(action::Select{cat.id});
  ASSERT_TRUE(controller.state().editing);
  EXPECT_EQ(*controller.state().editing, cat);
  EXPECT_GT(controller.state().editorRevision, revision);
}

TEST(Controller, DeletingOpenPromptClosesEditor) {
  auto controller = makeController();
  controller.apply(action::Save{{.title = "cat"}});
  const model::PromptId id = controller.state().editing->id;

  controller.apply(action::Delete{id});
  EXPECT_TRUE(controller.state().error.empty()) << controller.state().error;
  EXPECT_FALSE(controller.state().editing);
  EXPECT_TRUE(controller.state().prompts.empty());
}

}  // namespace
