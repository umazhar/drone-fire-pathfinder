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

bool sendLine(SOCKET s, const std::string& msg) {
    std::string withNewline = msg + "\n";
    int totalSent = 0;
    while (totalSent < (int)withNewline.size()) {
        int ret = send(s, withNewline.c_str() + totalSent, (int)withNewline.size() - totalSent, 0);
        if (ret == SOCKET_ERROR) return false;
        totalSent += ret;
    }
    return true;
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
    std::cout << "\n";
}

// Show the drone's local map
void displayDroneMap(const GridMap& map,
                     const std::vector<std::vector<bool>>& discovered,
                     int droneRow, int droneCol)
{
    int rows = map.getRows();
    int cols = map.getCols();
    for (int i = 0; i < rows; i++) {
        printRowSeparator(cols);
        for (int j = 0; j < cols; j++) {
            std::cout << "| ";
            if (!discovered[i][j]) {
                std::cout << "? ";
            }
            else if (i == droneRow && j == droneCol) {
                std::cout << "D ";
            }
            else if (map.getCell(i, j) == 'X') {
                std::cout << "X ";
            }
            else {
                std::cout << "  ";
            }
        }
        std::cout << "|\n";
    }
    printRowSeparator(cols);
}

// BFS pathfinding
std::vector<std::pair<int,int>> getPathBFS(const GridMap& map,
                                           int startR, int startC,
                                           int goalR, int goalC)
{
    if (startR == goalR && startC == goalC) {
        return {{startR, startC}};
    }
    int rows = map.getRows();
    int cols = map.getCols();
    if (map.getCell(goalR, goalC) == 'X') {
        return {};
    }
    int DIR[8][2] = {{1,0},{-1,0},{0,1},{0,-1},{1,1},{1,-1},{-1,1},{-1,-1}};
    std::vector<std::vector<bool>> visited(rows, std::vector<bool>(cols, false));
    std::vector<std::vector<std::pair<int,int>>> parent(rows, std::vector<std::pair<int,int>>(cols, {-1,-1}));
    std::queue<std::pair<int,int>> q;
    q.push({startR, startC});
    visited[startR][startC] = true;

    bool found = false;
    while (!q.empty() && !found) {
        auto [r, c] = q.front();
        q.pop();
        for (auto &d : DIR) {
            int nr = r + d[0];
            int nc = c + d[1];
            if (nr<0||nr>=rows||nc<0||nc>=cols) continue;
            if (visited[nr][nc] || map.getCell(nr,nc) == 'X') continue;
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
    std::vector<std::pair<int,int>> path;
    int cr = goalR, cc = goalC;
    while (!(cr==startR && cc==startC)) {
        path.push_back({cr, cc});
        auto p = parent[cr][cc];
        cr = p.first;
        cc = p.second;
    }
    path.push_back({startR, startC});
    std::reverse(path.begin(), path.end());
    return path;
}

bool anyReachableUndiscovered(const GridMap& map,
                              const std::vector<std::vector<bool>>& discovered,
                              int droneRow, int droneCol)
{
    int rows = map.getRows();
    int cols = map.getCols();
    std::vector<std::vector<bool>> visited(rows, std::vector<bool>(cols, false));
    std::queue<std::pair<int,int>> q;
    q.push({droneRow, droneCol});
    visited[droneRow][droneCol] = true;

    int DIR[8][2] = {{1,0},{-1,0},{0,1},{0,-1},{1,1},{1,-1},{-1,1},{-1,-1}};
    while (!q.empty()) {
        auto [r, c] = q.front();
        q.pop();
        if (!discovered[r][c]) {
            return true;
        }
        for (auto &d : DIR) {
            int nr = r + d[0];
            int nc = c + d[1];
            if (nr<0||nr>=rows||nc<0||nc>=cols) continue;
            if (visited[nr][nc] || map.getCell(nr,nc) == 'X') continue;
            visited[nr][nc] = true;
            q.push({nr,nc});
        }
    }
    return false;
}

// When the drone first "sees" a new fire cell, we send "FIRE r c" to the server.
void discoverCells(const GridMap& map,
                   std::vector<std::vector<bool>>& discovered,
                   int droneRow, int droneCol,
                   SOCKET sock)
{
    int rows = map.getRows();
    int cols = map.getCols();
    int perceptionRange = 2;

    for (int dr = -perceptionRange; dr <= perceptionRange; dr++) {
        for (int dc = -perceptionRange; dc <= perceptionRange; dc++) {
            int rr = droneRow + dr;
            int cc = droneCol + dc;
            if (rr >= 0 && rr < rows && cc >= 0 && cc < cols) {
                // Mark discovered
                if (!discovered[rr][cc]) {
                    discovered[rr][cc] = true;
                    if (map.getCell(rr, cc) == 'X') {
                        // We found a new fire => notify server
                        std::string msg = "FIRE " + std::to_string(rr) + " " + std::to_string(cc);
                        sendLine(sock, msg);
                    }
                }
            }
        }
    }
}

int main() {
    // Initialize Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed.\n";
        return 1;
    }

    // Create client socket
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        std::cerr << "socket() failed.\n";
        WSACleanup();
        return 1;
    }

    // Connect to server (localhost:12345)
    sockaddr_in servAddr;
    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_port   = htons(12345);

    if (inet_pton(AF_INET, "127.0.0.1", &servAddr.sin_addr) <= 0) {
        std::cerr << "inet_pton failed.\n";
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
    std::cout << "[Drone] Connected to server!\n\n";

    // Prompt #1: grid size in one line
    int rows, cols;
    std::cout << "Enter the grid size (rows columns): ";
    std::cin >> rows >> cols;

    // Create the drone’s local map
    GridMap map(rows, cols);
    // Populate with random fires (10% chance)
    map.populateRandomFires(10);

    // Prompt #2: drone start coordinate in one line
    int droneRow, droneCol;
    std::cout << "Enter the drone's starting coordinate (row column): ";
    std::cin >> droneRow >> droneCol;

    // Check if coordinates are valid
    if (droneRow < 0 || droneRow >= rows || droneCol < 0 || droneCol >= cols) {
        std::cerr << "[Drone] Error: starting position is out of bounds.\n";
        sendLine(sock, "END");
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    // Check if that cell is on fire
    if (map.getCell(droneRow, droneCol) == 'X') {
        std::cerr << "[Drone] Cannot start at (" << droneRow << "," << droneCol 
                  << ") - it's on fire.\n";
        sendLine(sock, "END");
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    // BFS "discovered" data
    std::vector<std::vector<bool>> discovered(rows, std::vector<bool>(cols, false));

    // Mark neighbors, send FIRE messages for newly seen fires, etc.
    discoverCells(map, discovered, droneRow, droneCol, sock);

    int stepCount = 0;
    double spreadChance = 0.02; // fire spread chance
    bool signalLost = false;
    time_t start = time(0);

    // Scan the entire map row by row
    for (int i = 0; i < rows && !signalLost; i++) {
        if (i % 2 == 0) {
            // left->right
            for (int j = 0; j < cols && !signalLost; j++) {
                if (discovered[i][j]) continue;

                auto path = getPathBFS(map, droneRow, droneCol, i, j);
                if (path.empty()) {
                    // If no path, check if there is any undiscovered cell we can still reach
                    if (!anyReachableUndiscovered(map, discovered, droneRow, droneCol)) {
                        std::cout << "Signal lost!\n";
                        signalLost = true;
                    }
                    continue;
                }
                for (size_t idx = 1; idx < path.size() && !signalLost; idx++) {
                    auto [r, c] = path[idx];

                    // If the next cell is on fire, try a path recalculation
                    if (map.getCell(r,c) == 'X') {
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

                    // Move drone
                    droneRow = r; 
                    droneCol = c;
                    discoverCells(map, discovered, droneRow, droneCol, sock);

                    stepCount++;
                    if (stepCount % 5 == 0) {
                        map.spreadFires(spreadChance);
                    }

                    clearScreen();
                    std::cout << "=== Drone Map ===\n";
                    displayDroneMap(map, discovered, droneRow, droneCol);
                    std::this_thread::sleep_for(std::chrono::milliseconds(300));
                }
            }
        } else {
            // right->left
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

                    // If the next cell is on fire, try a path recalculation
                    if (map.getCell(r,c) == 'X') {
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

                    // Move drone
                    droneRow = r; 
                    droneCol = c;
                    discoverCells(map, discovered, droneRow, droneCol, sock);

                    stepCount++;
                    if (stepCount % 5 == 0) {
                        map.spreadFires(spreadChance);
                    }

                    clearScreen();
                    std::cout << "=== Drone Map ===\n";
                    displayDroneMap(map, discovered, droneRow, droneCol);
                    std::this_thread::sleep_for(std::chrono::milliseconds(300));
                }
            }
        }
    }

    // Display final map if the drone didn't lose signal
    if (!signalLost) {
        clearScreen();
        std::cout << "=== Final Drone Map ===\n";
        displayDroneMap(map, discovered, -1, -1);
    }

    double seconds = difftime(time(0), start);
    std::cout << "Seconds since start: " << seconds << "s\n";
    if (signalLost) {
        std::cout << "\nSimulation ended prematurely (Signal lost!).\n";
    } else {
        std::cout << "\nDone scanning!\n";
    }

    // tell the server we’re done
    sendLine(sock, "END");

    closesocket(sock);
    WSACleanup();
    system("pause");
    
    return 0;
}
