#include <iostream>
#include <string>
#include <map>
using namespace std;

// 产品接口：所有具体产品都继承自这个接口
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

int main() {
    // 使用工厂创建不同的车辆
    Vehicle* car = VehicleFactory::createVehicle("car");
    if (car) {
        car->drive();
        delete car; // 手动释放内存
    }

    Vehicle* motorcycle = VehicleFactory::createVehicle("motorcycle");
    if (motorcycle) {
        motorcycle->drive();
        delete motorcycle;
    }

    Vehicle* truck = VehicleFactory::createVehicle("truck");
    if (truck) {
        truck->drive();
        delete truck;
    }

    Vehicle* unknown = VehicleFactory::createVehicle("unknown");
    if (unknown) {
        unknown->drive();
        delete unknown;
    }

    return 0;
}