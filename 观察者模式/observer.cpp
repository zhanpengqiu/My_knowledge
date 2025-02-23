#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
//使用智能指针对内存进行管理
#include <memory>
using namespace std;

class Observer{
public:
    virtual void update(const string& message) = 0;
    virtual ~Observer()=default;
};

class Subject{
private:
    vector<shared_ptr<Observer>> observers;
public:
    void attach(shared_ptr<Observer> observer){
        observers.push_back(observer);
    }
    void detach(shared_ptr<Observer> observer){
        observers.erase(remove(observers.begin(), observers.end(), observer), observers.end());
    }
    void notify(const string& message){
        for(const auto& observer:observers){
            observer->update(message);
        }
    }
};

class ConcreteSubject:public Subject{
private:   
    string state;
public:
    void setState(const string& newState){
        state=newState;
        notify(state);
    }

    string getState() const{
        return state;
    }
};

class ConcreteObserver:public Observer{
private:
    string name;
    shared_ptr<ConcreteSubject> subject;
public:
    ConcreteObserver(const string& name, shared_ptr<ConcreteSubject> subject):name(name), subject(subject){
    }
    void update(const string& message) override{
        cout<<name<<" received message: "<<message<<endl;
    }
};

int main(){
    auto subject = make_shared<ConcreteSubject>();

    auto observer1 = make_shared<ConcreteObserver>("Observer1", subject);
    auto observer2 = make_shared<ConcreteObserver>("Observer2", subject);
    auto observer3 = make_shared<ConcreteObserver>("Observer3", subject);

    subject->attach(observer1);
    subject->attach(observer2);
    subject->attach(observer3);

    subject->setState("New StateA");
    cout<<"----------------------"<<endl;
    subject->setState("New StateB");

    subject->detach(observer1);
    cout<<"----------------------"<<endl;

    subject->setState("New StateC");

    return 0;
}