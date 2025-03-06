#include <iostream>
#include <thread>
#include <vector>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

void client_task(int client_id, const char* ip, int port) {
    int sock = 0;
    struct sockaddr_in serv_addr;
    const char* hello = "Hello from client";
    char buffer[1024] = {0};

    // 创建socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cerr << "Client " << client_id << ": Socket creation error\n";
        return;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    // 将IP地址从文本转换为二进制形式
    if (inet_pton(AF_INET, ip, &serv_addr.sin_addr) <= 0) {
        std::cerr << "Client " << client_id << ": Invalid address/ Address not supported\n";
        return;
    }

    // 连接到服务器
    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "Client " << client_id << ": Connection failed\n";
        return;
    }

    // 发送消息给服务器
    send(sock, hello, strlen(hello), 0);
    std::cout << "Client " << client_id << ": Hello message sent\n";

    // 接收服务器的响应
    read(sock, buffer, 1024);
    std::cout << "Client " << client_id << ": Server response: " << buffer << std::endl;

    close(sock); // 关闭socket
}

int main() {
    const int num_clients = 100; // 要启动的客户端数量
    const char* server_ip = "127.0.0.1"; // 服务器IP地址
    int server_port = 8080; // 服务器端口

    std::vector<std::thread> clients;
    for(int i = 0; i < num_clients; ++i) {
        clients.emplace_back(client_task, i + 1, server_ip, server_port);
    }

    // 等待所有客户端线程完成
    for(auto& t : clients) {
        if(t.joinable()) t.join();
    }

    return 0;
}