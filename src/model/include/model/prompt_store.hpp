#ifndef PROMPT_COLLECTION_MODEL_PROMPT_STORE_HPP
#define PROMPT_COLLECTION_MODEL_PROMPT_STORE_HPP

#include <expected>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "model/prompt.hpp"

struct sqlite3;

namespace model {

// The prompts in an SQLite database. Errors are SQLite's messages.
// One thread at a time: the connection is not shared.
class PromptStore {
 public:
  // Creates the file and the schema when missing. ":memory:" gives a
  // database that lives as long as the store.
  static auto open(const std::string& path)
      -> std::expected<PromptStore, std::string>;

  // The prompts with the text in the title, positive or negative part,
  // ignoring ASCII letter case; all of them for an empty text. By title.
  auto find(std::string_view text) const
      -> std::expected<std::vector<Prompt>, std::string>;

  // Stores a new prompt; its id is ignored. Returns the id it got.
  auto insert(const Prompt& prompt) -> std::expected<PromptId, std::string>;

  // Overwrites the stored prompt with the same id.
  auto update(const Prompt& prompt) -> std::expected<void, std::string>;

  auto remove(PromptId id) -> std::expected<void, std::string>;

 private:
  struct Close {
    auto operator()(sqlite3* db) const -> void;
  };

  explicit PromptStore(sqlite3* db);

  std::unique_ptr<sqlite3, Close> db_;
};

}  // namespace model

#endif  // PROMPT_COLLECTION_MODEL_PROMPT_STORE_HPP
