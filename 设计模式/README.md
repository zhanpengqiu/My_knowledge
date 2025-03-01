# 设计模式
设计模式分为三大类：
创建型：工厂方法、抽象工厂、单例、建造者、原型
结构型：适配器、装饰器、代理、外观、桥接、组合、享原
行为型：策略、模板方法、观察者、迭代器、责任链、命令、备忘录、状态、访问者、中介者、解释器
## 创建型

### 工厂
工厂模式是一种常用的创建型设计模式，用于解决对象的创建问题。它通过定义一个用于创建对象的接口，将具体的对象创建过程延迟到**子类**中实现。

#### 功能
1. 定义创建对象的接口，封装了对象的创建。
2. 使得具体化类的工作延迟到了子类中。

```cpp
class Vehicle {
public:
    virtual void drive() const = 0; // 纯虚函数，定义产品的行为
    virtual ~Vehicle() {}           // 虚析构函数，确保正确释放资源
};

// 具体产品1：Car
class Car : public Vehicle {
public:
    void drive() const override {
        cout << "Driving a Car!" << endl;
    }
};

// 具体产品2：Motorcycle
class Motorcycle : public Vehicle {
public:
    void drive() const override {
        cout << "Riding a Motorcycle!" << endl;
    }
};

// 具体产品3：Truck
class Truck : public Vehicle {
public:
    void drive() const override {
        cout << "Hauling with a Truck!" << endl;
    }
};

// 工厂类：负责创建具体的产品
class VehicleFactory {
public:
    static Vehicle* createVehicle(const string& type) {
        if (type == "car") {
            return new Car();
        } else if (type == "motorcycle") {
            return new Motorcycle();
        } else if (type == "truck") {
            return new Truck();
        } else {
            cout << "Unknown vehicle type!" << endl;
            return nullptr;
        }
    }
};
```
#### 解释
1. 产品接口 (Vehicle)：定义了一个抽象类 Vehicle，所有的具体产品（如 Car、Motorcycle 和 Truck）都继承自该接口。
2. 具体产品：Car、Motorcycle 和 Truck 是具体的产品类，分别实现了 drive() 方法。
3. 工厂类 (VehicleFactory)：提供了一个静态方法 createVehicle，根据传入的类型字符串创建对应的具体产品实例。
4. 客户端代码 (main)：通过工厂类创建不同的产品，并调用它们的行为。

总结：将对象创建过程延迟到子类中实现

### 抽象工厂
抽象工厂模式是一种创建型设计模式，它提供了一种方式来创建一系列相关或依赖的对象，而无需指定它们具体的类。与简单工厂或工厂方法不同的是，抽象工厂负责创建一组产品，而不是单一的产品。

#### 代码
```cpp
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

```
#### 解释
1. 抽象产品：AbstractProductA 和 AbstractProductB 是两个抽象产品类，分别定义了产品的接口。
2. 具体产品：ConcreteProductA1, ConcreteProductA2, ConcreteProductB1, ConcreteProductB2 是具体的产品类，实现了抽象产品的接口。
3. 抽象工厂：AbstractFactory 定义了创建抽象产品的接口。
4. 具体工厂：ConcreteFactory1 和 ConcreteFactory2 是具体工厂类，负责创建具体的产品实例。
5. 客户端代码：clientCode 函数展示了如何使用抽象工厂来创建和使用产品。

### 单例

### 建造者
建造者模式（Builder Pattern）是一种创建型设计模式，它允许你逐步构建复杂对象，而无需将所有构造参数一次性传递给构造函数。通过使用不同的具体建造者，可以构建出不同表示的对象。

