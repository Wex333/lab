#include "taskflow/in_memory_task_repository.hpp"
#include "taskflow/line_task_parser.hpp"
#include "taskflow/notice_board.hpp"
#include "taskflow/priority_strategy.hpp"
#include "taskflow/task_service.hpp"
#include <iostream>
#include <memory>

int main() {
    using namespace taskflow;

    auto repo = std::make_shared<InMemoryTaskRepository>();
    auto strat = std::make_shared<LabelPriorityStrategy>();
    TaskService svc(repo, strat);

    NoticeBoard board;
    svc.bind_notifier([&](std::string msg) { board.post(std::move(msg)); });

    svc.add("Собрать требования", "Кудряшов", 2, "короткое ТЗ", "обычно");
    svc.add("Сделать сервис", "Кудряшов", 1, "TaskService", "срочно");

    auto parsed = parse_pipe_line("Проверить тесты|QA|3|ctest|обычная");
    parsed = repo->create(parsed);

    std::cout << "=== Kudryashov / TaskFlow ===\n";
    std::cout << "всего задач: " << svc.total() << "\n\n";
    for (const auto& t : svc.ordered()) {
        std::cout << "• #" << t.id << " " << t.title
                  << " [" << t.label << "] -> " << t.assignee
                  << ", " << to_string(t.status)
                  << ", приоритет " << t.priority
                  << "\n  " << t.description << '\n';
    }
    std::cout << "\nуведомления (" << board.count() << "):\n";
    for (const auto& m : board.all()) {
        std::cout << " - " << m << '\n';
    }
    std::cout << "импорт pipe id=" << parsed.id << '\n';
    return 0;
}
