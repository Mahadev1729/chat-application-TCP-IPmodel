#define _WIN32_WINNT 0x0601
#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <thread>

#pragma comment(lib, "ws2_32.lib")

void receiveMessages(SOCKET sock) {
    char buffer[1024];
    while (true) {
        int bytesReceived = recv(sock, buffer, sizeof(buffer), 0);
        if (bytesReceived > 0) {
            buffer[bytesReceived] = '\0';
            std::cout << "\n[Message]: " << buffer << "\n> ";
            std::cout.flush();
        }
    }
}

int main() {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2,2), &wsaData);

    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(54000);
    inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);

    connect(sock, (sockaddr*)&serverAddr, sizeof(serverAddr));

    std::cout << "[Client] Connected to server.\n";

    std::thread(receiveMessages, sock).detach();

    std::string msg;
    while (true) {
        std::cout << "> ";
        std::getline(std::cin, msg);
        send(sock, msg.c_str(), msg.size(), 0);
    }

    closesocket(sock);
    WSACleanup();
    return 0;
}
