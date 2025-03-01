#include "ThreadPool.h"

ThreadPool::ThreadPool(const std::string& name):
    name_(name),
    running_(false),
    mutex_(),
    cond_(),
    threadInitCallback_(nullptr),
    threadSize_(0)
{
    // 所有变量初始化
}

ThreadPool::~ThreadPool(){
    stop();
    for(const auto& t:threads_){
        t->join();//等待线程结束
    }
}

/// @brief 启动线程池，并且将runInThread加入到线程中执行
///        并且启动线程初始化回调函数
void ThreadPool::start(){
    running_ =true;
    threads_.reserve(threadSize_);
    for(int i=0;i<threadSize_;i++){
        char id[32];
        snprintf(id, sizeof(id), "%s-%d", name_.c_str(), i+1);
        threads_.emplace_back(new Thread(
            std::bind(&ThreadPool::runInThread,this),name_+id
        ));
        threads_[i]->start();
    }

    //不创建新的线程
    if(threadSize_ == 0 && threadInitCallback_){
        threadInitCallback_();
    }

}

void ThreadPool::stop(){
    std::lock_guard<std::mutex> lock(mutex_);
    running_ = false;
    cond_.notify_all();  //通知所有线程结束了
}

size_t ThreadPool::queueSize() const{
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.size();
}

void ThreadPool::add(ThreadFunction function){
    std::unique_lock<std::mutex> lock(mutex_);
    queue_.push_back(function);
    cond_.notify_one();  // 通知一个线程,有一个任务来了
}

void ThreadPool::runInThread(){
    try{
        if(threadInitCallback_){
            threadInitCallback_();
        }
        ThreadFunction task;

        // 设置while true让线程一直执行不退出
        while(true){
            {
                std::unique_lock<std::mutex> lock(mutex_);
                // 如果任务队列为空，那么启用条件变量进行等待操作
                // 如果线程池调用了stop操作，那么这个running就会暂停，并且推出线程执行程序
                while(queue_.empty()){
                    if(!running_){
                        return ;
                    }
                    cond_.wait(lock);
                }
                // 从任务池中取出一个任务进行执行
                task=queue_.front();
                queue_.pop_front();
            }
            if(task!=nullptr){
                task();
            }
        }
    }
    catch(...){
        // 处理线程中的异常，并且报错
        std::cerr<<"exception caught in ThreadPool"<<std::endl;
        throw;
    }
}