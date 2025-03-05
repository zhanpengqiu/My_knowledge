#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <fcntl.h>

#define PORT 8080
#define MAX_EVENTS 1024
#define BACKLOG 10

void set_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) {
        perror("fcntl F_GETFL failed");
        return;
    }
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        perror("fcntl F_SETFL failed");
    }
}

int main() {
    int server_fd, new_socket, epoll_fd;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    struct epoll_event event, events[MAX_EVENTS];

    // 创建socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // 设置端口可重用
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // 绑定socket到指定端口
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // 监听连接
    if (listen(server_fd, BACKLOG) < 0) {
        perror("listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // 将服务器socket设置为非阻塞模式
    set_nonblocking(server_fd);

    // 创建epoll实例
    if ((epoll_fd = epoll_create1(0)) < 0) {
        perror("epoll_create1 failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // 将服务器socket添加到epoll实例中
    event.events = EPOLLIN | EPOLLET; // 使用边缘触发
    event.data.fd = server_fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &event) < 0) {
        perror("epoll_ctl failed");
        close(server_fd);
        close(epoll_fd);
        exit(EXIT_FAILURE);
    }

    printf("Waiting for connections on port %d...\n", PORT);
    // =============================核心代码=============================
    while (1) {
        // 获得epoll_create的句柄、事件发生的句柄（存放就绪事件的events，内核将数据放入，用户只需要接受event的事件即可）
        int n = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        for (int i = 0; i < n; i++) {
            if (events[i].data.fd == server_fd) {
                // 处理新的连接请求
                while ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) > 0) {
                    printf("New connection, socket fd is %d, ip is : %s, port: %d\n",
                           new_socket, inet_ntoa(address.sin_addr), ntohs(address.sin_port));

                    set_nonblocking(new_socket); // 设置新连接为非阻塞模式

                    event.events = EPOLLIN | EPOLLET; // 使用边缘触发
                    event.data.fd = new_socket;
                    // 加入新连接事件到epoll当中
                    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, new_socket, &event) < 0) {
                        perror("epoll_ctl failed");
                        close(new_socket);
                    }
                }
                if (new_socket == -1 && errno != EAGAIN && errno != EWOULDBLOCK) {
                    perror("accept failed");
                }
            } else {
                // 处理客户端的数据
                // 处理客户端的数据
                char buffer[1024] = {0};
                int valread = read(events[i].data.fd, buffer, 1023);
                if (valread <= 0) {
                    // 客户端关闭连接或其他错误
                    printf("Client disconnected, socket fd is %d\n", events[i].data.fd);
                    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
                    close(events[i].data.fd);
                } else {
                    printf("Received data: %s from socket fd: %d\n", buffer, events[i].data.fd);
                    write(events[i].data.fd, buffer, strlen(buffer)); // 回显给客户端
                }
            }
        }
    }

    close(server_fd);
    close(epoll_fd);
    return 0;
}