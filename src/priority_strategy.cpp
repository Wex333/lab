#include "taskflow/priority_strategy.hpp"

namespace taskflow {

int DefaultPriorityStrategy::score(const Task& task) const {
    return 10 - task.priority;
}

int ClosedLastPriorityStrategy::score(const Task& task) const {
    return closed(task.status) ? 1000 : 10 - task.priority;
}

int LabelPriorityStrategy::score(const Task& task) const {
    if (closed(task.status)) {
        return 5000;
    }
    // "срочно" почти всегда сверху
    if (task.label == "срочно" || task.label == "urgent") {
        return task.priority; // 1..5
    }
    return 100 + (10 - task.priority);
}

} // namespace taskflow
