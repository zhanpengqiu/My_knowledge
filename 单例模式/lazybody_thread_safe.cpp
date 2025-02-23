#include <iostream>
#include <mutex>

using namespace std;

class Singleton {
public:
    static Singleton* getInstance(){
        // 确保线程安全模式
        // 双重锁检查
        if(_instance==nullptr){
            lock_guard<mutex>lock(_mtx);
            if(_instance==nullptr){
                _instance = new Singleton();
            }
        }
        return _instance;
    }
    // 删除拷贝赋值构造函数，确保单例的唯一性
    Singleton(const Singleton&)=delete;
    Singleton& operator=(const Singleton&)=delete;
private:
    //私有化构造函数
    Singleton(){
        cout<<"Singleton Created!"<<endl;
    };

    static Singleton* _instance;
    static mutex _mtx;
};
Singleton *Singleton::_instance=nullptr;
mutex Singleton::_mtx;
int main() {
    Singleton *ptr=Singleton::getInstance();
    return 0;
}