#ifndef PATHFINDER_HPP
#define PATHFINDER_HPP

#include <vector>
#include <utility> // for std::pair
#include "GridMap.hpp"

class Pathfinder {
public:
    // Returns a list of (x, y) steps from (startX, startY) to (goalX, goalY)
    // If no path is found, returns an empty vector.
    static std::vector<std::pair<int,int>> findPath(
        const GridMap& map, int startX, int startY, int goalX, int goalY
    );
};

#endif // PATHFINDER_HPP
