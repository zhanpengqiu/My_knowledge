#include <iostream>
#include <string>

using namespace std;

class Component{
public:
    virtual ~Component()=default;
    virtual void operation() const=0;
};

class ConcreteComponent:public Component{
public:
    void operation() const override{
        cout<<"ConcreteComponent operation"<<endl;
    }
};

class Decorator:public Component{
protected:
    Component* component;
public:
    explicit Decorator(Component* comp):component(comp) {}
    ~Decorator() override{
        delete component;
    }

    void operation() const override{
        component->operation();
    }
};

class ConcreteDecoratorA:public Decorator{
public:
    explicit ConcreteDecoratorA(Component* comp):Decorator(comp) {}

    void operation() const {
        Decorator::operation();
        addedBehaviorA();
    }

    void addedBehaviorA()const {
        cout<<"ConcreteDecoratorA added behavior A"<<endl;
    }
};

class ConcreteDecoratorB : public Decorator {
public:
    explicit ConcreteDecoratorB(Component* comp) : Decorator(comp) {}

    void operation() const override {
        Decorator::operation(); // 执行被装饰组件的操作
        addedBehaviorB();       // 添加额外的行为
    }

    void addedBehaviorB() const {
        cout << "ConcreteDecoratorB: Added behavior B" << endl;
    }
};

int main(){
    // 创建具体组件
    Component* baseComponent = new ConcreteComponent();

    // 使用具体装饰器A包装组件
    Component* decoratedComponentA = new ConcreteDecoratorA(baseComponent);

    // 使用具体装饰器B进一步包装
    Component* decoratedComponentB = new ConcreteDecoratorB(decoratedComponentA);

    // 调用最终装饰后的操作
    cout << "Executing decorated operations:" << endl;
    decoratedComponentB->operation();

    // 清理资源
    delete decoratedComponentB;

    return 0;
}