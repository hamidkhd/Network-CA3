#include <map>
#include <vector>
#include <string>
#include <bits/stdc++.h>

#include "../others/Constants.hpp"

struct STPConfig {
    int root;
    int sender;
    int cost;
    int rootPort;
    bool isItBetter(int _root, int _sender, int _cost);
    void set(int _root, int _sender, int _cost, int port);
    bool isItDesignated(int _root, int _sender, int _cost);
    void makeStpFrame(char* frame, int src);
};

class Router {
public:
    Router(int _id, int _ports);
    void run();

private:
    int id;
    int portsCount;
    int maxSd;
    fd_set masterSet;
    std::map<int, int> lookupTable; // system_id -> port_number
    std::map<int, int> writePorts; // port_number -> write_pipe
    std::map<int, int> readPorts; // read_pipe -> port_number
    std::map<int, std::set<int>> groupRouteTable; // group_id -> system_id set
    void handleStdIn(std::string in);
    void handleFrame(char* frame, int readPortFD);
    void updateLookupTable(int src, int readPortFD);
    void forwardFrame(char* frame, int src, int dst);
    void handleIGMP(char* frame, int port);
    void forwardMulticast(char* frame, int port);

    struct STPConfig stp;
    std::unordered_set<int> blockedPorts;
    void handleStpMessage(char* frame, int port);
    void broadcastStp();

};