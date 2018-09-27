// Copyright 2018 Citra Emulator Project / PPSSPP Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <atomic>
#include <functional>
#include <future>
#include <mutex>
#include <thread>
#include <vector>
#include "common/assert.h"
#include "common/threadsafe_queue.h"

namespace Common {

class ThreadPool : NonCopyable {
private:
    explicit ThreadPool(std::size_t num_threads) : num_threads(num_threads), workers(num_threads) {
        ASSERT(num_threads);
    }

public:
    static ThreadPool& GetPool() {
        static ThreadPool thread_pool(std::thread::hardware_concurrency());
        return thread_pool;
    }

    void SetSpinlocking(bool enable) {
        for (auto& worker : workers) {
            worker.spinlock_enabled = enable;
            if (enable) {
                std::unique_lock<std::mutex> lock(worker.mutex);
                lock.unlock();
                worker.cv.notify_one();
            }
        }
    }

    template <typename F, typename... Args>
    auto Push(F&& f, Args&&... args) {
        auto ret = workers[next_worker].Push(std::forward<F>(f), std::forward<Args>(args)...);
        next_worker = (next_worker + 1) % num_threads;
        return ret;
    }

    std::size_t TotalThreads() const {
        return num_threads;
    }

private:
    class Worker {
    public:
        Worker() : exit_loop(false), spinlock_enabled(false), thread([this] { Loop(); }) {}

        ~Worker() {
            exit_loop = true;
            std::unique_lock<std::mutex> lock(mutex);
            lock.unlock();
            cv.notify_one();
            thread.join();
        }

        template <typename F, typename... Args>
        std::future<void> Push(F&& f, Args&&... args) {
            auto task = std::make_shared<std::packaged_task<decltype(f(args...))()>>(
                std::bind(std::forward<F>(f), std::forward<Args>(args)...));

            queue.Push([task] { (*task)(); });

            if (!spinlock_enabled.load(std::memory_order_relaxed)) {
                std::unique_lock<std::mutex> lock(mutex);
                lock.unlock();
                cv.notify_one();
            }

            return task->get_future();
        }

        std::atomic<bool> spinlock_enabled;
        std::mutex mutex;
        std::condition_variable cv;

    private:
        void Loop() {
            while (true) {
                std::function<void()> task;
                while (queue.Pop(task))
                    task();
                if (spinlock_enabled)
                    continue;

                std::unique_lock<std::mutex> lock(mutex);
                if (!queue.Empty())
                    continue;
                if (exit_loop)
                    break;
                cv.wait(lock);
            }
        }

        bool exit_loop;
        SPSCQueue<std::function<void()>> queue;
        std::thread thread;
    };

    const std::size_t num_threads;
    std::size_t next_worker = 0;
    std::vector<Worker> workers;
};
} // namespace Common
