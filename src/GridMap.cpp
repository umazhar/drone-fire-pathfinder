#include "GridMap.h"
#include <cstdlib>  // for rand()
#include <ctime>
#include <algorithm>
#include <vector>

GridMap::GridMap(int r, int c)
    : rows(r), cols(c)
{
    grid.resize(rows, std::vector<char>(cols, ' '));
}

void GridMap::populateRandomFires(int fireChancePercent)
{
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            if (rand() % 100 < fireChancePercent) {
                grid[i][j] = 'X';
            }
        }
    }
}

int GridMap::getRows() const {
    return rows;
}

int GridMap::getCols() const {
    return cols;
}

char GridMap::getCell(int row, int col) const {
    return grid[row][col];
}

void GridMap::setCell(int row, int col, char value) {
    grid[row][col] = value;
}

/**
 * spreadFires:
 * For each cell that is on fire ('X'), we attempt to ignite its neighbors.
 *  - We make a copy of the grid first, so newly ignited fires do not
 *    themselves spread within the same iteration.
 *  - 'spreadChance' is a float from 0.0 to 1.0 that controls how likely
 *    each neighbor will catch fire.
 *  - We check all 8 neighboring cells in this example. You can limit to 4 if desired.
 */
void GridMap::spreadFires(double spreadChance)
{
    // Create a copy of our current grid
    std::vector<std::vector<char>> newGrid = grid;

    // Directions (8-neighbors)
    int directions[8][2] = {
        {-1, -1}, {-1, 0}, {-1, 1},
        { 0, -1},           { 0, 1},
        { 1, -1}, { 1, 0}, { 1, 1}
    };

    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            if (grid[r][c] == 'X') {
                // Try to ignite neighbors
                for (auto &d : directions) {
                    int nr = r + d[0];
                    int nc = c + d[1];
                    // Check bounds
                    if (nr >= 0 && nr < rows && nc >= 0 && nc < cols) {
                        // If neighbor is not already on fire, it might catch fire
                        if (grid[nr][nc] != 'X') {
                            double roll = (double)rand() / RAND_MAX; // random [0..1)
                            if (roll < spreadChance) {
                                newGrid[nr][nc] = 'X'; // ignite
                            }
                        }
                    }
                }
            }
        }
    }

    // Copy newGrid back into grid
    grid = newGrid;
}
