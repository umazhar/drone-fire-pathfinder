#include <iostream>
#include <vector>
#include <queue>
#include <utility>
#include <cstdlib>
#include <ctime>
#include <thread>
#include <chrono>
#include <algorithm> // for std::reverse

#include "GridMap.h"

using namespace std;

// Clears the console screen
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

/**
 * Display the drone's map with fog-of-war:
 * - 'D' for drone's current position
 * - 'X' for discovered fires
 * - ' ' for discovered empty cells
 * - '?' for undiscovered cells
 */
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
                // Cell not yet discovered
                cout << "? ";
            }
            else if (i == droneRow && j == droneCol) {
                // Drone is here
                cout << "D ";
            }
            else if (map.getCell(i, j) == 'X') {
                // Known fire
                cout << "X ";
            }
            else {
                // Known empty cell
                cout << "  ";
            }
        }
        cout << "|" << endl;
    }
    printRowSeparator(cols);
}

/**
 * Display the base station map:
 * - 'X' for reported fires
 * - ' ' (blank) for everything else (unknown or no fire)
 */
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

/**
 * Mark all cells in a 3×3 block around (droneRow, droneCol) as discovered.
 * If any discovered cell is a fire, update the baseStation map to 'X' there.
 */
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
            // Check boundaries
            if (rr >= 0 && rr < rows && cc >= 0 && cc < cols) {
                discovered[rr][cc] = true;
                if (map.getCell(rr, cc) == 'X') {
                    baseStation[rr][cc] = 'X';  // Mark fire in base station's map
                }
            }
        }
    }
}

/**
 * BFS path from (startR, startC) to (goalR, goalC) avoiding fires.
 * Returns a sequence of (row, col) from start to goal (inclusive) 
 * if possible; otherwise returns an empty vector.
 */
vector<pair<int,int>> getPathBFS(const GridMap& map,
                                 int startR, int startC,
                                 int goalR, int goalC)
{
    // If start == goal, trivial path
    if (startR == goalR && startC == goalC) {
        return {{startR, startC}};
    }

    int rows = map.getRows();
    int cols = map.getCols();

    // If goal is on fire, can't stand there
    if (map.getCell(goalR, goalC) == 'X') {
        return {};
    }

    vector<vector<bool>> visited(rows, vector<bool>(cols, false));
    vector<vector<pair<int,int>>> parent(rows, vector<pair<int,int>>(cols, {-1, -1}));

    queue<pair<int,int>> q;
    q.push({startR, startC});
    visited[startR][startC] = true;

    // 4-directional moves
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
            // skip if visited or on fire
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
        // No path
        return {};
    }

    // Reconstruct path by walking parent[] from goal back to start
    vector<pair<int,int>> path;
    int cr = goalR, cc = goalC;
    while (!(cr == startR && cc == startC)) {
        path.push_back({cr, cc});
        auto p = parent[cr][cc];
        cr = p.first;
        cc = p.second;
    }
    // Add start
    path.push_back({startR, startC});
    // Reverse so it's start -> goal
    reverse(path.begin(), path.end());
    return path;
}

int main() {
    srand((unsigned)time(0));

    // Grid dimensions
    int rows = 15;
    int cols = 15;

    // Create map
    GridMap map(rows, cols);
    map.populateRandomFires(15);

    // Drone's discovered array
    //  - false => not discovered (show as '?')
    //  - true  => discovered
    vector<vector<bool>> discovered(rows, vector<bool>(cols, false));

    // Base station map
    //  - ' ' => not known or no fire
    //  - 'X' => fire discovered & reported
    vector<vector<char>> baseStation(rows, vector<char>(cols, ' '));

    // Start drone at (0,0) if it's not on fire
    if (map.getCell(0,0) == 'X') {
        cout << "Cannot start at (0,0) because it's on fire.\n";
        return 0;
    }

    int droneRow = 0, droneCol = 0;
    // Discover initial position
    discoverCells(map, discovered, baseStation, droneRow, droneCol);

    // Row-by-row scanning
    for (int i = 0; i < rows; i++) {
        if (i % 2 == 0) {
            // Left to right
            for (int j = 0; j < cols; j++) {
                // Skip if already discovered (no need to physically stand there)
                if (discovered[i][j]) continue;

                // Otherwise, try to BFS a path from current (droneRow, droneCol) to (i,j)
                auto path = getPathBFS(map, droneRow, droneCol, i, j);
                if (path.empty()) {
                    // No path => skip
                    continue;
                }

                // Move drone along path, discovering
                // path[0] is the current cell, so start from index=1
                for (size_t idx = 1; idx < path.size(); idx++) {
                    auto [r, c] = path[idx];
                    droneRow = r;
                    droneCol = c;

                    // Discover around new position
                    discoverCells(map, discovered, baseStation, droneRow, droneCol);

                    // Show updated maps
                    clearScreen();
                    cout << "=== Drone Map (Fog-of-War) ===\n";
                    displayDroneMap(map, discovered, droneRow, droneCol);

                    cout << "\n=== Base Station Map ===\n";
                    displayBaseStationMap(baseStation);

                    this_thread::sleep_for(std::chrono::milliseconds(300));
                }
            }
        } else {
            // Right to left
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

    cout << "\nDone scanning!\n";
    return 0;
}
