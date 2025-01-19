#ifndef GRIDMAP_H
#define GRIDMAP_H

#include <vector>

class GridMap {
public:
    // Constructor
    GridMap(int rows, int cols);

    void populateRandomFires(int fireChancePercent);

    int getRows() const;
    int getCols() const;

    char getCell(int row, int col) const;
    void setCell(int row, int col, char value);

    // fire spread
    void spreadFires(double spreadChance);

private:
    std::vector<std::vector<char>> grid;
    int rows;
    int cols;
};

#endif // GRIDMAP_H
