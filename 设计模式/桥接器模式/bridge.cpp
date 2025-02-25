#include <iostream>
#include<string>

using namespace std;

//抽象实现类
class Implementor{
public:
    virtual ~Implementor()=default;
    virtual void operationImplementation()=0;
};

//具体实现类A
class ConcreteImplementorA:public Implementor{
public:
    void operationImplementation() override{
        cout<<"ConcreteImplementorA operation"<<endl;
    }
};

//具体实现类B
class ConcreteImplementorB:public Implementor{
public:
    void operationImplementation() override{
        cout<<"ConcreteImplementorB operation"<<endl;
    }
};

class Abstraction{
protected:
    Implementor* impl;
public:
    Abstraction(Implementor* _impl):impl(_impl){}
    virtual ~Abstraction(){
        delete impl;
    }
    virtual void operation()=0;
};

//具体抽象类A
class ConcreteAbstractionA:public Abstraction{
public:
    ConcreteAbstractionA(Implementor* _impl):Abstraction(_impl){}
    void operation() override{
        cout<<"ConcreteAbstractionA operation"<<endl;
        impl->operationImplementation();
    }
};

int main(){
    cout<<"ConcreteAbstractionA:"<<endl;
    Implementor* implA=new ConcreteImplementorA();
    Abstraction* absA=new ConcreteAbstractionA(implA);
    absA->operation();
    delete absA;

    cout<<"\nConcreteAbstractionB:"<<endl;
    Implementor* implB=new ConcreteImplementorB();
    Abstraction* absB=new ConcreteAbstractionA(implB);
    absB->operation();
    delete absB;

    return 0;

}