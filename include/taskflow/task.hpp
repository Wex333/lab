#pragma once
#include <cstdint>
#include <string>

namespace taskflow {

enum class TaskStatus { New, InProgress, Done, Cancelled };

// Задача заявки (вариант Kudryashov)
struct Task {
    std::int64_t id{};
    std::string title;
    std::string assignee;
    TaskStatus status{TaskStatus::New};
    int priority{3};
    std::string description; // ЛР1: текст не длиннее 1000
    std::string label{"обычная"}; // метка для стратегии
};

inline bool closed(TaskStatus s) {
    return s == TaskStatus::Done || s == TaskStatus::Cancelled;
}

// старое имя на всякий случай
inline bool is_closed(TaskStatus s) { return closed(s); }

std::string status_to_text(TaskStatus status);
TaskStatus text_to_status(const std::string& value);
std::string to_string(TaskStatus status);

} // namespace taskflow
