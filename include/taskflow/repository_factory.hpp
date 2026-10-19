#pragma once
#include "taskflow/in_memory_task_repository.hpp"
#include "taskflow/sqlite_task_repository.hpp"
#include <memory>
#include <stdexcept>
#include <string>

namespace taskflow {

struct AppConfig {
    std::string engine{"memory"}; // memory / sqlite
    std::string database{"kudryashov.db"};
};

inline TaskRepositoryPtr make_repository(const AppConfig& cfg) {
    if (cfg.engine == "memory") {
        return std::make_shared<InMemoryTaskRepository>();
    }
    if (cfg.engine == "sqlite") {
        return std::make_shared<SQLiteTaskRepository>(cfg.database);
    }
    throw std::invalid_argument("неизвестный engine: " + cfg.engine);
}

} // namespace taskflow
