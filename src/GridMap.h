#ifndef GRIDMAP_H
#define GRIDMAP_H

#include <vector>

class GridMap {
public:
    // Constructor: Initialize the grid of size (rows x cols)
    GridMap(int rows, int cols);

    // Populate the grid with randomly placed fires (char 'X'),
    // given a probability percentage [0-100].
    void populateRandomFires(int fireChancePercent);

    // Getters for number of rows and columns
    int getRows() const;
    int getCols() const;

    // Get or set a particular cell in the grid
    char getCell(int row, int col) const;
    void setCell(int row, int col, char value);

private:
    std::vector<std::vector<char>> grid;
    int rows;
    int cols;
};

#endif // GRIDMAP_H
