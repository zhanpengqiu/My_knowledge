#include <iostream>
#include <memory>

// 抽象产品A
class AbstractProductA {
public:
    virtual ~AbstractProductA() = default;
    virtual void use() const = 0;
};

// 具体产品A1
class ConcreteProductA1 : public AbstractProductA {
public:
    void use() const override {
        std::cout << "Using ConcreteProductA1" << std::endl;
    }
};

// 具体产品A2
class ConcreteProductA2 : public AbstractProductA {
public:
    void use() const override {
        std::cout << "Using ConcreteProductA2" << std::endl;
    }
};

// 抽象产品B
class AbstractProductB {
public:
    virtual ~AbstractProductB() = default;
    virtual void use() const = 0;
};

// 具体产品B1
class ConcreteProductB1 : public AbstractProductB {
public:
    void use() const override {
        std::cout << "Using ConcreteProductB1" << std::endl;
    }
};

// 具体产品B2
class ConcreteProductB2 : public AbstractProductB {
public:
    void use() const override {
        std::cout << "Using ConcreteProductB2" << std::endl;
    }
};

// 抽象工厂
class AbstractFactory {
public:
    virtual ~AbstractFactory() = default;
    virtual std::unique_ptr<AbstractProductA> createProductA() const = 0;
    virtual std::unique_ptr<AbstractProductB> createProductB() const = 0;
};

// 具体工厂1
class ConcreteFactory1 : public AbstractFactory {
public:
    std::unique_ptr<AbstractProductA> createProductA() const override {
        return std::make_unique<ConcreteProductA1>();
    }

    std::unique_ptr<AbstractProductB> createProductB() const override {
        return std::make_unique<ConcreteProductB1>();
    }
};

// 具体工厂2
class ConcreteFactory2 : public AbstractFactory {
public:
    std::unique_ptr<AbstractProductA> createProductA() const override {
        return std::make_unique<ConcreteProductA2>();
    }

    std::unique_ptr<AbstractProductB> createProductB() const override {
        return std::make_unique<ConcreteProductB2>();
    }
};

// 客户端代码
void clientCode(const AbstractFactory& factory) {
    auto productA = factory.createProductA();
    auto productB = factory.createProductB();

    productA->use();
    productB->use();
}

int main() {
    // 使用具体工厂1
    ConcreteFactory1 factory1;
    std::cout << "Using ConcreteFactory1:" << std::endl;
    clientCode(factory1);

    // 使用具体工厂2
    ConcreteFactory2 factory2;
    std::cout << "Using ConcreteFactory2:" << std::endl;
    clientCode(factory2);

    return 0;
}