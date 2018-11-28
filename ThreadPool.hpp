#ifndef _THREAD_POOL
#define _THREAD_POOL

#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>

namespace Fang
{
    class noncopyable{
    public:
        noncopyable(noncopyable&) = delete;
        noncopyable& operator=(noncopyable&) = delete;
    protected:
        noncopyable() = default;
        ~noncopyable() = default;
    };

    class ThreadPool : noncopyable {
    public:
        explicit ThreadPool(std::size_t n);
        ~ThreadPool();
        template<typename FUNC, typename... ARGS> void insert_task(FUNC&& func, ARGS&&... args);
        void join();
    private:
        std::vector<std::thread> workers_;
        std::queue<std::function<void()>> tasks_;
        std::mutex qmutex_;
        std::condition_variable cv_;
        bool stop_;
    };
} // namespace Fang

namespace Fang
{
    ThreadPool::ThreadPool(size_t n): stop_(false) {
        for (int i=0; i<n; ++i) {
            workers_.emplace_back([this]{
                    while(true) {
                        std::function<void()> task;
                        {
                            std::unique_lock<std::mutex> lock(qmutex_);
                            cv_.wait(lock, [this]{return this->stop_ || !this->tasks_.empty();});
                            if (this->stop_ && this->tasks_.empty()) {
                                return;
                            }
                            task = std::move(this->tasks_.front());
                            this->tasks_.pop();
                        }
                        task();
                    }
                });
        }
    }

    template<typename FUNC, typename... ARGS>
    void ThreadPool::insert_task(FUNC &&func, ARGS&&... args) {
        auto f = std::bind(std::forward<FUNC>(func), std::forward<ARGS>(args)...);
        {
            std::unique_lock<std::mutex> lock(qmutex_);
            if (stop_) {
                throw std::runtime_error("insert task into a stoped threadpool.");
            }
            tasks_.emplace([f]{f();});
            cv_.notify_one();
        }
    }

    void ThreadPool::join(){
        {
            std::unique_lock<std::mutex> lock;
            stop_ = true;
        }
        cv_.notify_all();
        for (auto& w: workers_)
            w.join();
    }

    ThreadPool::~ThreadPool() {
        join();
    }
} // namespace Fang


#endif  // #ifndef _THREAD_POOL
