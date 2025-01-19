#include <iostream>
#include <vector>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

static const int PORT = 12345;

// Clears the console
void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

// Print a row separator for a grid
void printRowSeparator(int cols) {
    std::cout << "+";
    for (int j = 0; j < cols; j++) {
        std::cout << "---+";
    }
    std::cout << "\n";
}

// Display the server’s blank grid with discovered fires
void displayServerMap(const std::vector<std::vector<char>>& serverMap) {
    int rows = (int)serverMap.size();
    if (rows == 0) return;
    int cols = (int)serverMap[0].size();

    for (int r = 0; r < rows; r++) {
        printRowSeparator(cols);
        for (int c = 0; c < cols; c++) {
            std::cout << "| ";
            if (serverMap[r][c] == 'X') {
                std::cout << "X ";
            } else {
                std::cout << "  ";
            }
        }
        std::cout << "|\n";
    }
    printRowSeparator(cols);
}

// Receive a single line
bool recvLine(SOCKET s, std::string& out) {
    out.clear();
    char c;
    while (true) {
        int ret = recv(s, &c, 1, 0);
        if (ret <= 0) {
            // Error or disconnected
            return false;
        }
        if (c == '\n') {
            break;
        }
        out.push_back(c);
    }
    return true;
}

int main() {
    // init winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed.\n";
        return 1;
    }

    //Create a TCP socket
    SOCKET serverSock = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSock == INVALID_SOCKET) {
        std::cerr << "socket() failed.\n";
        WSACleanup();
        return 1;
    }

    // bind to port 12345
    sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family      = AF_INET;
    serverAddr.sin_port        = htons(PORT);
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(serverSock, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "bind() failed.\n";
        closesocket(serverSock);
        WSACleanup();
        return 1;
    }

    // Listen
    if (listen(serverSock, 1) == SOCKET_ERROR) {
        std::cerr << "listen() failed.\n";
        closesocket(serverSock);
        WSACleanup();
        return 1;
    }

    std::cout << "[Server] Listening on port " << PORT << "...\n";

    //Accept the drone’s connection
    sockaddr_in clientAddr;
    int clientSize = sizeof(clientAddr);
    SOCKET clientSock = accept(serverSock, (sockaddr*)&clientAddr, &clientSize);
    if (clientSock == INVALID_SOCKET) {
        std::cerr << "accept() failed.\n";
        closesocket(serverSock);
        WSACleanup();
        return 1;
    }

    std::cout << "[Server] Drone connected!\n";

    // We create a blank 15x15 grid on the server
    int rows = 15;
    int cols = 15;
    std::vector<std::vector<char>> serverMap(rows, std::vector<char>(cols, ' '));

    //    We expect lines like: "FIRE r c" or "END"
    bool running = true;
    while (running) {
        std::string line;
        if (!recvLine(clientSock, line)) {
            std::cout << "[Server] Drone disconnected or error.\n";
            break;
        }

        if (line.rfind("FIRE", 0) == 0) {
            // line example: "FIRE 3 5"
            int r, c;
            if (sscanf(line.c_str(), "FIRE %d %d", &r, &c) == 2) {
                // Mark that cell as discovered fire
                if (r >= 0 && r < rows && c >= 0 && c < cols) {
                    serverMap[r][c] = 'X';
                }

                // Re-display
                clearScreen();
                std::cout << "[Server] Fire discovered at (" << r << "," << c << ")\n";
                displayServerMap(serverMap);
            }
        }
        else if (line == "END") {
            std::cout << "[Server] Drone ended scanning.\n";
            running = false;
        }
        else {
            std::cout << "[Server] Unknown command: " << line << "\n";
        }
    }

    // Final display
    std::cout << "[Server] Final discovered map:\n";
    displayServerMap(serverMap);

    // Cleanup
    closesocket(clientSock);
    closesocket(serverSock);
    WSACleanup();
    system("pause");
    return 0;
}
