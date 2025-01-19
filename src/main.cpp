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

void displayDroneMap(const GridMap& map,
                     const vector<vector<bool>>& discovered,
                     int droneRow, int droneCol)
{
    int rows = map.getRows();
    int cols = map.getCols();

    for (int i = 0; i < rows; i++) {
        printRowSeparator(cols);
        for (int j = 0; j < cols; j++) {
            cout << "| ";
            if (!discovered[i][j]) {
                cout << "? ";
            }
            else if (i == droneRow && j == droneCol) {
                cout << "D ";
            }
            else if (map.getCell(i, j) == 'X') {
                cout << "X ";
            }
            else {
                cout << "  ";
            }
        }
        cout << "|" << endl;
    }
    printRowSeparator(cols);
}

//base station map
void displayBaseStationMap(const vector<vector<char>>& baseStation) {
    int rows = (int) baseStation.size();
    if (rows == 0) return;
    int cols = (int) baseStation[0].size();

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

//marking cells around perception range as discovered
void discoverCells(const GridMap& map,
                   vector<vector<bool>>& discovered,
                   vector<vector<char>>& baseStation,
                   int droneRow, int droneCol)
{
    int rows = map.getRows();
    int cols = map.getCols();

    /*
        perception range size
        1, -1 = 3x3
        2, -2 = 5x5
    
    */
    int perceptionRangeUpperbound = 2;
    int perceptionRangeLowerbound = -2;

    for (int dr = perceptionRangeLowerbound; dr <= perceptionRangeUpperbound; dr++) {
        for (int dc = perceptionRangeLowerbound; dc <= perceptionRangeUpperbound; dc++) {
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

/**
 * BFS path from (startR, startC) to (goalR, goalC) avoiding fires.
 */
vector<pair<int,int>> getPathBFS(const GridMap& map,
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
    vector<vector<bool>> visited(rows, vector<bool>(cols, false));
    vector<vector<pair<int,int>>> parent(rows, vector<pair<int,int>>(cols, {-1, -1}));

    queue<pair<int,int>> q;
    q.push({startR, startC});
    visited[startR][startC] = true;

    int DIR[4][2] = {{1,0},{-1,0},{0,1},{0,-1}};
    
    bool found = false;
    while (!q.empty() && !found) {
        auto [r, c] = q.front();
        q.pop();

        for (auto &d : DIR) {
            int nr = r + d[0];
            int nc = c + d[1];
            // bounds check
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
        //not path
        return {};
    }

    // Reconstruct path 
    vector<pair<int,int>> path;
    int cr = goalR, cc = goalC;
    while (!(cr == startR && cc == startC)) {
        path.push_back({cr, cc});
        auto p = parent[cr][cc];
        cr = p.first;
        cc = p.second;
    }
    path.push_back({startR, startC});
    // Reverse so start -> goal
    reverse(path.begin(), path.end());
    return path;
}

int main() {
    srand((unsigned)time(0));
    time_t start = time(0);


    // Grid dimensions
    int rows = 15;
    int cols = 15;

    // Create map
    GridMap map(rows, cols);
    map.populateRandomFires(15);

    vector<vector<bool>> discovered(rows, vector<bool>(cols, false));

    // Base station map
    vector<vector<char>> baseStation(rows, vector<char>(cols, ' '));
    if (map.getCell(0,0) == 'X') {
        cout << "Cannot start at (0,0) because it's on fire.\n";
        return 0;
    }

    int droneRow = 0, droneCol = 0;
    discoverCells(map, discovered, baseStation, droneRow, droneCol);

    // Row-by-row scanning
    for (int i = 0; i < rows; i++) {
        if (i % 2 == 0) {
            for (int j = 0; j < cols; j++) {
                if (discovered[i][j]) continue;

                // bfs from current to i, j
                auto path = getPathBFS(map, droneRow, droneCol, i, j);
                if (path.empty()) {
                    continue;
                }
                for (size_t idx = 1; idx < path.size(); idx++) {
                    auto [r, c] = path[idx];
                    droneRow = r;
                    droneCol = c;

                    discoverCells(map, discovered, baseStation, droneRow, droneCol);

                    clearScreen();
                    cout << "=== Drone Map (Fog-of-War) ===\n";
                    displayDroneMap(map, discovered, droneRow, droneCol);

                    cout << "\n=== Base Station Map ===\n";
                    displayBaseStationMap(baseStation);

                    this_thread::sleep_for(std::chrono::milliseconds(300));
                }
            }
        } else {
            for (int j = cols - 1; j >= 0; j--) {
                if (discovered[i][j]) continue;

                auto path = getPathBFS(map, droneRow, droneCol, i, j);
                if (path.empty()) {
                    continue;
                }
                for (size_t idx = 1; idx < path.size(); idx++) {
                    auto [r, c] = path[idx];
                    droneRow = r;
                    droneCol = c;
                    discoverCells(map, discovered, baseStation, droneRow, droneCol);

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

    // Final display
    clearScreen();
    cout << "=== Final Drone Map ===\n";
    displayDroneMap(map, discovered, -1, -1);

    cout << "\n=== Final Base Station Map ===\n";
    displayBaseStationMap(baseStation);

    double seconds_since_start = difftime( time(0), start);
    cout << "Seconds since start:" << seconds_since_start;

    cout << "\nDone scanning!\n";
    system("pause");
    return 0;
}
