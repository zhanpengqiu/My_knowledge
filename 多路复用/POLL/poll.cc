#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <poll.h>

#define PORT 8080
#define BACKLOG 10
#define MAX_CLIENTS 1024

struct client {
    int fd;
    struct pollfd pfd;
};

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    struct pollfd fds[MAX_CLIENTS + 1]; // +1 for the listening socket
    struct client clients[MAX_CLIENTS];
    int nfds = 1; // 初始值为1，因为至少有一个监听socket

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

    // 初始化pollfd结构体数组的第一个元素，用于监听服务器socket
    fds[0].fd = server_fd;
    fds[0].events = POLLIN;

    printf("Waiting for connections on port %d...\n", PORT);

    while (1) {
        int poll_count = poll(fds, nfds, -1); // 等待直到有事件发生
        if (poll_count < 0) {
            perror("poll error");
            break;
        }

        for (int i = 0; i < nfds; i++) {
            // 循环检测fds中的事件，检测是否有POLLIN事件到来
            // 如有到来，则执行相应的事件逻辑
            if (fds[i].revents & POLLIN) { // 如果该文件描述符准备好可读
                // 如果是服务端接收到新的连接请求的话
                if (fds[i].fd == server_fd) {
                    // 处理新的连接请求
                    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
                        perror("accept failed");
                        continue;
                    }
                    printf("New connection, socket fd is %d, ip is : %s, port: %d\n",
                           new_socket, inet_ntoa(address.sin_addr), ntohs(address.sin_port));

                    // 将新连接添加到poll监控列表中
                    // poll监控列表的大小是认为设定的
                    if (nfds < MAX_CLIENTS + 1) {
                        fds[nfds].fd = new_socket;
                        fds[nfds].events = POLLIN;
                        clients[nfds - 1].fd = new_socket;
                        clients[nfds - 1].pfd = fds[nfds];
                        nfds++;
                    } else {
                        printf("Too many clients.\n");
                        close(new_socket);
                    }
                } else {
                    // 处理客户端的数据
                    char buffer[1024] = {0};
                    // fds[i].fd获得socket句柄调用系统调用读取数据到buffer中
                    int valread = read(fds[i].fd, buffer, 1024);
                    if (valread <= 0) {
                        // 客户端关闭连接或其他错误
                        printf("Client disconnected, socket fd is %d\n", fds[i].fd);
                        close(fds[i].fd);
                        // 从fds和clients数组中移除这个客户端
                        for (int j = i; j < nfds - 1; j++) {
                            fds[j] = fds[j + 1];
                            clients[j] = clients[j + 1];
                        }
                        nfds--;
                    } else {
                        printf("Received data: %s from socket fd: %d\n", buffer, fds[i].fd);
                        write(fds[i].fd, buffer, strlen(buffer)); // 回显给客户端
                    }
                }
            }
        }
    }

    close(server_fd);
    return 0;
}