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

#include "GridMap.h"

using namespace std;

void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void printRowSeparator(int cols) {
    cout << "+";
    for (int j = 0; j < cols; j++) {
        cout << "---+";
    }
    cout << endl;
}

void displayDroneMap(const GridMap& map, const vector<vector<bool>>& discovered, int droneRow, int droneCol) {
    int rows = map.getRows();
    int cols = map.getCols();
    for (int i = 0; i < rows; i++) {
        printRowSeparator(cols);
        for (int j = 0; j < cols; j++) {
            cout << "| ";
            if (!discovered[i][j]) {
                cout << "? ";
            } else if (i == droneRow && j == droneCol) {
                cout << "D ";
            } else if (map.getCell(i, j) == 'X') {
                cout << "X ";
            } else {
                cout << "  ";
            }
        }
        cout << "|" << endl;
    }
    printRowSeparator(cols);
}

void displayBaseStationMap(const vector<vector<char>>& baseStation) {
    int rows = (int)baseStation.size();
    if (rows == 0) return;
    int cols = (int)baseStation[0].size();
    for (int i = 0; i < rows; i++) {
        printRowSeparator(cols);
        for (int j = 0; j < cols; j++) {
            cout << "| ";
            if (baseStation[i][j] == 'X') {
                cout << "X ";
            } else {
                cout << "  ";
            }
        }
        cout << "|" << endl;
    }
    printRowSeparator(cols);
}

void discoverCells(const GridMap& map, vector<vector<bool>>& discovered, vector<vector<char>>& baseStation, int droneRow, int droneCol) {
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
                    baseStation[rr][cc] = 'X';
                }
            }
        }
    }
}

vector<pair<int, int>> getPathBFS(const GridMap& map, int startR, int startC, int goalR, int goalC) {
    if (startR == goalR && startC == goalC) {
        return {{startR, startC}};
    }
    int rows = map.getRows();
    int cols = map.getCols();
    if (map.getCell(goalR, goalC) == 'X') {
        return {};
    }
    int DIR[8][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}, {1, 1}, {1, -1}, {-1, 1}, {-1, -1}};
    vector<vector<bool>> visited(rows, vector<bool>(cols, false));
    vector<vector<pair<int, int>>> parent(rows, vector<pair<int, int>>(cols, {-1, -1}));
    queue<pair<int, int>> q;
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
    vector<pair<int, int>> path;
    int cr = goalR, cc = goalC;
    while (!(cr == startR && cc == startC)) {
        path.push_back({cr, cc});
        auto p = parent[cr][cc];
        cr = p.first;
        cc = p.second;
    }
    path.push_back({startR, startC});
    reverse(path.begin(), path.end());
    return path;
}

bool anyReachableUndiscovered(const GridMap& map, const vector<vector<bool>>& discovered, int droneRow, int droneCol) {
    int rows = map.getRows();
    int cols = map.getCols();
    vector<vector<bool>> visited(rows, vector<bool>(cols, false));
    queue<pair<int, int>> q;
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
    srand((unsigned)time(0));
    time_t start = time(0);
    int rows = 15;
    int cols = 15;
    GridMap map(rows, cols);
    map.populateRandomFires(10);
    vector<vector<bool>> discovered(rows, vector<bool>(cols, false));
    vector<vector<char>> baseStation(rows, vector<char>(cols, ' '));
    if (map.getCell(0, 0) == 'X') {
        cout << "Cannot start at (0,0) because it's on fire.\n";
        return 0;
    }
    int droneRow = 0, droneCol = 0;
    discoverCells(map, discovered, baseStation, droneRow, droneCol);
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
                        cout << "Signal lost!" << endl;
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
                                cout << "Signal lost!" << endl;
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
                    discoverCells(map, discovered, baseStation, droneRow, droneCol);
                    stepCount++;
                    if (stepCount % 5 == 0) {
                        map.spreadFires(spreadChance);
                    }
                    clearScreen();
                    cout << "=== Drone Map (Fog-of-War) ===\n";
                    displayDroneMap(map, discovered, droneRow, droneCol);
                    cout << "\n=== Base Station Map ===\n";
                    displayBaseStationMap(baseStation);
                    this_thread::sleep_for(std::chrono::milliseconds(300));
                }
            }
        } else {
            for (int j = cols - 1; j >= 0 && !signalLost; j--) {
                if (discovered[i][j]) continue;
                auto path = getPathBFS(map, droneRow, droneCol, i, j);
                if (path.empty()) {
                    if (!anyReachableUndiscovered(map, discovered, droneRow, droneCol)) {
                        cout << "Signal lost!" << endl;
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
                                cout << "Signal lost!" << endl;
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
                    discoverCells(map, discovered, baseStation, droneRow, droneCol);
                    stepCount++;
                    if (stepCount % 5 == 0) {
                        map.spreadFires(spreadChance);
                    }
                    clearScreen();
                    cout << "=== Drone Map (Fog-of-War) ===\n";
                    displayDroneMap(map, discovered, droneRow, droneCol);
                    cout << "\n=== Base Station Map ===\n";
                    displayBaseStationMap(baseStation);
                    this_thread::sleep_for(std::chrono::milliseconds(300));
                }
            }
        }
    }
    if (!signalLost) {
        clearScreen();
        cout << "=== Final Drone Map ===\n";
        displayDroneMap(map, discovered, -1, -1);
        cout << "\n=== Final Base Station Map ===\n";
        displayBaseStationMap(baseStation);
    }
    double seconds_since_start = difftime(time(0), start);
    cout << "Seconds since start: " << seconds_since_start << "s\n";
    if (signalLost) {
        cout << "\nSimulation ended prematurely (Signal lost!).\n";
    } else {
        cout << "\nDone scanning!\n";
    }
#ifdef _WIN32
    system("pause");
#endif
    return 0;
}