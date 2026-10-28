#include "model/prompt_store.hpp"

#include <sqlite3.h>

#include <expected>
#include <format>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "model/prompt.hpp"

namespace model {

namespace {

constexpr const char* kSchema = R"sql(
CREATE TABLE IF NOT EXISTS prompts (
  id       INTEGER PRIMARY KEY,
  title    TEXT NOT NULL,
  positive TEXT NOT NULL,
  negative TEXT NOT NULL
);
)sql";

struct Finalize {
  auto operator()(sqlite3_stmt* statement) const -> void {
    sqlite3_finalize(statement);
  }
};
using Statement = std::unique_ptr<sqlite3_stmt, Finalize>;

auto prepare(sqlite3* db, std::string_view sql)
    -> std::expected<Statement, std::string> {
  sqlite3_stmt* statement = nullptr;
  if (sqlite3_prepare_v2(db, sql.data(), static_cast<int>(sql.size()),
                         &statement, nullptr) != SQLITE_OK) {
    return std::unexpected(sqlite3_errmsg(db));
  }
  return Statement(statement);
}

// An empty string_view may have a null data(), which SQLite would bind as
// NULL and the NOT NULL columns would refuse.
auto bind(sqlite3_stmt* statement, int index, std::string_view text) -> int {
  return sqlite3_bind_text(statement, index, text.empty() ? "" : text.data(),
                           static_cast<int>(text.size()), SQLITE_TRANSIENT);
}

auto bind(sqlite3_stmt* statement, int index, PromptId id) -> int {
  return sqlite3_bind_int64(statement, index, id);
}

// Binds the values to ?1, ?2, ... in order.
template <typename... Values>
auto bindAll(sqlite3* db, sqlite3_stmt* statement, const Values&... values)
    -> std::expected<void, std::string> {
  int index = 0;
  const bool ok = ((bind(statement, ++index, values) == SQLITE_OK) && ...);
  if (!ok) return std::unexpected(sqlite3_errmsg(db));
  return {};
}

auto text(sqlite3_stmt* statement, int column) -> std::string {
  const unsigned char* bytes = sqlite3_column_text(statement, column);
  if (bytes == nullptr) return {};
  // Reading unsigned char bytes through char is allowed aliasing.
  return {reinterpret_cast<const char*>(bytes),
          static_cast<std::size_t>(sqlite3_column_bytes(statement, column))};
}

// Runs a statement that returns no rows, binding the values first.
template <typename... Values>
auto execute(sqlite3* db, std::string_view sql, const Values&... values)
    -> std::expected<void, std::string> {
  auto statement = prepare(db, sql);
  if (!statement) return std::unexpected(statement.error());
  if (auto bound = bindAll(db, statement->get(), values...); !bound) {
    return bound;
  }
  if (sqlite3_step(statement->get()) != SQLITE_DONE) {
    return std::unexpected(sqlite3_errmsg(db));
  }
  return {};
}

}  // namespace

auto PromptStore::Close::operator()(sqlite3* db) const -> void {
  sqlite3_close(db);
}

PromptStore::PromptStore(sqlite3* db) : db_(db) {}

auto PromptStore::open(const std::string& path)
    -> std::expected<PromptStore, std::string> {
  sqlite3* db = nullptr;
  const int code = sqlite3_open_v2(
      path.c_str(), &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr);
  // Owned at once: SQLite gives a handle to close even when opening fails.
  PromptStore store(db);
  if (code != SQLITE_OK) {
    return std::unexpected(
        std::format("cannot open {}: {}", path,
                    db != nullptr ? sqlite3_errmsg(db) : sqlite3_errstr(code)));
  }
  if (sqlite3_exec(db, kSchema, nullptr, nullptr, nullptr) != SQLITE_OK) {
    return std::unexpected(sqlite3_errmsg(db));
  }
  return store;
}

auto PromptStore::find(std::string_view text) const
    -> std::expected<std::vector<Prompt>, std::string> {
  // lower() only knows ASCII: enough for Danbooru tags, not for Cyrillic.
  constexpr std::string_view kSql = R"sql(
SELECT id, title, positive, negative FROM prompts
WHERE ?1 = ''
   OR instr(lower(title), lower(?1))
   OR instr(lower(positive), lower(?1))
   OR instr(lower(negative), lower(?1))
ORDER BY title COLLATE NOCASE, id
)sql";
  sqlite3* db = db_.get();
  auto statement = prepare(db, kSql);
  if (!statement) return std::unexpected(statement.error());
  if (auto bound = bindAll(db, statement->get(), text); !bound) {
    return std::unexpected(bound.error());
  }

  std::vector<Prompt> prompts;
  int code = SQLITE_ROW;
  while ((code = sqlite3_step(statement->get())) == SQLITE_ROW) {
    prompts.push_back({
        .id = sqlite3_column_int64(statement->get(), 0),
        .title = model::text(statement->get(), 1),
        .positive = model::text(statement->get(), 2),
        .negative = model::text(statement->get(), 3),
    });
  }
  if (code != SQLITE_DONE) return std::unexpected(sqlite3_errmsg(db));
  return prompts;
}

auto PromptStore::insert(const Prompt& prompt)
    -> std::expected<PromptId, std::string> {
  sqlite3* db = db_.get();
  auto done = execute(
      db, "INSERT INTO prompts (title, positive, negative) VALUES (?1, ?2, ?3)",
      prompt.title, prompt.positive, prompt.negative);
  if (!done) return std::unexpected(done.error());
  return sqlite3_last_insert_rowid(db);
}

auto PromptStore::update(const Prompt& prompt)
    -> std::expected<void, std::string> {
  sqlite3* db = db_.get();
  auto done =
      execute(db,
              "UPDATE prompts SET title = ?2, positive = ?3, negative = ?4 "
              "WHERE id = ?1",
              prompt.id, prompt.title, prompt.positive, prompt.negative);
  if (!done) return done;
  if (sqlite3_changes(db) == 0) {
    return std::unexpected(std::format("no prompt {}", prompt.id));
  }
  return {};
}

auto PromptStore::remove(PromptId id) -> std::expected<void, std::string> {
  sqlite3* db = db_.get();
  auto done = execute(db, "DELETE FROM prompts WHERE id = ?1", id);
  if (!done) return done;
  if (sqlite3_changes(db) == 0) {
    return std::unexpected(std::format("no prompt {}", id));
  }
  return {};
}

}  // namespace model
