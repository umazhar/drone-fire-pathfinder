#ifndef GRID_MAP_HPP
#define GRID_MAP_HPP

#include <vector>
#include <iostream> 

enum class WindDirection {
    NORTH,
    SOUTH,
    EAST,
    WEST
};


class GridMap {
public:
    GridMap(int width, int height, WindDirection wind);

    // print to console
    void print() const;

private:
    std::vector<std::vector<int>> m_grid;
    int m_width;
    int m_height;
    WindDirection m_wind;
};

#endif // GRID_MAP_HPP
