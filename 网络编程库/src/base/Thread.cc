#include <semaphore.h>
#include "Thread.h"
#include "CurrentThread.h"

std::atomic_int32_t Thread::numCreated_(0);

Thread::Thread(ThreadFunc func,const std::string &name):
    started_(false),
    joined_(false),
    tid_(0),
    func_(std::move(func)),
    name_(name)
{
    setDefaultName();
}

void Thread::start(){
    started_ = true;
    sem_t sem;
    sem_init(&sem, false, 0);  // 创建信号量
    // 线程函数在新线程中执行
    // 注意：detach()和join()函数都可以让主线程在子线程结束后继续执行
    thread_ = std::make_shared<std::thread>([this](){
        tid_ = CurrentThread::tid();
        func_();
        joined_ = true;
    });
    // 线程函数在新线程中执行，detach()函数使主线程不等待子线程结束

    sem_wait(&sem);
}

Thread::~Thread(){
    // 线程启动时且并未设置等待线程时才可调用
    if(started_&&!joined_){
        thread_->detach();
    }
}

void Thread::join(){
    joined_ = true;
    // 等待进程执行完毕，执行完之后调用join挂起
    thread_->join();
}

// 设置默认名称
void Thread::setDefaultName(){
    int num=++numCreated_;

    if(name_.empty()){
        char buf[32];
        snprintf(buf, sizeof(buf), "Thread%d", num);
        name_=buf;
    }
}