#pragma once
#include "taskflow/priority_strategy.hpp"
#include "taskflow/repository.hpp"
#include <functional>
#include <string>
#include <vector>

namespace taskflow {

using NotifyFn = std::function<void(std::string message)>;

class TaskService {
public:
    explicit TaskService(TaskRepositoryPtr repo, PriorityStrategyPtr strat);

    Task add(std::string title, std::string who, int prio,
             std::string description = "", std::string label = "обычная");
    bool set_assignee(std::int64_t id, std::string who); // ЛР1 assign
    bool change_status(std::int64_t id, TaskStatus next);
    std::vector<Task> ordered() const;
    std::optional<Task> get(std::int64_t id) const;
    int total() const;

    void bind_notifier(NotifyFn fn); // ЛР2 Observer

private:
    void check(const std::string& title, const std::string& who,
               int prio, const std::string& description) const;
    bool allowed(TaskStatus from, TaskStatus to) const;

    TaskRepositoryPtr repo_;
    PriorityStrategyPtr strat_;
    std::vector<NotifyFn> notifiers_;
};

} // namespace taskflow
