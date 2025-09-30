#define _WIN32_WINNT 0x0601
#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <WinSock2.h>
#include <WS2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")

using namespace std;

vector<SOCKET> clients;     // store all connected clients
mutex clients_mutex;        // protect shared clients list

void handle_client(SOCKET clientSocket) {
    char buffer[1024];

    while (true) {
        ZeroMemory(buffer, sizeof(buffer));
        int bytesRecv = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesRecv <= 0) {
            cout << "[Server] A client disconnected.\n";
            closesocket(clientSocket);

            // Remove client safely
            lock_guard<mutex> guard(clients_mutex);
            clients.erase(remove(clients.begin(), clients.end(), clientSocket), clients.end());
            break;
        }

        cout << "[Message] " << buffer << endl;

        // Broadcast message to all clients
        lock_guard<mutex> guard(clients_mutex);
        for (SOCKET s : clients) {
            if (s != clientSocket) { // don’t send back to the sender
                send(s, buffer, bytesRecv, 0);
            }
        }
    }
}

int main() {
    WSADATA wsa;
    SOCKET serverSocket;
    sockaddr_in serverAddr, clientAddr;

    if (WSAStartup(MAKEWORD(2,2), &wsa) != 0) {
        cout << "Failed. Error Code: " << WSAGetLastError() << endl;
        return 1;
    }
    cout << "[Server] Winsock initialized.\n";

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        cout << "Could not create socket: " << WSAGetLastError() << endl;
        WSACleanup();
        return 1;
    }
    cout << "[Server] Socket created.\n";

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(54000);

    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        cout << "Bind failed: " << WSAGetLastError() << endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    listen(serverSocket, SOMAXCONN);
    cout << "[Server] Listening for connections...\n";

    while (true) {
        int clientSize = sizeof(clientAddr);
        SOCKET clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientSize);
        if (clientSocket == INVALID_SOCKET) {
            cout << "Accept failed: " << WSAGetLastError() << endl;
            continue;
        }

        {
            lock_guard<mutex> guard(clients_mutex);
            clients.push_back(clientSocket);
        }

        cout << "[Server] Client connected.\n";
        thread(handle_client, clientSocket).detach(); // run in background
    }

    closesocket(serverSocket);
    WSACleanup();
    return 0;
}
