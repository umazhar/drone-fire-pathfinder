#ifndef FIRE_SPREAD_SIMULATOR_HPP
#define FIRE_SPREAD_SIMULATOR_HPP

#include "GridMap.hpp"  // We need access to the GridMap and WindDirection

class FireSpreadSimulator {
public:
    FireSpreadSimulator(WindDirection wind);

    // Set the wind direction (optional if you want dynamic wind changes).
    void setWindDirection(WindDirection wind);

    // Spread fire on the map for one timestep.
    void spreadFires(GridMap& map);

private:
    WindDirection m_wind;
};

#endif // FIRE_SPREAD_SIMULATOR_HPP
