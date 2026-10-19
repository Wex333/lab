#include "taskflow/in_memory_task_repository.hpp"
#include "taskflow/priority_strategy.hpp"
#include "taskflow/task_service.hpp"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstdlib>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

namespace {

std::string esc(const std::string& s) {
    std::string o;
    for (char c : s) {
        if (c == '"' || c == '\\') o.push_back('\\');
        o.push_back(c);
    }
    return o;
}

std::string as_json(const taskflow::Task& t) {
    std::ostringstream o;
    o << "{\"id\":" << t.id
      << ",\"title\":\"" << esc(t.title) << "\""
      << ",\"assignee\":\"" << esc(t.assignee) << "\""
      << ",\"label\":\"" << esc(t.label) << "\""
      << ",\"priority\":" << t.priority
      << ",\"status\":\"" << taskflow::to_string(t.status) << "\""
      << ",\"description\":\"" << esc(t.description) << "\"}";
    return o.str();
}

std::string as_array(const std::vector<taskflow::Task>& v) {
    std::ostringstream o;
    o << "[";
    for (std::size_t i = 0; i < v.size(); ++i) {
        if (i) o << ",";
        o << as_json(v[i]);
    }
    o << "]";
    return o.str();
}

std::string read_str(const std::string& body, const std::string& key) {
    auto p = body.find("\"" + key + "\"");
    if (p == std::string::npos) return "";
    p = body.find('"', body.find(':', p));
    if (p == std::string::npos) return "";
    auto e = body.find('"', p + 1);
    if (e == std::string::npos) return "";
    return body.substr(p + 1, e - p - 1);
}

int read_int(const std::string& body, const std::string& key, int def) {
    auto p = body.find("\"" + key + "\"");
    if (p == std::string::npos) return def;
    p = body.find(':', p);
    if (p == std::string::npos) return def;
    try { return std::stoi(body.substr(p + 1)); } catch (...) { return def; }
}

std::string answer(int code, const char* why, const std::string& body) {
    std::ostringstream o;
    o << "HTTP/1.1 " << code << " " << why << "\r\n"
      << "Content-Type: application/json; charset=utf-8\r\n"
      << "Content-Length: " << body.size() << "\r\n"
      << "Connection: close\r\n\r\n" << body;
    return o.str();
}

} // namespace

int main() {
    using namespace taskflow;
    int port = 8080;
    if (const char* e = std::getenv("PORT")) {
        try { port = std::stoi(e); } catch (...) {}
    }

    auto repo = std::make_shared<InMemoryTaskRepository>();
    auto strat = std::make_shared<LabelPriorityStrategy>();
    TaskService svc(repo, strat);
    svc.add("Стартовая", "kudryashov", 3, "seed", "обычная");

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) return 1;
    int opt = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    sockaddr_in a{};
    a.sin_family = AF_INET;
    a.sin_addr.s_addr = INADDR_ANY;
    a.sin_port = htons(static_cast<uint16_t>(port));
    if (bind(fd, reinterpret_cast<sockaddr*>(&a), sizeof(a)) < 0) return 1;
    if (listen(fd, 16) < 0) return 1;
    std::cout << "kudryashov api port=" << port << std::endl;

    while (true) {
        int c = accept(fd, nullptr, nullptr);
        if (c < 0) continue;
        std::string req;
        char buf[2048];
        while (true) {
            auto n = recv(c, buf, sizeof(buf), 0);
            if (n <= 0) break;
            req.append(buf, static_cast<std::size_t>(n));
            auto end = req.find("\r\n\r\n");
            if (end == std::string::npos) continue;
            std::size_t cl = 0;
            auto p = req.find("Content-Length:");
            if (p != std::string::npos && p < end) {
                try { cl = static_cast<std::size_t>(std::stoul(req.substr(p + 15))); }
                catch (...) {}
            }
            if (req.size() >= end + 4 + cl) break;
        }
        if (req.empty()) { close(c); continue; }

        std::string method, path;
        { std::istringstream iss(req); iss >> method >> path; }

        std::string out;
        if (method == "GET" && path == "/health") {
            out = answer(200, "OK", "{\"result\":\"up\",\"student\":\"kudryashov\"}");
        } else if (method == "GET" && path.rfind("/tasks", 0) == 0) {
            out = answer(200, "OK", as_array(svc.ordered()));
        } else if (method == "POST" && path == "/tasks") {
            auto pos = req.find("\r\n\r\n");
            std::string body = pos == std::string::npos ? "" : req.substr(pos + 4);
            try {
                auto label = read_str(body, "label");
                if (label.empty()) label = "обычная";
                auto t = svc.add(read_str(body, "title"), read_str(body, "assignee"),
                                 read_int(body, "priority", 3),
                                 read_str(body, "description"), label);
                out = answer(201, "Created", as_json(t));
            } catch (const std::exception& ex) {
                out = answer(400, "Bad Request",
                             std::string("{\"error\":\"") + esc(ex.what()) + "\"}");
            }
        } else {
            out = answer(404, "Not Found", "{\"error\":\"нет такого маршрута\"}");
        }
        send(c, out.data(), out.size(), 0);
        close(c);
    }
}
