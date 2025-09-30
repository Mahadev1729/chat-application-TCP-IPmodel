#define _WIN32_WINNT 0x0601
#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <thread>
#include <vector>
#include <mutex>

#pragma comment(lib, "ws2_32.lib")

std::vector<SOCKET> clients;
std::mutex clientsMutex;

void handleClient(SOCKET clientSocket) {
    char buffer[1024];
    while (true) {
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesReceived <= 0) {
            std::lock_guard<std::mutex> lock(clientsMutex);
            closesocket(clientSocket);
            clients.erase(std::remove(clients.begin(), clients.end(), clientSocket), clients.end());
            std::cout << "[Server] Client disconnected." << std::endl;
            break;
        }

        buffer[bytesReceived] = '\0';
        std::string msg = buffer;

        std::cout << "[Client]: " << msg << std::endl;

        // Broadcast to all other clients
        std::lock_guard<std::mutex> lock(clientsMutex);
        for (SOCKET s : clients) {
            if (s != clientSocket) {
                send(s, msg.c_str(), msg.size(), 0);
            }
        }
    }
}

int main() {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2,2), &wsaData);

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(54000);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr));
    listen(serverSocket, SOMAXCONN);

    std::cout << "[Server] Waiting for clients..." << std::endl;

    while (true) {
        sockaddr_in clientAddr;
        int clientSize = sizeof(clientAddr);
        SOCKET clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientSize);

        {
            std::lock_guard<std::mutex> lock(clientsMutex);
            clients.push_back(clientSocket);
        }

        std::cout << "[Server] New client connected." << std::endl;
        std::thread(handleClient, clientSocket).detach();
    }

    closesocket(serverSocket);
    WSACleanup();
    return 0;
}
