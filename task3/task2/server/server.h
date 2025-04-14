#ifndef SERVER_H
#define SERVER_H

#include <atomic>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <thread>
#include <optional>


template<typename T>
class Server {
public:
    explicit Server(size_t num_workers);
    ~Server();

    void Start();
    void Stop();

    template<typename Fn, typename... Args>
    size_t AddTask(Fn &&func, Args &&...args);

    std::optional<T> RequestResult(size_t task_number);
    size_t GetTaskNumber();


private:
    void ProcessTasks();
    std::vector<std::jthread> workers_;
    size_t num_workers_;
    std::atomic<bool> running_ = false;
    size_t next_id_ = 0;
    std::mutex mtx_;
    std::condition_variable cv_;
    std::condition_variable cv_result_;

    std::queue<std::pair<size_t, std::function<T()>>> tasks_;
    std::unordered_map<size_t, std::future<T>> results_;
};

#include "server.tpp"
#endif // SERVER_H
