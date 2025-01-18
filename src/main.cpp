#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <thread>
#include <chrono>
#include "GridMap.h"

using namespace std;

// Function to clear the screen
void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear"); 
#endif
}

// Updated display function to handle "discovered" status.
void displayGrid(const GridMap& map,
                 int droneRow, int droneCol,
                 const vector<vector<bool>>& discovered)
{
    int x = map.getRows();
    int y = map.getCols();

    for (int i = 0; i < x; i++) {
        // Print top boundary of each row
        cout << "+";
        for (int j = 0; j < y; j++) {
            cout << "---+";
        }
        cout << endl;

        // Print row content
        for (int j = 0; j < y; j++) {
            // If undiscovered, display "?"
            if (!discovered[i][j]) {
                cout << "| ? ";
            }
            else if (i == droneRow && j == droneCol) {
                cout << "| D "; // Drone's current position
            }
            else if (map.getCell(i, j) == 'X') {
                cout << "| X "; // Fire
            }
            else {
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

int main() {
    srand(static_cast<unsigned int>(time(0))); // Seed random

    // grid dimensions
    int x = 7; 
    int y = 7; 

    // Create and populate the grid
    GridMap map(x, y);
    map.populateRandomFires(15);
    int droneRow = 0, droneCol = 0;

    // Prepare a 2D "discovered" array, initialized to false
    vector<vector<bool>> discovered(x, vector<bool>(y, false));
    auto discoverNearbyCells = [&](int row, int col) {
        for (int i = row - 1; i <= row + 1; i++) {
            for (int j = col - 1; j <= col + 1; j++) {
                if (i >= 0 && i < x && j >= 0 && j < y) {
                    discovered[i][j] = true;
                }
            }
        }
    };

    // Vector to store discovered fires
    vector<pair<int, int>> discoveredFires;

    // Drone's movement through the grid (simple row-by-row scan)
    for (int i = 0; i < x; i++) {
        if (i % 2 == 0) { 
            for (int j = 0; j < y; j++) {
                droneRow = i;
                droneCol = j;
                discoverNearbyCells(droneRow, droneCol);
                clearScreen();
                displayGrid(map, droneRow, droneCol, discovered);
                if (map.getCell(droneRow, droneCol) == 'X') {
                    discoveredFires.push_back({droneRow, droneCol});
                    cout << "Fire discovered at: (" << droneRow << ", " << droneCol << ")" << endl;
                }
                this_thread::sleep_for(chrono::milliseconds(300));
            }
        } else { // odd rows
            for (int j = y - 1; j >= 0; j--) {
                droneRow = i;
                droneCol = j;
                discoverNearbyCells(droneRow, droneCol);
                clearScreen();
                displayGrid(map, droneRow, droneCol, discovered);
                if (map.getCell(droneRow, droneCol) == 'X') {
                    discoveredFires.push_back({droneRow, droneCol});
                    cout << "Fire discovered at: (" << droneRow << ", " << droneCol << ")" << endl;
                }
                this_thread::sleep_for(chrono::milliseconds(300));
            }
        }
    }

    clearScreen();
    displayGrid(map, -1, -1, discovered);

    cout << "\nAll fires discovered:" << endl;
    for (auto fire : discoveredFires) {
        cout << "(" << fire.first << ", " << fire.second << ")" << endl;
    }

    return 0;
}
