#pragma once
#include "taskflow/repository.hpp"
#include <stdexcept>
#include <string>

namespace taskflow {

// заглушка под SQLite (ЛР2)
class SQLiteTaskRepository final : public ITaskRepository {
public:
    explicit SQLiteTaskRepository(std::string db) : db_(std::move(db)) {}

    Task create(Task) override { die(); }
    std::optional<Task> find_by_id(std::int64_t) const override { die(); }
    std::vector<Task> list() const override { die(); }
    bool update(const Task&) override { die(); }

    const std::string& db() const { return db_; }

private:
    std::string db_;
    [[noreturn]] void die() const {
        throw std::logic_error("SQLite пока не подключён, файл: " + db_);
    }
};

} // namespace taskflow
