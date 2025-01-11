#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <thread>
#include <chrono>

// Include our GridMap class
#include "GridMap.h"

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
void displayGrid(const GridMap& map, int droneRow, int droneCol) {
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
            if (i == droneRow && j == droneCol) {
                cout << "| D "; // Drone's current position
            } else if (map.getCell(i, j) == 'X') {
                cout << "| X "; // Fire
            } else {
                cout << "|   "; // Empty cell
            }
        }
        cout << "|" << endl;
    }

    // Print bottom boundary of the last row
    cout << "+";
    for (int j = 0; j < y; j++) {
        cout << "---+";
    }
    cout << endl;
}

int main() {
    srand(static_cast<unsigned int>(time(0))); // Seed for random number generation

    // Define grid dimensions
    int x = 7; // Number of rows
    int y = 7; // Number of columns

    // Create and populate the grid
    GridMap map(x, y);
    map.populateRandomFires(15); // 15% chance of a fire in any cell

    // Initialize the drone's position
    int droneRow = 0, droneCol = 0;

    // Vector to store discovered fires
    vector<pair<int, int>> discoveredFires;

    // Drone's movement through the grid (simple row-by-row scan)
    for (int i = 0; i < x; i++) {
        if (i % 2 == 0) { // Left-to-right for even rows
            for (int j = 0; j < y; j++) {
                droneRow = i;
                droneCol = j;

                // Clear the screen and display the updated grid
                clearScreen();
                displayGrid(map, droneRow, droneCol);

                // Check if the current cell has a fire
                if (map.getCell(droneRow, droneCol) == 'X') {
                    discoveredFires.push_back({droneRow, droneCol});
                    cout << "Fire discovered at: (" << droneRow << ", " << droneCol << ")" << endl;
                }

                // Wait for a moment to simulate animation
                this_thread::sleep_for(chrono::milliseconds(300));
            }
        } else { // Right-to-left for odd rows
            for (int j = y - 1; j >= 0; j--) {
                droneRow = i;
                droneCol = j;

                // Clear the screen and display the updated grid
                clearScreen();
                displayGrid(map, droneRow, droneCol);

                // Check if the current cell has a fire
                if (map.getCell(droneRow, droneCol) == 'X') {
                    discoveredFires.push_back({droneRow, droneCol});
                    cout << "Fire discovered at: (" << droneRow << ", " << droneCol << ")" << endl;
                }

                // Wait for a moment to simulate animation
                this_thread::sleep_for(chrono::milliseconds(300));
            }
        }
    }

    // Display the final list of discovered fires
    clearScreen();
    // Display the grid without the drone
    displayGrid(map, -1, -1);

    cout << "\nAll fires discovered:" << endl;
    for (auto fire : discoveredFires) {
        cout << "(" << fire.first << ", " << fire.second << ")" << endl;
    }

    return 0;
}
