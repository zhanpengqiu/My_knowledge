#include <iostream>
#include <thread>
#include <vector>
#include <sys/epoll.h>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>

#define MAX_EVENTS 10
#define PORT 8080
// 将句柄设置为非阻塞
void set_nonblock(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}
// 处理客户端连接
void handle_client(int client_fd) {
    char buffer[1024];
    // 循环读取并处理来自客户端的数据
    while(true) {
        // read输入客户端句柄、buffer、buffer大小，不断的将客户端的数据读入buffer
        ssize_t read_bytes = read(client_fd, buffer, sizeof(buffer));
        //受到数据之后就将服务端的招呼报文回复给客户端
        if(read_bytes > 0) {
            std::cout << "Received: " << std::string(buffer, read_bytes) << std::endl;
            write(client_fd, buffer, read_bytes); // Echo back to client
        } else {
            close(client_fd);
            break;
        }
    }
}
// 子reactor处理逻辑，输入为服务器侦听到的客户端socket连接句柄
void sub_reactor(int listen_fd) {
    // 创建epoll句柄
    int epoll_fd = epoll_create1(0);
    struct epoll_event event;
    event.events = EPOLLIN | EPOLLET;
    event.data.fd = listen_fd;
    // 将客户端的句柄加入到epoll当中
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listen_fd, &event);

    struct epoll_event events[MAX_EVENTS];
    while(true) {
        // 设置为阻塞态等待客户端数据到来
        int n = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        for(int i = 0; i < n; ++i) {
            if(events[i].data.fd == listen_fd) {
                // Accept new connection
                sockaddr_in client_addr;
                socklen_t client_len = sizeof(client_addr);
                int client_fd = accept(listen_fd, (sockaddr*)&client_addr, &client_len);
                if(client_fd == -1) continue;
                set_nonblock(client_fd);
                event.events = EPOLLIN | EPOLLET;
                event.data.fd = client_fd;
                epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &event);
            } else {
                // 处理来自客户端的数据
                handle_client(events[i].data.fd);
                epoll_ctl(epoll_fd, EPOLL_CTL_DEL, events[i].data.fd, nullptr);
                close(events[i].data.fd);
            }
        }
    }
}

int main() {
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    set_nonblock(listen_fd);
    
    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    
    bind(listen_fd, (sockaddr*)&server_addr, sizeof(server_addr));
    listen(listen_fd, SOMAXCONN);

    // Main reactor loop in the main thread
    int epoll_fd = epoll_create1(0);
    struct epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = listen_fd;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listen_fd, &event);

    std::vector<std::thread> threads;
    const int num_threads = std::thread::hardware_concurrency();
    for(int i = 0; i < num_threads; ++i) {
        threads.emplace_back(sub_reactor, listen_fd);
    }

    struct epoll_event events[MAX_EVENTS];
    while(true) {
        int n = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        for(int i = 0; i < n; ++i) {
            if(events[i].data.fd == listen_fd) {
                // New connection, pass it to a sub reactor
                sockaddr_in client_addr;
                socklen_t client_len = sizeof(client_addr);
                int client_fd = accept(listen_fd, (sockaddr*)&client_addr, &client_len);
                if(client_fd == -1) continue;
                set_nonblock(client_fd);
                
                // Pass the new connection to one of the sub reactors
                // For simplicity, we just print a message here.
                std::cout << "New connection accepted" << std::endl;
            }
        }
    }

    for(auto& t : threads) {
        if(t.joinable()) t.join();
    }

    return 0;
}