#define _WIN32_WINNT 0x0601
#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")

using namespace std;

int main() {
    WSADATA wsa;
    SOCKET serverSocket, clientSocket;
    sockaddr_in serverAddr, clientAddr;

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2,2), &wsa) != 0) {
        cout << "Failed. Error Code: " << WSAGetLastError() << endl;
        return 1;
    }
    cout << "[Server] Winsock initialized.\n";

    // Create socket
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        cout << "Could not create socket: " << WSAGetLastError() << endl;
        WSACleanup();
        return 1;
    }
    cout << "[Server] Socket created.\n";

    // Setup server address
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(54000);

    // Bind
    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        cout << "Bind failed: " << WSAGetLastError() << endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }
    cout << "[Server] Bind successful.\n";

    // Listen
    listen(serverSocket, 1);
    cout << "[Server] Waiting for client...\n";

    int clientSize = sizeof(clientAddr);
    clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientSize);
    if (clientSocket == INVALID_SOCKET) {
        cout << "Accept failed: " << WSAGetLastError() << endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }
    cout << "[Server] Client connected!\n";

    char buffer[1024];
    while (true) {
        ZeroMemory(buffer, sizeof(buffer));

        // Receive message from client
        int bytesRecv = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesRecv <= 0) {
            cout << "[Server] Client disconnected.\n";
            break;
        }
        cout << "Client: " << buffer << endl;

        // Send reply
        cout << "You (Server): ";
        string reply;
        getline(cin, reply);
        send(clientSocket, reply.c_str(), reply.size() + 1, 0);
    }

    closesocket(clientSocket);
    closesocket(serverSocket);
    WSACleanup();
    return 0;
}