#### 代码
```cpp
// 产品类：表示要构建的复杂对象
class Product {
private:
    std::string partA;
    std::string partB;
    std::string partC;

public:
    void setPartA(const std::string& value) { partA = value; }
    void setPartB(const std::string& value) { partB = value; }
    void setPartC(const std::string& value) { partC = value; }

    void showProduct() const {
        std::cout << "Product Parts:" << std::endl;
        if (!partA.empty()) std::cout << "Part A: " << partA << std::endl;
        if (!partB.empty()) std::cout << "Part B: " << partB << std::endl;
        if (!partC.empty()) std::cout << "Part C: " << partC << std::endl;
    }
};

// 抽象建造者：定义创建产品各个部分的接口
class Builder {
public:
    virtual ~Builder() = default;
    virtual void buildPartA() = 0;
    virtual void buildPartB() = 0;
    virtual void buildPartC() = 0;
    virtual std::unique_ptr<Product> getProduct() = 0;
};

// 具体建造者1：实现抽象建造者的接口
class ConcreteBuilder1 : public Builder {
private:
    std::unique_ptr<Product> product;

public:
    ConcreteBuilder1() : product(std::make_unique<Product>()) {}

    void buildPartA() override {
        product->setPartA("ConcreteBuilder1 Part A");
    }

    void buildPartB() override {
        product->setPartB("ConcreteBuilder1 Part B");
    }

    void buildPartC() override {
        product->setPartC("ConcreteBuilder1 Part C");
    }

    std::unique_ptr<Product> getProduct() override {
        return std::move(product);
    }
};

// 具体建造者2：实现抽象建造者的接口
class ConcreteBuilder2 : public Builder {
private:
    std::unique_ptr<Product> product;

public:
    ConcreteBuilder2() : product(std::make_unique<Product>()) {}

    void buildPartA() override {
        product->setPartA("ConcreteBuilder2 Part A");
    }

    void buildPartB() override {
        product->setPartB("ConcreteBuilder2 Part B");
    }

    void buildPartC() override {
        product->setPartC("ConcreteBuilder2 Part C");
    }

    std::unique_ptr<Product> getProduct() override {
        return std::move(product);
    }
};

// 指挥者：指导建造过程
class Director {
public:
    void construct(Builder& builder) {
        builder.buildPartA();
        builder.buildPartB();
        builder.buildPartC();
    }
};
```

#### 解释
1. 产品类 (Product)：表示要构建的复杂对象，包含多个部分（如 partA, partB, partC）。
2. 抽象建造者 (Builder)：定义了创建产品各个部分的接口。
3. 具体建造者 (ConcreteBuilder1 和 ConcreteBuilder2)：实现了抽象建造者的接口，负责构建产品的具体部分。
4. 指挥者 (Director)：负责指导建造过程，调用建造者的具体方法来逐步构建产品。
5. 客户端代码：在 main 函数中，创建指挥者和具体建造者，并通过指挥者构建最终的产品。

### 原型


### 适配器
适配器模式（Adapter Pattern）是一种结构型设计模式，它允许你将一个类的接口转换成客户端期望的另一个接口。适配器模式使得原本由于接口不兼容而不能一起工作的类可以协同工作。
#### 代码
```cpp
// 目标接口：定义客户端期望的接口
class TargetInterface {
public:
    virtual ~TargetInterface() = default;
    virtual void request() const = 0; // 客户端需要调用的方法
};

// 旧系统类：具有不兼容的接口
class OldSystem {
public:
    void specificRequest() const {
        std::cout << "OldSystem: Specific Request" << std::endl;
    }
};

// 适配器类：将旧系统的接口适配为目标接口
class Adapter : public TargetInterface {
private:
    OldSystem oldSystem;

public:
    void request() const override {
        // 调用旧系统的特定方法
        oldSystem.specificRequest();
    }
};

// 客户端代码：只依赖目标接口
void clientCode(const TargetInterface& target) {
    target.request(); // 客户端只需要调用目标接口的方法
}
```
#### 解释
1. 目标接口 (TargetInterface)：定义了客户端期望的接口，包含 request() 方法。
2. 旧系统类 (OldSystem)：表示现有的系统或类，其接口与目标接口不兼容，包含 specificRequest() 方法。
3. 适配器类 (Adapter)：实现了目标接口，并持有旧系统的实例。适配器通过调用旧系统的 specificRequest() 方法来满足目标接口的要求。
4. 客户端代码：只依赖目标接口，不需要知道具体实现。