#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <thread>
#include <chrono>

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
void displayGrid(char grid[5][5], int droneRow, int droneCol) {
    for (int i = 0; i < 5; i++) {
        cout << "+---+---+---+---+---+" << endl;
        for (int j = 0; j < 5; j++) {
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
    cout << "+---+---+---+---+---+" << endl;
}

// Main program
int main() {
    srand(time(0)); // Seed for random number generation

    // Initialize the grid
    char grid[5][5];
    vector<pair<int, int>> firePositions; // To store fire positions

    // Populate the grid with a 25% chance of a fire ('X') in each cell
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            if (rand() % 100 < 25) { // 25% chance
                grid[i][j] = 'X';
                firePositions.push_back({i, j});
            } else {
                grid[i][j] = ' ';
            }
        }
    }

    // Initialize the drone's position
    int droneRow = 0, droneCol = 0;

    // Vector to store discovered fires
    vector<pair<int, int>> discoveredFires;

    // Drone's movement through the grid
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            droneRow = i;
            droneCol = j;

            // Clear the screen and display the updated grid
            clearScreen();
            displayGrid(grid, droneRow, droneCol);

            // Check if the current cell has a fire
            if (grid[droneRow][droneCol] == 'X') {
                discoveredFires.push_back({droneRow, droneCol});
                cout << "Fire discovered at: (" << droneRow << ", " << droneCol << ")" << endl;
            }

            // Wait for a moment to simulate animation
            this_thread::sleep_for(chrono::milliseconds(300));
        }
    }

    // Display the final list of discovered fires
    clearScreen();
    displayGrid(grid, -1, -1); // Display the grid without the drone
    cout << "\nAll fires discovered:" << endl;
    for (auto fire : discoveredFires) {
        cout << "(" << fire.first << ", " << fire.second << ")" << endl;
    }

    return 0;
}
