#pragma once
#include <deque>
#include <mutex>
#include <atomic>
#include <vector>
#include <functional>

namespace core
{
    class ThreadPool
	{
    public:
        using Job = std::function<void()>;

        explicit ThreadPool(size_t numThreads = std::thread::hardware_concurrency());

        ~ThreadPool();

        // 작업 등록 (반환값 필요 없는 경우)
        void Enqueue(Job job);

        // 모든 작업 완료 대기
        void WaitAll() const;

    private:
        // 워커 루프
        void workerLoop();

        std::vector<std::thread>      _workers;
        std::deque<Job>               _jobQueue;
        std::mutex                    _queueMutex;
        std::condition_variable       _queueCv;
        std::atomic<bool>             _stopping;
        std::atomic<int>              _pendingJobs;
    };

    inline ThreadPool::ThreadPool(size_t numThreads): _stopping(false), _pendingJobs(0)
    {
	    // 워커 스레드 생성
	    for (size_t i = 0; i < numThreads; ++i)
	    {
		    _workers.emplace_back([this] { workerLoop(); });
	    }
    }

    inline ThreadPool::~ThreadPool()
    {
	    // 종료 신호 및 모든 워커 join
	    {
		    std::lock_guard<std::mutex> lk(_queueMutex);
		    _stopping = true;
	    }
	    _queueCv.notify_all();
	    for (auto& t : _workers)
	    {
		    if (t.joinable()) t.join();
	    }
    }

    inline void ThreadPool::Enqueue(Job job)
    {
	    {
		    std::lock_guard<std::mutex> lk(_queueMutex);
		    _jobQueue.push_back(std::move(job));
		    ++_pendingJobs;
	    }
	    _queueCv.notify_one();
    }

    inline void ThreadPool::WaitAll() const
    {
	    while (_pendingJobs.load() > 0) 
	    {
		    std::this_thread::yield();
	    }
    }

    inline void ThreadPool::workerLoop()
    {
	    while (true) 
	    {
		    Job job;
		    {
			    std::unique_lock<std::mutex> lk(_queueMutex);
			    _queueCv.wait(lk, [this] { return _stopping || !_jobQueue.empty(); });
			    if (_stopping && _jobQueue.empty())
				    return;
			    job = std::move(_jobQueue.front());
			    _jobQueue.pop_front();
		    }
		    try 
		    {
			    job();
		    }
		    catch (...) 
		    {
			    // 로깅 또는 예외 처리
		    }
		    --_pendingJobs;
	    }
    }
}
