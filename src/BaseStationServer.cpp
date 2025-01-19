#include <iostream>
#include <vector>
#include <ctime>
#include <cstdlib>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

int sendAll(SOCKET s, const char* buffer, int length) {
    int totalSent = 0;
    while (totalSent < length) {
        int ret = send(s, buffer + totalSent, length - totalSent, 0);
        if (ret == SOCKET_ERROR || ret == 0) {
            return SOCKET_ERROR;
        }
        totalSent += ret;
    }
    return totalSent;
}

int recvAll(SOCKET s, char* buffer, int length) {
    int totalRecv = 0;
    while (totalRecv < length) {
        int ret = recv(s, buffer + totalRecv, length - totalRecv, 0);
        if (ret == SOCKET_ERROR || ret == 0) {
            return SOCKET_ERROR;
        }
        totalRecv += ret;
    }
    return totalRecv;
}

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed.\n";
        return 1;
    }

    SOCKET serverSock = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSock == INVALID_SOCKET) {
        std::cerr << "socket() failed.\n";
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(12345);
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(serverSock, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "bind() failed. Error: " << WSAGetLastError() << "\n";
        closesocket(serverSock);
        WSACleanup();
        return 1;
    }

    if (listen(serverSock, 1) == SOCKET_ERROR) {
        std::cerr << "listen() failed. Error: " << WSAGetLastError() << "\n";
        closesocket(serverSock);
        WSACleanup();
        return 1;
    }

    std::cout << "[BaseStation] Listening on port 12345...\n";

    sockaddr_in clientAddr;
    int clientSize = sizeof(clientAddr);
    SOCKET clientSock = accept(serverSock, (sockaddr*)&clientAddr, &clientSize);
    if (clientSock == INVALID_SOCKET) {
        std::cerr << "accept() failed. Error: " << WSAGetLastError() << "\n";
        closesocket(serverSock);
        WSACleanup();
        return 1;
    }

    std::cout << "[BaseStation] Drone connected!\n";

    int rows = 15;
    int cols = 15;
    std::vector<std::vector<char>> baseMap(rows, std::vector<char>(cols, ' '));

    srand((unsigned)time(nullptr));
    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            if (rand() % 100 < 10) {
                baseMap[r][c] = 'X';
            }
        }
    }

    {
        int sizeBuf[2] = { rows, cols };
        int byteCount = sizeof(sizeBuf);
        if (sendAll(clientSock, reinterpret_cast<const char*>(sizeBuf), byteCount) == SOCKET_ERROR) {
            std::cerr << "[BaseStation] Error sending map size.\n";
            closesocket(clientSock);
            closesocket(serverSock);
            WSACleanup();
            return 1;
        }
    }

    {
        std::vector<char> flattened(rows * cols);
        for (int r = 0; r < rows; r++) {
            for (int c = 0; c < cols; c++) {
                flattened[r * cols + c] = baseMap[r][c];
            }
        }
        int totalSize = static_cast<int>(flattened.size());
        if (sendAll(clientSock, flattened.data(), totalSize) == SOCKET_ERROR) {
            std::cerr << "[BaseStation] Error sending map data.\n";
            closesocket(clientSock);
            closesocket(serverSock);
            WSACleanup();
            return 1;
        }
    }

    std::cout << "[BaseStation] Sent map to drone. Waiting for discovered fires...\n";

    while (true) {
        int fireCoord[2];
        int bytesNeeded = sizeof(fireCoord);
        int ret = recvAll(clientSock, reinterpret_cast<char*>(fireCoord), bytesNeeded);
        if (ret == SOCKET_ERROR) {
            std::cout << "[BaseStation] Drone disconnected or error. Exiting.\n";
            break;
        }

        int fr = fireCoord[0];
        int fc = fireCoord[1];
        if (fr < 0 || fr >= rows || fc < 0 || fc >= cols) {
            std::cout << "[BaseStation] Received out-of-bounds fire coord: (" << fr << "," << fc << ")\n";
        } else {
            baseMap[fr][fc] = 'X';
            std::cout << "[BaseStation] Discovered fire at (" << fr << "," << fc << ")\n";
        }
    }

    closesocket(clientSock);
    closesocket(serverSock);
    WSACleanup();
    system("pause");
    return 0;
}
