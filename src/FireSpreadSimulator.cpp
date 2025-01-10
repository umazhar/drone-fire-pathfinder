#include "../include/FireSpreadSimulator.hpp"

FireSpreadSimulator::FireSpreadSimulator(WindDirection wind)
    : m_wind(wind)
{
}

void FireSpreadSimulator::setWindDirection(WindDirection wind) {
    m_wind = wind;
}

void FireSpreadSimulator::spreadFires(GridMap& map) {
    // Pseudocode example:
    // 1. Identify all currently burning cells.
    // 2. Based on wind direction, pick which adjacent cells might ignite.
    // 3. Mark them for ignition.
    // 4. Update the map in a second pass, so you don’t modify while iterating.

    // For now, just print a debug message:
    // (This is where your actual fire-spread logic will go.)
    std::cout << "Spreading fires with wind direction...\n";
}
