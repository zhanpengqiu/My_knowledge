#ifndef EVENT_LOOP_THREAD_H
#define EVENT_LOOP_THREAD_H
#include <functional>
#include <mutex>
#include <condition_variable>

#include "noncopyable.h"
#include "Thread.h"


class EventLoop;
class EventLoopThread : noncopyable
{
public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;


    EventLoopThread(const ThreadInitCallback& cb=ThreadInitCallback(), 
                    const std::string& name);
    ~EventLoopThread();

    EventLoop* startLoop();

private:
    void threadFunc();

    // EventLoopThread::threadFunc 会创建 EventLoop
    EventLoop* loop_;
    bool exiting_;
    Thread thread_;
    std::mutex mutex_;
    std::condition_variable cond_;
    ThreadInitCallback callback_;
};



#endif

