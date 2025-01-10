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

// Main program
int main() {
    srand(static_cast<unsigned int>(time(0))); // Seed for random number generation

    // Define grid dimensions
    int x = 15; // Number of rows
    int y = 15; // Number of columns

    // Initialize the grid using a vector
    vector<vector<char>> grid(x, vector<char>(y, ' '));
    vector<pair<int, int>> firePositions; // To store fire positions

    // Populate the grid with a 25% chance of a fire ('X') in each cell
    for (int i = 0; i < x; i++) {
        for (int j = 0; j < y; j++) {
            if (rand() % 100 < 15) { // 15% chance
                grid[i][j] = 'X';
                firePositions.push_back({i, j});
            }
        }
    }

    // Initialize the drone's position
    int droneRow = 0, droneCol = 0;

    // Vector to store discovered fires
    vector<pair<int, int>> discoveredFires;

    // Drone's movement through the grid
    for (int i = 0; i < x; i++) {
        if (i % 2 == 0) { // Left-to-right for even rows
            for (int j = 0; j < y; j++) {
                droneRow = i;
                droneCol = j;

                // Clear the screen and display the updated grid
                clearScreen();
                displayGrid(grid, droneRow, droneCol, x, y);

                // Check if the current cell has a fire
                if (grid[droneRow][droneCol] == 'X') {
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
                displayGrid(grid, droneRow, droneCol, x, y);

                // Check if the current cell has a fire
                if (grid[droneRow][droneCol] == 'X') {
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
    displayGrid(grid, -1, -1, x, y); // Display the grid without the drone
    cout << "\nAll fires discovered:" << endl;
    for (auto fire : discoveredFires) {
        cout << "(" << fire.first << ", " << fire.second << ")" << endl;
    }

    return 0;
}
