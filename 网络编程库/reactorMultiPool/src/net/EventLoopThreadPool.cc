#include <memory>
#include <iostream>
#include "EventLoopThread.h"
#include "EventLoopThreadPool.h"


EventLoopThreadPool::EventLoopThreadPool(EventLoop* baseLoop, const std::string &name)
        : baseLoop_(baseLoop),
        name_(name),
        started_(false),
        numThreads_(0),
        next_(0){}


EventLoopThreadPool::~EventLoopThreadPool(){
    // dont't delete loop, it's stack variable
}


void EventLoopThreadPool::start(const ThreadInitCallback &callback){
    started_ = true;

    // 创建线程并执行
    for(int i = 0; i < numThreads_; ++i){
        char buf[name_.size()+32];
        snprintf(buf, sizeof buf, "%s%d", name_.c_str(), i);
        //创建EventLoopThread对象
        EventLoopThread *t=new EventLoopThread(callback,buf);
        // 加入此EventLoopThread入容器
        threads_.push_back(std::unique_ptr<EventLoopThread>(t));
        // 底层创建线程 绑定一个新的EventLoop 并返回该loop的地址
        // 此时已经开始执行新线程了
        loops_.push_back(t->startLoop());
    }

    //整个服务端只有一个线程运行baseloop
    if(numThreads_ == 0 && callback){
        // 那么就不需要给子线程完成用户回调函数
        callback(baseLoop_);
    }
}

//如果在多线程中，baseLoop(mainLoop)会默认以轮训的方式分配给Channel给suboop
EventLoop* EventLoopThreadPool::getNextLoop(){
    // 如果只设置一个线程 也就是只有一个mainReactor 无subReactor 
    // 那么轮询只有一个线程 getNextLoop()每次都返回当前的baseLoop_
    EventLoop *loop = baseLoop_;

    if(!loops_.empty()){
        loop=loops_[next_];
        ++next_;
        //轮训
        if(next_>=loops_.size()){
            next_=0;  // 循环
        }
    }
    return loop;
}

std::vector<EventLoop *> EventLoopThreadPool::getAllLoops(){
    if(loops_.empty()){
        // 只有一个线程 所以返回一个baseLoop_
        return std::vector<EventLoop *>(1,baseLoop_);
    }
    else{
        return loops_;
    }
}