#include <iostream>
#include <vector>
#include <queue>
#include <cstdlib>
#include <ctime>
#include <thread>
#include <chrono>
#include <algorithm>  // for std::reverse

using namespace std;

// Function to clear the screen
void clearScreen() {
#ifdef _WIN32
    system("cls"); // For Windows
#else
    system("clear"); // For Unix/Linux/Mac
#endif
}

// Function to display the grid with the drone and fires
void displayGrid(const vector<vector<char>>& grid, int droneRow, int droneCol, int x, int y) {
    for (int i = 0; i < x; i++) {
        cout << "+";
        for (int j = 0; j < y; j++) {
            cout << "---+";
        }
        cout << endl;
        for (int j = 0; j < y; j++) {
            if (i == droneRow && j == droneCol) {
                cout << "| D "; // Drone's current position
            } else if (grid[i][j] == 'X') {
                cout << "| X "; // Fire
            } else {
                cout << "|   "; // Empty cell
            }
        }
        cout << "|" << endl;
    }
    cout << "+";
    for (int j = 0; j < y; j++) {
        cout << "---+";
    }
    cout << endl;
}

// Use BFS to find a path from (startRow, startCol) to the nearest 'X'.
// Returns an empty vector if no fire is reachable, or a list of (row, col) steps otherwise.
vector<pair<int,int>> findPathToNearestFire(const vector<vector<char>>& grid,
                                            int startRow, int startCol)
{
    int rows = grid.size();
    if (rows == 0) return {};
    int cols = grid[0].size();
    
    // Directions: up, down, left, right
    static vector<pair<int,int>> directions = {{-1,0},{1,0},{0,-1},{0,1}};

    // Visited array
    vector<vector<bool>> visited(rows, vector<bool>(cols, false));
    visited[startRow][startCol] = true;

    // Parent map to reconstruct path
    // parent[r][c] = (prevR, prevC) from which we came
    vector<vector<pair<int,int>>> parent(rows, vector<pair<int,int>>(cols, {-1,-1}));

    queue<pair<int,int>> q;
    q.push({startRow, startCol});

    // BFS
    while (!q.empty()) {
        auto [curR, curC] = q.front();
        q.pop();

        // Check if this cell is a fire
        if (grid[curR][curC] == 'X') {
            // Found the nearest fire -> reconstruct path
            vector<pair<int,int>> path;
            int r = curR, c = curC;
            while (!(r == startRow && c == startCol)) {
                path.push_back({r, c});
                auto p = parent[r][c];
                r = p.first;
                c = p.second;
            }
            // Don't forget the start cell
            path.push_back({startRow, startCol});
            // Reverse to get path from start -> fire
            reverse(path.begin(), path.end());
            return path;
        }

        // Otherwise, explore neighbors
        for (auto &d : directions) {
            int nr = curR + d.first;
            int nc = curC + d.second;
            // Check boundaries
            if (nr >= 0 && nr < rows && nc >= 0 && nc < cols) {
                if (!visited[nr][nc]) {
                    visited[nr][nc] = true;
                    parent[nr][nc] = {curR, curC};
                    q.push({nr, nc});
                }
            }
        }
    }

    // No fire found
    return {};
}

int main() {
    srand(static_cast<unsigned int>(time(0))); // Seed for random number generation

    // Define grid dimensions
    int x = 4; // Number of rows
    int y = 8; // Number of columns

    // Initialize the grid using a vector
    vector<vector<char>> grid(x, vector<char>(y, ' '));

    // Populate the grid with a 25% chance of a fire ('X') in each cell
    for (int i = 0; i < x; i++) {
        for (int j = 0; j < y; j++) {
            if (rand() % 100 < 25) {
                grid[i][j] = 'X';
            }
        }
    }

    // Initialize the drone's position (top-left corner for now)
    int droneRow = 0;
    int droneCol = 0;

    // Vector to store discovered fires
    vector<pair<int, int>> discoveredFires;

    while (true) {
        // Find a BFS path to the nearest fire
        vector<pair<int,int>> path = findPathToNearestFire(grid, droneRow, droneCol);

        if (path.empty()) {
            // No more reachable fires
            break;
        }

        // Animate traveling along the path (excluding the start cell, since we’re already there)
        // path[0] is our current location, so we start from path[1] if it exists
        for (int i = 1; i < (int)path.size(); i++) {
            droneRow = path[i].first;
            droneCol = path[i].second;

            clearScreen();
            displayGrid(grid, droneRow, droneCol, x, y);

            // If the current cell is a fire, record it
            if (grid[droneRow][droneCol] == 'X') {
                discoveredFires.push_back({droneRow, droneCol});
                // Mark this cell as discovered, so BFS won't target it again
                grid[droneRow][droneCol] = ' '; 
            }

            this_thread::sleep_for(chrono::milliseconds(300));
        }
    }

    // Done searching for fires or no fires reachable
    clearScreen();
    // Show final grid (drone position can be -1 to hide it, or keep last known position)
    displayGrid(grid, -1, -1, x, y);

    cout << "\nAll discovered fires:" << endl;
    if (discoveredFires.empty()) {
        cout << "No fires found or no fires were reachable." << endl;
    } else {
        for (auto &fire : discoveredFires) {
            cout << "(" << fire.first << ", " << fire.second << ")" << endl;
        }
    }

    return 0;
}
