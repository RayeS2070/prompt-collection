#ifndef PROMPT_COLLECTION_MODEL_PROMPT_HPP
#define PROMPT_COLLECTION_MODEL_PROMPT_HPP

#include <cstdint>
#include <string>

namespace model {

// SQLite rowid. 0 is never one: a prompt with id 0 is not stored yet.
using PromptId = std::int64_t;

struct Prompt {
  PromptId id = 0;
  // {}: designated initializers may skip these without GCC's
  // -Wmissing-field-initializers.
  std::string title{};
  std::string positive{};
  std::string negative{};

  auto operator==(const Prompt&) const -> bool = default;
};

}  // namespace model

#endif  // PROMPT_COLLECTION_MODEL_PROMPT_HPP
