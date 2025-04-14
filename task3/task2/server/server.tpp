#ifndef SERVER_TPP
#define SERVER_TPP

#include "server.h"

template<typename T>
Server<T>::Server(size_t num_workers) : num_workers_(num_workers){};

template<typename T>
Server<T>::~Server() {
    Server<T>::Stop();
}

template<typename T>
void Server<T>::Start() {
    this->running_ = true;

    for (size_t i = 0; i < this->num_workers_; i++) {
        this->workers_.emplace_back(&Server<T>::ProcessTasks, this);
    }
}

template<typename T>
void Server<T>::Stop() {
    this->running_ = false;
    cv_.notify_all();
}

template<typename T>
template<typename Fn, typename... Args>
size_t Server<T>::AddTask(Fn &&func, Args &&...args) {
    std::unique_lock<std::mutex> lock(mtx_);

    tasks_.push({this->next_id_++,
                 [func = std::forward<Fn>(func), ... args = std::forward<Args>(args)]() { return func(args...); }});
    cv_.notify_one();
    return next_id_ - 1;
}

template<typename T>
std::optional<T> Server<T>::RequestResult(size_t task_number) {
    std::lock_guard<std::mutex> lock(mtx_);
    if (results_.find(task_number) != results_.end()) {
        return results_[task_number].get();
    } else {
        return std::nullopt;
    }
}

template<typename T>
size_t Server<T>::GetTaskNumber() {
    return next_id_ - 1;
}

template<typename T>
void Server<T>::ProcessTasks() {
    while (running_) {
        std::unique_lock<std::mutex> lock(mtx_);
        cv_.wait(lock, [this] { return !tasks_.empty() || !running_; });

        if (!running_ && tasks_.empty())
            break;

        auto task = std::move(tasks_.front());
        tasks_.pop();

        lock.unlock();

        auto result = std::async(std::launch::async, task.second);

        std::lock_guard<std::mutex> result_lock(mtx_);
        results_[task.first] = std::move(result);
        cv_result_.notify_all();
    }
}

#endif // SERVER_TPP
