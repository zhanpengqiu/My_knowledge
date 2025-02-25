#include <iostream>
#include <mutex>
#include <condition_variable>
#include <queue>
#include<thread>
#include <chrono>

using namespace std;

queue<int> buffer;

mutex mtx;

condition_variable cv;

const int BUFFER_SIZE = 10;

void producer(int id) {
    int item=0;

    while(true){
        {
            unique_lock<mutex> lock(mtx);
            cv.wait(lock, []{return buffer.size()<BUFFER_SIZE;});

            buffer.push(item++);

            cout << "Producer " << id << " produced item " << item << endl;
        }

        cv.notify_one();

        this_thread::sleep_for(chrono::milliseconds(100));
    }


}

void consumer(int id) {
    while(true){
        {
            unique_lock<mutex> lock(mtx);
            cv.wait(lock,[]{return buffer.size()>0;});

            int item = buffer.front();

            buffer.pop();

            lock.unlock();
            cout<<"consumer"<<id<<" consumerd item:"<<item<<" buffersize:"<<buffer.size()<<endl;
        }
        cv.notify_one();
        this_thread::sleep_for(chrono::milliseconds(100));
    }

}

int main(){
    thread thread_producer1(producer,1);
    thread thread_producer2(producer,2);
    thread thread_consumer1(consumer,1);
    thread thread_consumer2(consumer,2);

    this_thread::sleep_for(std::chrono::seconds(122));

    thread_consumer1.detach();
    thread_consumer2.detach();
    thread_producer1.detach();
    thread_producer2.detach();

    return 0;
}