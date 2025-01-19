#include <iostream>
#include <vector>
#include <queue>
#include <utility>
#include <cstdlib>
#include <ctime>
#include <thread>
#include <chrono>
#include <algorithm>
#include <time.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#include "GridMap.h"

int sendAll(SOCKET s, const char* buffer, int length) {
    int totalSent = 0;
    while (totalSent < length) {
        int ret = send(s, buffer + totalSent, length - totalSent, 0);
        if (ret == SOCKET_ERROR) return SOCKET_ERROR;
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

void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void printRowSeparator(int cols) {
    std::cout << "+";
    for (int j = 0; j < cols; j++) {
        std::cout << "---+";
    }
    std::cout << std::endl;
}

void displayDroneMap(const GridMap& map, const std::vector<std::vector<bool>>& discovered, int droneRow, int droneCol) {
    int rows = map.getRows();
    int cols = map.getCols();
    for (int i = 0; i < rows; i++) {
        printRowSeparator(cols);
        for (int j = 0; j < cols; j++) {
            std::cout << "| ";
            if (!discovered[i][j]) {
                std::cout << "? ";
            } else if (i == droneRow && j == droneCol) {
                std::cout << "D ";
            } else if (map.getCell(i, j) == 'X') {
                std::cout << "X ";
            } else {
                std::cout << "  ";
            }
        }
        std::cout << "|\n";
    }
    printRowSeparator(cols);
}

void displayBaseStationMap(const std::vector<std::vector<char>>& baseStation) {
    int rows = (int)baseStation.size();
    if (rows == 0) return;
    int cols = (int)baseStation[0].size();
    for (int i = 0; i < rows; i++) {
        printRowSeparator(cols);
        for (int j = 0; j < cols; j++) {
            std::cout << "| ";
            if (baseStation[i][j] == 'X') {
                std::cout << "X ";
            } else {
                std::cout << "  ";
            }
        }
        std::cout << "|\n";
    }
    printRowSeparator(cols);
}

void discoverCells(const GridMap& map, std::vector<std::vector<bool>>& discovered, std::vector<std::vector<char>>& baseStation, int droneRow, int droneCol, SOCKET sock) {
    int rows = map.getRows();
    int cols = map.getCols();
    int perceptionRange = 2;
    for (int dr = -perceptionRange; dr <= perceptionRange; dr++) {
        for (int dc = -perceptionRange; dc <= perceptionRange; dc++) {
            int rr = droneRow + dr;
            int cc = droneCol + dc;
            if (rr >= 0 && rr < rows && cc >= 0 && cc < cols) {
                discovered[rr][cc] = true;
                if (map.getCell(rr, cc) == 'X') {
                    if (baseStation[rr][cc] != 'X') {
                        baseStation[rr][cc] = 'X';
                        int fireCoord[2] = { rr, cc };
                        sendAll(sock, reinterpret_cast<const char*>(fireCoord), sizeof(fireCoord));
                    }
                }
            }
        }
    }
}

std::vector<std::pair<int, int>> getPathBFS(const GridMap& map, int startR, int startC, int goalR, int goalC) {
    if (startR == goalR && startC == goalC) {
        return {{startR, startC}};
    }
    int rows = map.getRows();
    int cols = map.getCols();
    if (map.getCell(goalR, goalC) == 'X') {
        return {};
    }
    int DIR[8][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}, {1, 1}, {1, -1}, {-1, 1}, {-1, -1}};
    std::vector<std::vector<bool>> visited(rows, std::vector<bool>(cols, false));
    std::vector<std::vector<std::pair<int, int>>> parent(rows, std::vector<std::pair<int, int>>(cols, {-1, -1}));
    std::queue<std::pair<int, int>> q;
    q.push({startR, startC});
    visited[startR][startC] = true;
    bool found = false;
    while (!q.empty() && !found) {
        auto [r, c] = q.front();
        q.pop();
        for (auto& d : DIR) {
            int nr = r + d[0];
            int nc = c + d[1];
            if (nr < 0 || nr >= rows || nc < 0 || nc >= cols) {
                continue;
            }
            if (visited[nr][nc] || map.getCell(nr, nc) == 'X') {
                continue;
            }
            visited[nr][nc] = true;
            parent[nr][nc] = {r, c};
            q.push({nr, nc});
            if (nr == goalR && nc == goalC) {
                found = true;
                break;
            }
        }
    }
    if (!found) {
        return {};
    }
    std::vector<std::pair<int, int>> path;
    int cr = goalR, cc = goalC;
    while (!(cr == startR && cc == startC)) {
        path.push_back({cr, cc});
        auto p = parent[cr][cc];
        cr = p.first;
        cc = p.second;
    }
    path.push_back({startR, startC});
    std::reverse(path.begin(), path.end());
    return path;
}

bool anyReachableUndiscovered(const GridMap& map, const std::vector<std::vector<bool>>& discovered, int droneRow, int droneCol) {
    int rows = map.getRows();
    int cols = map.getCols();
    std::vector<std::vector<bool>> visited(rows, std::vector<bool>(cols, false));
    std::queue<std::pair<int, int>> q;
    q.push({droneRow, droneCol});
    visited[droneRow][droneCol] = true;
    int DIR[8][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}, {1, 1}, {1, -1}, {-1, 1}, {-1, -1}};
    while (!q.empty()) {
        auto [r, c] = q.front();
        q.pop();
        if (!discovered[r][c]) {
            return true;
        }
        for (auto& d : DIR) {
            int nr = r + d[0];
            int nc = c + d[1];
            if (nr < 0 || nr >= rows || nc < 0 || nc >= cols) {
                continue;
            }
            if (visited[nr][nc] || map.getCell(nr, nc) == 'X') {
                continue;
            }
            visited[nr][nc] = true;
            q.push({nr, nc});
        }
    }
    return false;
}

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed.\n";
        return 1;
    }
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        std::cerr << "socket() failed.\n";
        WSACleanup();
        return 1;
    }
    sockaddr_in servAddr;
    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(12345);
    if (inet_pton(AF_INET, "127.0.0.1", &servAddr.sin_addr) <= 0) {
        std::cerr << "inet_pton() failed.\n";
        closesocket(sock);
        WSACleanup();
        return 1;
    }
    if (connect(sock, (sockaddr*)&servAddr, sizeof(servAddr)) == SOCKET_ERROR) {
        std::cerr << "connect() failed.\n";
        closesocket(sock);
        WSACleanup();
        return 1;
    }
    std::cout << "[Drone] Connected to BaseStation!\n";
    int sizeBuf[2];
    if (recvAll(sock, reinterpret_cast<char*>(sizeBuf), sizeof(sizeBuf)) == SOCKET_ERROR) {
        std::cerr << "[Drone] Error receiving map size.\n";
        closesocket(sock);
        WSACleanup();
        return 1;
    }
    int rows = sizeBuf[0];
    int cols = sizeBuf[1];
    std::cout << "[Drone] Map size: " << rows << " x " << cols << "\n";
    std::vector<char> mapData(rows * cols);
    if (recvAll(sock, mapData.data(), (int)mapData.size()) == SOCKET_ERROR) {
        std::cerr << "[Drone] Error receiving map data.\n";
        closesocket(sock);
        WSACleanup();
        return 1;
    }
    GridMap map(rows, cols);
    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            map.setCell(r, c, mapData[r * cols + c]);
        }
    }
    std::cout << "[Drone] Received map. Starting BFS scanning...\n";
    time_t startTime = time(0);
    std::vector<std::vector<bool>> discovered(rows, std::vector<bool>(cols, false));
    std::vector<std::vector<char>> baseStation(rows, std::vector<char>(cols, ' '));
    if (map.getCell(0, 0) == 'X') {
        std::cout << "Cannot start at (0,0). It's on fire.\n";
        closesocket(sock);
        WSACleanup();
        return 0;
    }
    int droneRow = 0, droneCol = 0;
    discoverCells(map, discovered, baseStation, droneRow, droneCol, sock);
    int stepCount = 0;
    double spreadChance = 0.013;
    bool signalLost = false;
    for (int i = 0; i < rows && !signalLost; i++) {
        if (i % 2 == 0) {
            for (int j = 0; j < cols && !signalLost; j++) {
                if (discovered[i][j]) continue;
                auto path = getPathBFS(map, droneRow, droneCol, i, j);
                if (path.empty()) {
                    if (!anyReachableUndiscovered(map, discovered, droneRow, droneCol)) {
                        std::cout << "Signal lost!\n";
                        signalLost = true;
                    }
                    continue;
                }
                for (size_t idx = 1; idx < path.size() && !signalLost; idx++) {
                    auto [r, c] = path[idx];
                    if (map.getCell(r, c) == 'X') {
                        auto newPath = getPathBFS(map, droneRow, droneCol, i, j);
                        if (newPath.empty()) {
                            if (!anyReachableUndiscovered(map, discovered, droneRow, droneCol)) {
                                std::cout << "Signal lost!\n";
                                signalLost = true;
                            }
                            break;
                        } else {
                            path = newPath;
                            idx = 0;
                            continue;
                        }
                    }
                    droneRow = r;
                    droneCol = c;
                    discoverCells(map, discovered, baseStation, droneRow, droneCol, sock);
                    stepCount++;
                    if (stepCount % 5 == 0) {
                        map.spreadFires(spreadChance);
                    }
                    clearScreen();
                    std::cout << "=== Drone Map (Fog-of-War) ===\n";
                    displayDroneMap(map, discovered, droneRow, droneCol);
                    std::cout << "\n=== Base Station Map (Local Copy) ===\n";
                    displayBaseStationMap(baseStation);
                    std::this_thread::sleep_for(std::chrono::milliseconds(300));
                }
            }
        } else {
            for (int j = cols - 1; j >= 0 && !signalLost; j--) {
                if (discovered[i][j]) continue;
                auto path = getPathBFS(map, droneRow, droneCol, i, j);
                if (path.empty()) {
                    if (!anyReachableUndiscovered(map, discovered, droneRow, droneCol)) {
                        std::cout << "Signal lost!\n";
                        signalLost = true;
                    }
                    continue;
                }
                for (size_t idx = 1; idx < path.size() && !signalLost; idx++) {
                    auto [r, c] = path[idx];
                    if (map.getCell(r, c) == 'X') {
                        auto newPath = getPathBFS(map, droneRow, droneCol, i, j);
                        if (newPath.empty()) {
                            if (!anyReachableUndiscovered(map, discovered, droneRow, droneCol)) {
                                std::cout << "Signal lost!\n";
                                signalLost = true;
                            }
                            break;
                        } else {
                            path = newPath;
                            idx = 0;
                            continue;
                        }
                    }
                    droneRow = r;
                    droneCol = c;
                    discoverCells(map, discovered, baseStation, droneRow, droneCol, sock);
                    stepCount++;
                    if (stepCount % 5 == 0) {
                        map.spreadFires(spreadChance);
                    }
                    clearScreen();
                    std::cout << "=== Drone Map (Fog-of-War) ===\n";
                    displayDroneMap(map, discovered, droneRow, droneCol);
                    std::cout << "\n=== Base Station Map ===\n";
                    displayBaseStationMap(baseStation);
                    std::this_thread::sleep_for(std::chrono::milliseconds(300));
                }
            }
        }
    }
    if (!signalLost) {
        clearScreen();
        std::cout << "=== Final Drone Map ===\n";
        displayDroneMap(map, discovered, -1, -1);
        std::cout << "\n=== Final Base Station Map ===\n";
        displayBaseStationMap(baseStation);
    }
    double seconds_since_start = difftime(time(0), startTime);
    std::cout << "Seconds since start: " << seconds_since_start << "s\n";
    if (signalLost) {
        std::cout << "\nSimulation ended prematurely (Signal lost!).\n";
    } else {
        std::cout << "\nDone scanning!\n";
    }
    closesocket(sock);
    WSACleanup();
    system("pause");
    return 0;
}