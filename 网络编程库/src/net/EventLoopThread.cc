#include "EventLoopThread.h"
#include "EventLoop.h"

EventLoopThread::EventLoopThread(const ThreadInitCallback& cb,
                                const std::string &name)
                                : loop_(nullptr), 
                                exiting_(false),
                                thread_(std::bind(&EventLoopThread::threadFunc, this),name),
                                mutex_(),
                                cond_(),
                                callback_(cb){}// 传入的线程初始化回调函数，用户自定义的

EventLoopThread::~EventLoopThread(){
    exiting_ = true;
    if(loop_){
        /**
         * TODO:
         * 不允许指针指向不完整的类类型 "EventLoop"
         * 因为我们的EventLoop在头文件只是前置声明，告诉编译器有这个东西存在
         * 但是实际声明的头文件并未加上，所以使用里面的函数都会失败
         * 我们需要在实现的cc文件里面加入头文件
         */
        loop_->quit();
        thread_.join();
    }
}

EventLoop* EventLoopThread::startLoop(){
    // 调用startLoop即开启一个新线程
    thread_.start();

    EventLoop* loop=nullptr;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        // lambda function
        // 一直等待这个loop为有效的地址，如果为有效的，那么返回loop
        // 如果不是有效的。那么会一直等待
        cond_.wait(lock, [&]{return loop_!=nullptr; });
        loop = loop_;
    }
    return loop;
}

void EventLoopThread::threadFunc(){
    EventLoop loop;

    // 用户自定义的函数
    if(callback_ ){
        callback_(&loop);
    }

    {
        std::unique_lock<std::mutex> lock(mutex_);
        loop_ = &loop;  // 等到生成EventLoop对象之后才唤醒startLoop
        cond_.notify_one();
    }
    // 执行EventLoop的loop() 开启了底层的Poller的poll()
    // 这个是subLoop
    loop.loop();
    // loop是一个事件循环，如果往下执行说明停止了事件循环，需要关闭eventLoop
    // 此处是获取互斥锁再置loop_为空
    std::unique_lock<std::mutex> lock(mutex_);
    loop_ = nullptr;
}


// int main(){
//     EventLoopThread thread;
//     EventLoop* loop = thread.startLoop();

//     loop->runAfter(5, [](){
//         std::cout << "Hello from another thread!" << std::endl;
//     });

//     loop->loop();

//     return 0;
// }