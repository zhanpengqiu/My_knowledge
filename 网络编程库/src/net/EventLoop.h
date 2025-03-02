#ifndef EVENTLOOP_H
#define EVENTLOOP_H

#include "Timestamp.h"
#include "noncopyable.h"
#include "CurrentThread.h"
#include "TimerQueue.h"

#include <memory>
#include <vector>
#include <functional>
#include <mutex>
#include <atomic>

class Channel;
class Poller;
// 事件循环类 主要包含了两大模块，channel poller
class EventLoop:noncopyable{
public:
    using Functor=std::function<void()>;
    EventLoop();
    ~EventLoop();

    // 运行事件循环
    void loop();

    // 退出事件循环
    void quit();

    Timestamp pollReturnTime()const {return pollReturnTime_;}

    void runInLoop(Functor cb); // 在当前线程中执行cb
    void queueInLoop(Functor cb); // 在当前线程中执行cb，并加入pendingFunctors_中
    void wakeup(); 

    void updateChannel(Channel* channel);
    void removeChannel(Channel* channel);
    bool hasChannel(Channel* channel);

    bool isInLoopThread()const {return threadId_==CurrentThread::tid();}
    /**
     * 定时任务相关函数
     */
    void runAt(Timestamp timestamp,Functor&& cb){
        timerQueue_->addTimer(std::move(cb),timestamp,0.0);
    }
    void runAfter(double waitTime, Functor&& cb){
        Timestamp timestamp(addTime(Timestamp::now(),waitTime));
        runAt(timestamp,std::move(cb));
    }
    void runEvery(double interval, Functor&& cb){
        Timestamp timestamp(addTime(Timestamp::now(), interval));
        timerQueue_->addTimer(std::move(cb),timestamp,interval);
    }
private:
    void handleRead();
    void doPendingFunctors();

    using ChannelList = std::vector<Channel*>;

    std::atomic_bool looping_;  // 原子操作，通过CAS实现
    std::atomic_bool quit_;     // 标志退出事件循环
    std::atomic_bool callingPendingFunctors_; // 标志当前loop是否有需要执行的回调操作
    const pid_t threadId_;      // 记录当前loop所在线程的id
    Timestamp pollReturnTime_;  // poller返回发生事件的channels的返回时间
    std::unique_ptr<Poller> poller_;
    std::unique_ptr<TimerQueue> timerQueue_;
    /**
     * TODO:eventfd用于线程通知机制，libevent和我的webserver是使用sockepair
     * 作用：当mainLoop获取一个新用户的Channel 需通过轮询算法选择一个subLoop 
     * 通过该成员唤醒subLoop处理Channel
    */
    int wakeupFd_;
    std::unique_ptr<Channel> wakeupChannel_;

    ChannelList activeChannels_;            // 活跃的Channel
    Channel* currentActiveChannel_;         // 当前处理的活跃channel
    std::mutex mutex_;                      // 用于保护pendingFunctors_线程安全操作
    std::vector<Functor> pendingFunctors_;  // 存储loop跨线程需要执行的所有回调操作
};

#endif // EVENTLOOP_H