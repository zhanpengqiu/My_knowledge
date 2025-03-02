#include "EventLoop.h"
#include "Poller.h"
#include <unistd.h>
#include <sys/eventfd.h>
#include <fcntl.h>
// 防止一个线程创建多个EventLoop (thread_local)
__thread EventLoop *t_loopInThisThread = nullptr;
// 定义默认的Poller IO复用接口的超时时间
const int kPollTimeMs=10000;

// eventfd复用
int createEventfd(){
    int eventfd = ::eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK);
    if(eventfd < 0){
        perror("eventfd");
        abort();
    }
    return eventfd;
}

EventLoop::EventLoop():
    looping_(false),
    quit_(false),
    callingPendingFunctors_(false),
    threadId_(CurrentThread::tid()),
    poller_(Poller::newDefaultPoller(this)),
    timerQueue_(new TimerQueue(this)),
    wakeupFd_(createEventfd()),
    wakeupChannel_(new Channel(this,wakeupFd_)),
    currentActiveChannel_(nullptr){
    // 让mainLoop和wakeupChannel在同一个线程中执行
    if(t_loopInThisThread){
        std::cerr<< "Another EventLoop" << t_loopInThisThread << " exists in this thread " << threadId_<<std::endl;
    }else{
        t_loopInThisThread = this;
    }

    // 设置wakeupfd的事件类型以及发生事件的回调函数
    wakeupChannel_->setReadCallback(std::bind(&EventLoop::handleRead, this));
    // 每一个EventLoop都监听wakeChannel的EPOLLIN事件
    wakeupChannel_->enableReading();
}

EventLoop::~EventLoop(){
    //移除所有感兴趣的事件
    wakeupChannel_->disableAll();
    // 将channel从EentLoop中删除
    wakeupChannel_->remove();
    ::close(wakeupFd_);
    //指向EventLoop的指针至空
    t_loopInThisThread = nullptr;
}

void EventLoop::loop(){
    looping_=true;
    quit_=false;

    while(!quit_){
        activeChannels_.clear();
        // 调用Poller的poll函数获取感兴的事件
        pollReturnTime_=poller_->poll(kPollTimeMs, &activeChannels_);
        // 处理感兴趣的事件
        for(Channel* channel : activeChannels_){
            currentActiveChannel_ = channel;
            // 处理channel上发生的事件
            channel->handleEvent(pollReturnTime_);
        }
        // 执行当前EventLoop事件循环需要处理的回调操作
        /**
         * IO thread：mainLoop accept fd 打包成 chennel 分发给 subLoop
         * mainLoop实现注册一个回调，交给subLoop来执行，wakeup subLoop 之后，让其执行注册的回调操作
         * 这些回调函数在 std::vector<Functor> pendingFunctors_; 之中
         */
        doPendingFunctors();
    }
    looping_=false;
}

void EventLoop::quit(){
    quit_=true;
    /**
     * TODO:生产者消费者队列派发方式和muduo的派发方式
     * 有可能是别的线程调用quit(调用线程不是生成EventLoop对象的那个线程)
     * 比如在工作线程(subLoop)中调用了IO线程(mainLoop)
     * 这种情况会唤醒主线程
    */
    if(isInLoopThread()){
        wakeup();
    }
}

void EventLoop::runInLoop(Functor cb){
    // 每个EventLoop都保存创建自己的线程tid
    // 我们可以通过CurrentThread::tid()获取当前执行线程的tid然后和EventLoop保存的进行比较
    if(isInLoopThread()){
        cb();
    }
    // 在非当前eventLoop线程中执行回调函数，需要唤醒evevntLoop所在线程 
    else{
        queueInLoop(cb);
    }
}

void EventLoop::queueInLoop(Functor cb){
    {
        std::unique_lock<std::mutex> lock(mutex_);
        pendingFunctors_.emplace_back(cb);
    }
    // 唤醒相应的，需要执行上面回调操作的loop线程
    /** 
     * TODO:
     * std::atomic_bool callingPendingFunctors_; 标志当前loop是否有需要执行的回调操作
     * 这个 || callingPendingFunctors_ 比较有必要，因为在执行回调的过程可能会加入新的回调
     * 则这个时候也需要唤醒，否则就会发生有事件到来但是仍被阻塞住的情况
     */
    if(!isInLoopThread()||callingPendingFunctors_){
        //唤醒loop所在线程
        wakeup();
    }
}

void EventLoop::wakeup(){
    uint64_t one = 1;
    ssize_t n = ::write(wakeupFd_, &one, sizeof one);
    if(n!=sizeof(one)){
        perror("EventLoop::wakeup");
        abort();
    }
}

void EventLoop::handleRead(){
    uint64_t one = 1;
    ssize_t n = ::read(wakeupFd_, &one, sizeof(one));
    // 忽略读取的字节数
    if(n!=sizeof(one)){
        perror("EventLoop::wakeup");
        abort();
    }
}

void EventLoop::updateChannel(Channel* channel){
    poller_->updateChannel(channel);
}

void EventLoop::removeChannel(Channel* channel){
    poller_->removeChannel(channel);
}

bool EventLoop::hasChannel(Channel* channel){
    return poller_->hasChannel(channel);
}

void EventLoop::doPendingFunctors(){
    std::vector<Functor> pendingFunctors;
    callingPendingFunctors_ = true;
    /**
     * TODO:
     * 如果没有生成这个局部的 functors
     * 则在互斥锁加持下，我们直接遍历pendingFunctors
     * 其他线程这个时候无法访问，无法向里面注册回调函数，增加服务器时延
    */
    {
        std::unique_lock<std::mutex> lock(mutex_);
        pendingFunctors_.swap(pendingFunctors);
    }

    for(const auto& cb : pendingFunctors){
        cb();
    }

    callingPendingFunctors_ = false;
}