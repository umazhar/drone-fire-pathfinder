#include "../include/GridMap.hpp"

GridMap::GridMap(int width, int height, WindDirection wind)
    : m_width(width), m_height(height), m_wind(wind)
{
    //Implement
    m_grid.resize(m_height, std::vector<int>(m_width, 0));

    //Temp to avoid unused var warning, change later
    (void)m_wind;  


}
void GridMap::print() const {
    //Implement
}
