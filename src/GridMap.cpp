#include "GridMap.h"
#include <cstdlib>  // For rand()

GridMap::GridMap(int r, int c)
    : rows(r), cols(c)
{
    grid.resize(rows, std::vector<char>(cols, ' '));
}

void GridMap::populateRandomFires(int fireChancePercent)
{
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            // Randomly place a fire with the given percentage chance
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
