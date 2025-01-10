#ifndef COMM_MODULE_HPP
#define COMM_MODULE_HPP

#include <vector>
#include "PerceptionMap.hpp"

class CommModule {
public:
    // Serialize the perception map into a raw binary buffer.
    static std::vector<unsigned char> serializePerception(const PerceptionMap& pmap);

    // Send data over UDP/TCP (sockets).
    static void sendToBaseStation(const std::vector<unsigned char>& data, 
                                  const std::string& ip, 
                                  int port);

    // (Optional) a receive function if you are also implementing the ground station side.
    static std::vector<unsigned char> receiveData(int listeningPort);
};

#endif // COMM_MODULE_HPP
