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
    thread_ = std::make_shared<std::thread>([&](){
        // 新的func创建新的tid
        tid_ = CurrentThread::tid();
        // v操作
        sem_post(&sem);
        // 线程函数在新线程中执行
        func_();
    });
    // 线程函数在新线程中执行，detach()函数使主线程不等待子线程结束
    /**
     * 这里必须等待获取上面新创建的线程的tid
     * 未获取到信息则不会执行sem_post，所以会被阻塞住
     * 如果不使用信号量操作，则别的线程访问tid时候，可能上面的线程还没有获取到tid
     */
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