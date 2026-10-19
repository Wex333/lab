#include "simple_test.hpp"
#include "taskflow/in_memory_task_repository.hpp"
#include "taskflow/notice_board.hpp"
#include "taskflow/priority_strategy.hpp"
#include "taskflow/task_service.hpp"
#include <memory>
#include <string>

using namespace taskflow;

static TaskService make() {
    return TaskService(std::make_shared<InMemoryTaskRepository>(),
                       std::make_shared<DefaultPriorityStrategy>());
}

TEST_CASE("добавление задачи") {
    auto s = make();
    auto t = s.add("Отчёт", "Кудряшов", 2, "текст", "обычно");
    CHECK_TRUE(t.id > 0);
    CHECK_EQ(t.label, std::string("обычно"));
    CHECK_TRUE(s.total() == 1);
}

TEST_CASE("плохой приоритет") {
    CHECK_THROWS(make().add("x", "y", 8));
}

TEST_CASE("переходы статусов") {
    auto s = make();
    auto t = s.add("Работа", "К", 1);
    CHECK_TRUE(s.change_status(t.id, TaskStatus::InProgress));
    CHECK_TRUE(s.change_status(t.id, TaskStatus::Done));
    CHECK_TRUE(!s.change_status(t.id, TaskStatus::InProgress));
}

TEST_CASE("назначение исполнителя") {
    auto s = make();
    auto t = s.add("Баг", "A", 2);
    CHECK_TRUE(s.set_assignee(t.id, "B"));
    CHECK_EQ(s.get(t.id)->assignee, std::string("B"));
}

TEST_CASE("назначение закрытой запрещено") {
    auto s = make();
    auto t = s.add("Баг", "A", 2);
    s.change_status(t.id, TaskStatus::InProgress);
    s.change_status(t.id, TaskStatus::Done);
    CHECK_TRUE(!s.set_assignee(t.id, "B"));
}

TEST_CASE("длинное описание") {
    CHECK_THROWS(make().add("t", "a", 2, std::string(1001, 'x')));
}

TEST_CASE("описание 1000 символов ок") {
    auto t = make().add("t", "a", 2, std::string(1000, 'x'));
    CHECK_EQ(t.description.size(), static_cast<std::size_t>(1000));
}

TEST_CASE("пустое название") { CHECK_THROWS(make().add("", "a", 2)); }
TEST_CASE("пустой исполнитель") { CHECK_THROWS(make().add("t", "", 2)); }
TEST_CASE("приоритет 0") { CHECK_THROWS(make().add("t", "a", 0)); }
TEST_CASE("приоритет 6") { CHECK_THROWS(make().add("t", "a", 6)); }

TEST_CASE("отмена из New") {
    auto s = make();
    auto t = s.add("t", "a", 3);
    CHECK_TRUE(s.change_status(t.id, TaskStatus::Cancelled));
}

TEST_CASE("set_assignee пустой бросает") {
    auto s = make();
    auto t = s.add("t", "a", 3);
    CHECK_THROWS(s.set_assignee(t.id, ""));
}

TEST_CASE("нет такой задачи") { CHECK_TRUE(!make().get(99).has_value()); }

TEST_CASE("интеграция репозитория") {
    auto repo = std::make_shared<InMemoryTaskRepository>();
    Task d; d.title = "x"; d.assignee = "y"; d.priority = 2;
    auto c = repo->create(d);
    CHECK_TRUE(repo->find_by_id(c.id).has_value());
    c.status = TaskStatus::InProgress;
    CHECK_TRUE(repo->update(c));
    CHECK_TRUE(repo->list().size() == 1);
}

TEST_CASE("порядок default") {
    auto s = make();
    auto low = s.add("L", "a", 5);
    auto high = s.add("H", "b", 1);
    auto o = s.ordered();
    CHECK_EQ(o.front().id, low.id);
    CHECK_EQ(o.back().id, high.id);
}
