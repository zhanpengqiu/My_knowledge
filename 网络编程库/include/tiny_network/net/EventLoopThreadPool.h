#ifndef EVENT_LOOP_THREAD_POOL_H
#define EVENT_LOOP_THREAD_POOL_H

#include <string>
#include <vector>
#include <memory>
#include <functional>

class EventLoop;
class EventLoopThread;

class EventLoopThreadPool {
public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;
    EventLoopThreadPool(EventLoop *baseLoop,const std::string &name);
    ~EventLoopThreadPool();

    // 设置线程数量
    void setThreadNum(int numThreads){
        numThreads_ = numThreads;
    };
    // 开启线程池
    void start(const ThreadInitCallback &cb = ThreadInitCallback());

    // 获取下一个EventLoop,如果是多线程的状态下，那么会默认以轮训的方式分配Channel给子Loop
    EventLoop *getNextLoop();

    std::vector<EventLoop *> getAllLoops();

    bool started()const {return started_;};
    const std::string name()const {return name_;};

private:
    EventLoop *baseLoop_; // 用户使用muduo创建的loop 如果线程数为1 那直接使用用户创建的loop 否则创建多EventLoop
    std::string name_;
    bool started_;      // 开启线程池标志
    int numThreads_;    // 创建线程数量
    size_t next_;          // 轮询的下标
    std::vector<std::unique_ptr<EventLoopThread>> threads_; // 保存所有的EventLoopThread容器
    std::vector<EventLoop *> loops_;    // 保存创建的所有EventLoop
};


#endif  // EVENT_LOOP_THREAD_POOL_H