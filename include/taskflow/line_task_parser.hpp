#pragma once
#include "taskflow/task.hpp"
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace taskflow {

// ЛР2: разбор строки вида title|assignee|priority|description
inline Task parse_pipe_line(const std::string& line) {
    std::stringstream ss(line);
    std::string chunk;
    std::vector<std::string> parts;
    while (std::getline(ss, chunk, '|')) {
        parts.push_back(chunk);
    }
    if (parts.size() < 3) {
        throw std::invalid_argument("нужно минимум 3 поля через |");
    }
    Task t;
    t.title = parts[0];
    t.assignee = parts[1];
    t.priority = std::stoi(parts[2]);
    if (parts.size() >= 4) t.description = parts[3];
    if (parts.size() >= 5) t.label = parts[4];
    t.status = TaskStatus::New;
    return t;
}

} // namespace taskflow
