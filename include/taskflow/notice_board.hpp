#pragma once
#include <string>
#include <vector>

namespace taskflow {

// ЛР2: доска уведомлений (Observer в простом виде)
class NoticeBoard {
public:
    void post(std::string text) { items_.push_back(std::move(text)); }
    const std::vector<std::string>& all() const { return items_; }
    std::size_t count() const { return items_.size(); }
    void clear() { items_.clear(); }

private:
    std::vector<std::string> items_;
};

} // namespace taskflow
