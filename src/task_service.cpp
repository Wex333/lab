#include "taskflow/task_service.hpp"
#include <algorithm>
#include <stdexcept>
#include <utility>

namespace taskflow {

std::string status_to_text(TaskStatus status) {
    switch (status) {
        case TaskStatus::New: return "New";
        case TaskStatus::InProgress: return "InProgress";
        case TaskStatus::Done: return "Done";
        case TaskStatus::Cancelled: return "Cancelled";
    }
    return "?";
}

TaskStatus text_to_status(const std::string& value) {
    if (value == "New") return TaskStatus::New;
    if (value == "InProgress") return TaskStatus::InProgress;
    if (value == "Done") return TaskStatus::Done;
    if (value == "Cancelled") return TaskStatus::Cancelled;
    throw std::invalid_argument("статус неизвестен");
}

// совместимость с кодом тестов/API, где ждут to_string
std::string to_string(TaskStatus status) { return status_to_text(status); }

TaskService::TaskService(TaskRepositoryPtr repo, PriorityStrategyPtr strat)
    : repo_(std::move(repo)), strat_(std::move(strat)) {
    if (!repo_ || !strat_) {
        throw std::invalid_argument("нужны репозиторий и стратегия");
    }
}

void TaskService::bind_notifier(NotifyFn fn) {
    if (fn) notifiers_.push_back(std::move(fn));
}

Task TaskService::add(std::string title, std::string who, int prio,
                      std::string description, std::string label) {
    check(title, who, prio, description);
    Task draft;
    draft.title = std::move(title);
    draft.assignee = std::move(who);
    draft.priority = prio;
    draft.description = std::move(description);
    draft.label = std::move(label);
    draft.status = TaskStatus::New;
    auto saved = repo_->create(std::move(draft));
    for (auto& n : notifiers_) {
        n("создана задача #" + std::to_string(saved.id) + " «" + saved.title + "»");
    }
    return saved;
}

bool TaskService::set_assignee(std::int64_t id, std::string who) {
    if (who.empty()) {
        throw std::invalid_argument("исполнитель пустой");
    }
    auto task = repo_->find_by_id(id);
    if (!task) return false;
    if (closed(task->status)) return false; // нельзя трогать закрытую
    task->assignee = std::move(who);
    return repo_->update(*task);
}

bool TaskService::change_status(std::int64_t id, TaskStatus next) {
    auto task = repo_->find_by_id(id);
    if (!task) return false;
    if (!allowed(task->status, next)) return false;
    task->status = next;
    return repo_->update(*task);
}

std::vector<Task> TaskService::ordered() const {
    auto items = repo_->list();
    std::sort(items.begin(), items.end(), [&](const Task& a, const Task& b) {
        const int sa = strat_->score(a);
        const int sb = strat_->score(b);
        if (sa != sb) return sa < sb;
        return a.id < b.id;
    });
    return items;
}

std::optional<Task> TaskService::get(std::int64_t id) const {
    return repo_->find_by_id(id);
}

int TaskService::total() const {
    return static_cast<int>(repo_->list().size());
}

void TaskService::check(const std::string& title, const std::string& who,
                        int prio, const std::string& description) const {
    if (title.empty()) throw std::invalid_argument("название пустое");
    if (who.empty()) throw std::invalid_argument("исполнитель пустой");
    if (prio < 1 || prio > 5) throw std::invalid_argument("приоритет 1..5");
    if (description.size() > 1000) throw std::invalid_argument("описание > 1000");
}

bool TaskService::allowed(TaskStatus from, TaskStatus to) const {
    if (from == to) return true;
    if (closed(from)) return false;
    if (from == TaskStatus::New) {
        return to == TaskStatus::InProgress || to == TaskStatus::Cancelled;
    }
    if (from == TaskStatus::InProgress) {
        return to == TaskStatus::Done || to == TaskStatus::Cancelled;
    }
    return false;
}

} // namespace taskflow
