#include "simple_test.hpp"
#include "taskflow/in_memory_task_repository.hpp"
#include "taskflow/line_task_parser.hpp"
#include "taskflow/notice_board.hpp"
#include "taskflow/priority_strategy.hpp"
#include "taskflow/repository_factory.hpp"
#include "taskflow/sqlite_task_repository.hpp"
#include "taskflow/task_service.hpp"
#include <memory>
#include <string>

using namespace taskflow;

TEST_CASE("фабрика memory") {
    AppConfig cfg; cfg.engine = "memory";
    auto repo = make_repository(cfg);
    Task d; d.title = "a"; d.assignee = "b"; d.priority = 3;
    CHECK_TRUE(repo->find_by_id(repo->create(d).id).has_value());
}

TEST_CASE("закрытые в конце") {
    auto repo = std::make_shared<InMemoryTaskRepository>();
    TaskService s(repo, std::make_shared<ClosedLastPriorityStrategy>());
    auto a = s.add("A", "u", 1);
    auto b = s.add("B", "u", 5);
    s.change_status(a.id, TaskStatus::InProgress);
    s.change_status(a.id, TaskStatus::Done);
    CHECK_EQ(s.ordered().front().id, b.id);
}

TEST_CASE("фабрика sqlite") {
    AppConfig cfg; cfg.engine = "sqlite"; cfg.database = "tmp_k.db";
    auto repo = make_repository(cfg);
    auto* p = dynamic_cast<SQLiteTaskRepository*>(repo.get());
    CHECK_TRUE(p != nullptr);
    CHECK_EQ(p->db(), std::string("tmp_k.db"));
}

TEST_CASE("фабрика неизвестный engine") {
    AppConfig cfg; cfg.engine = "postgres";
    CHECK_THROWS(make_repository(cfg));
}

TEST_CASE("метка срочно выше") {
    auto repo = std::make_shared<InMemoryTaskRepository>();
    TaskService s(repo, std::make_shared<LabelPriorityStrategy>());
    auto normal = s.add("N", "u", 1, "", "обычная");
    auto urgent = s.add("U", "u", 3, "", "срочно");
    CHECK_EQ(s.ordered().front().id, urgent.id);
    CHECK_EQ(s.ordered().back().id, normal.id);
}

TEST_CASE("доска уведомлений") {
    TaskService s(std::make_shared<InMemoryTaskRepository>(),
                  std::make_shared<DefaultPriorityStrategy>());
    NoticeBoard board;
    s.bind_notifier([&](std::string m) { board.post(std::move(m)); });
    auto t = s.add("X", "К", 2);
    CHECK_TRUE(board.count() == 1);
    CHECK_TRUE(board.all().front().find(std::to_string(t.id)) != std::string::npos);
}

TEST_CASE("разбор pipe-строки") {
    auto t = parse_pipe_line("Hello|World|2|desc|срочно");
    CHECK_EQ(t.title, std::string("Hello"));
    CHECK_EQ(t.assignee, std::string("World"));
    CHECK_EQ(t.priority, 2);
    CHECK_EQ(t.label, std::string("срочно"));
}

TEST_CASE("плохая pipe-строка") {
    CHECK_THROWS(parse_pipe_line("мало"));
}

TEST_CASE("sqlite stub") {
    SQLiteTaskRepository r("z.db");
    CHECK_THROWS(r.create(Task{}));
}
