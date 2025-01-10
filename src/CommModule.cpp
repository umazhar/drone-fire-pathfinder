#include "../include/CommModule.hpp"
#include <iostream> // for debugging
// For real networking, include <sys/socket.h>, <arpa/inet.h>, etc. on Linux
// or <Winsock2.h> on Windows

std::vector<unsigned char> CommModule::serializePerception(const PerceptionMap& pmap) {
    // For demonstration, we’ll just flatten the bool 2D array into bytes (0 or 1).
    // Also store width/height in the first 2 bytes, for example.
    // Pseudocode:
    std::vector<unsigned char> buffer;
    auto& knownFires = pmap.getKnownFires();
    int height = knownFires.size();
    int width = height > 0 ? knownFires[0].size() : 0;

    // Store width, height as 2 bytes:
    buffer.push_back(static_cast<unsigned char>(width));
    buffer.push_back(static_cast<unsigned char>(height));

    // Then store the map data:
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            buffer.push_back(knownFires[y][x] ? 1 : 0);
        }
    }

    return buffer;
}

void CommModule::sendToBaseStation(const std::vector<unsigned char>& data, 
                                   const std::string& ip, 
                                   int port) 
{
    // Basic pseudocode for a UDP send:
    /*
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = inet_addr(ip.c_str());

    sendto(sock, (const char*)data.data(), data.size(), 0, 
           (sockaddr*)&serverAddr, sizeof(serverAddr));
    close(sock);
    */
   std::cout << "Sending " << data.size() << " bytes to " << ip << ":" << port << std::endl;
}

std::vector<unsigned char> CommModule::receiveData(int listeningPort) {
    // Basic pseudocode for receiving data over UDP:
    /*
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(listeningPort);
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    bind(sock, (sockaddr*)&serverAddr, sizeof(serverAddr));

    unsigned char buffer[1024];
    sockaddr_in clientAddr;
    socklen_t addrLen = sizeof(clientAddr);
    int recvLen = recvfrom(sock, (char*)buffer, 1024, 0, 
                           (sockaddr*)&clientAddr, &addrLen);

    close(sock);

    return std::vector<unsigned char>(buffer, buffer + recvLen);
    */
   std::cout << "receiveData not implemented yet!\n";
   return {};
}
