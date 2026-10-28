#include "model/prompt_store.hpp"

#include <gtest/gtest.h>

#include <vector>

#include "model/prompt.hpp"

namespace {

auto openInMemory() -> model::PromptStore {
  auto store = model::PromptStore::open(":memory:");
  EXPECT_TRUE(store) << store.error();
  return std::move(*store);
}

TEST(PromptStore, StartsEmpty) {
  const auto store = openInMemory();
  const auto prompts = store.find("");
  ASSERT_TRUE(prompts) << prompts.error();
  EXPECT_TRUE(prompts->empty());
}

TEST(PromptStore, InsertsAndFindsByTitle) {
  auto store = openInMemory();
  const auto b = store.insert({.title = "b", .positive = "x", .negative = ""});
  const auto a = store.insert({.title = "A", .positive = "y", .negative = "z"});
  ASSERT_TRUE(a && b);
  EXPECT_NE(*a, *b);

  const auto prompts = store.find("");
  ASSERT_TRUE(prompts);
  const std::vector<model::Prompt> expected = {
      {.id = *a, .title = "A", .positive = "y", .negative = "z"},
      {.id = *b, .title = "b", .positive = "x", .negative = ""},
  };
  EXPECT_EQ(*prompts, expected);
}

TEST(PromptStore, FindsTextInAnyPartIgnoringCase) {
  auto store = openInMemory();
  ASSERT_TRUE(store.insert({.title = "cat", .positive = "1girl, Solo"}));
  ASSERT_TRUE(store.insert({.title = "dog", .negative = "lowres"}));
  ASSERT_TRUE(store.insert({.title = "solo dog"}));

  const auto solo = store.find("SOLO");
  ASSERT_TRUE(solo);
  ASSERT_EQ(solo->size(), 2);
  EXPECT_EQ((*solo)[0].title, "cat");
  EXPECT_EQ((*solo)[1].title, "solo dog");

  const auto lowres = store.find("lowres");
  ASSERT_TRUE(lowres);
  ASSERT_EQ(lowres->size(), 1);
  EXPECT_EQ(lowres->front().title, "dog");
}

TEST(PromptStore, Updates) {
  auto store = openInMemory();
  const auto id = store.insert({.title = "old"});
  ASSERT_TRUE(id);
  const model::Prompt changed{.id = *id, .title = "new", .positive = "p"};
  ASSERT_TRUE(store.update(changed));

  const auto prompts = store.find("");
  ASSERT_TRUE(prompts);
  EXPECT_EQ(*prompts, std::vector{changed});
}

TEST(PromptStore, RemovesAndReportsMissing) {
  auto store = openInMemory();
  const auto id = store.insert({.title = "t"});
  ASSERT_TRUE(id);
  EXPECT_TRUE(store.remove(*id));
  EXPECT_FALSE(store.remove(*id));
  EXPECT_FALSE(store.update({.id = *id, .title = "t"}));
}

}  // namespace
