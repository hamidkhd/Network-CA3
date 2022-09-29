#include "Router.hpp"
#include "../others/Utils.cpp"


Router::Router(int _id, int _ports): id(_id), portsCount(_ports) {
    FD_ZERO(&masterSet);
	FD_SET(0, &masterSet);
    for (int i = 0; i < portsCount; i++) {
        string inName = "fifos/s" + to_string(id) + "-" + to_string(i+1) + "-" + "in";
        string outName = "fifos/s" + to_string(id) + "-" + to_string(i+1) + "-" + "out";
        mkfifo(inName.c_str(), 0666);
        mkfifo(outName.c_str(), 0666);

        int in = open(inName.c_str(), O_RDWR);
        int out = open(outName.c_str(), O_RDWR);

        if (in < 0 || out < 0)
            cout << "Router " << id << ": Error in openning pipe" << endl;

        readPorts[in] = i+1;
        writePorts[i+1] = out;
        FD_SET(in, &masterSet);
        maxSd = out;
    }

    stp.root = id;
    stp.sender = id;
    stp.rootPort = 0;
    stp.cost = -1;
}

void Router::run() {
    char in[100];
    char data[FRAME_SIZE];

    fd_set working_set;

	while(true) {
		working_set = masterSet;
        select(1020, &working_set, NULL, NULL, NULL); 

        for (int i = 0; i <= 1020; i++) {
            if (FD_ISSET(i, &working_set)) {
                if (i == 0) { // stdin
                    read(0, in, 100);
					handleStdIn(in);
                }
                else { // ports
					read(i, data, FRAME_SIZE);
                    handleFrame(data, i);
                }
            }
        }
	}
}

void Router::handleStdIn(string in) {
    vector<string> tokenizedInput = tokenizeByChar(in, '#');
    if (tokenizedInput[0] == "connects") {
        close(writePorts[stoi(tokenizedInput[2])]);
        int fifo = open(("fifos/" + tokenizedInput[1]).c_str(), O_RDWR);

        if (fifo < 0)
            cout << "System " << id << ": Error in openning pipe" << endl;

        writePorts[stoi(tokenizedInput[2])] = fifo;
        maxSd = fifo;
    }
    else if (tokenizedInput[0] == "stp") {
        cout << "ROUTER " << id << " STARTING STP" << endl;
        broadcastStp();
    }
}

void Router::handleFrame(char* frame, int readPortFD) {
    int port = readPorts[readPortFD];

    if (blockedPorts.count(port))
        return;

    int dst = readNumber(frame, 0, 3);
    int src = readNumber(frame, 3, 3);
    

    if (dst == STP) {
        handleStpMessage(frame, port);
    }
    else if (dst == IGMP) {
        updateLookupTable(src, readPortFD);
        handleIGMP(frame, port);
    }
    else if (dst == MULTICAST_FILE || dst == MULTICAST_MSG) {
        updateLookupTable(src, port);
        forwardMulticast(frame, port);
    } 
    else { // Data to forward
        updateLookupTable(src, readPortFD);
        forwardFrame(frame, src, dst);
    }

}

void Router::updateLookupTable(int srcId, int readPortFD) {
    if (lookupTable.count(srcId))
        return;

    lookupTable[srcId] = readPorts[readPortFD];
}

void Router::forwardFrame(char* frame, int src, int dst) {
    if (lookupTable.count(dst)) {
        write(writePorts[lookupTable[dst]], frame, FRAME_SIZE);
    } 
    else { // forward to all ports
        for (int port = 1; port <= portsCount; port++) {
            if (lookupTable[src] == port || blockedPorts.count(port)) // dont resend to src and blocked ports
                continue;

            write(writePorts[port], frame, FRAME_SIZE);
        }
    }
}

void Router::handleIGMP(char* frame, int port) {
    int src = readNumber(frame, 3, 3);
    int groupId = readNumber(frame, 6, 3);
    int type = readNumber(frame, 9, 1);

    if (type == IGMP_JOIN) {
        groupRouteTable[groupId].insert(src);
        cout << "Router " << id << ": System " << src << " JOINed Group " << groupId << endl; 
    }
    else if (type == IGMP_LEAVE) {
        groupRouteTable[groupId].erase(src);
        cout << "Router " << id << ": System " << src << " LEFT Group " << groupId << endl;
    }

    for (int p = 1; p <= portsCount; p++) { // broadcast IGMP message
        if (blockedPorts.count(p) || p == port)
            continue;
        write(writePorts[p], frame, FRAME_SIZE);
    }
}

void Router::forwardMulticast(char* frame, int port) {
    int src = readNumber(frame, 3, 3);
    int groupId = readNumber(frame, 6, 3);

    set<int> sent_ports;
    for (int dst : groupRouteTable[groupId]) {
        if (dst == src)
            continue;
        
        int next_port = lookupTable[dst];
        if (sent_ports.count(next_port) || next_port == port)
            continue;
        
        sent_ports.insert(next_port);
        write(writePorts[next_port], frame, FRAME_SIZE);
    }
}

void Router::handleStpMessage(char* frame, int port) {
    int src = readNumber(frame, 3, 3);
    int root = readNumber(frame, 6, 3);
    int cost = readNumber(frame, 9, 3);

    if (stp.isItBetter(root, src, cost)) {
        stp.set(root, src, cost, port);
    }
    else if (stp.isItDesignated(root, src, cost) && port != stp.rootPort) {
        blockedPorts.insert(port);
        cout << "Router " << id << ": Blocked port " << port << endl;
    }

    broadcastStp();
}

void Router::broadcastStp() {
    char stpMessage[FRAME_SIZE];
    stp.makeStpFrame(stpMessage, id);
    for (int port = 1; port <= portsCount; port++) {
        if (blockedPorts.count(port) || port == stp.rootPort)
            continue;
        write(writePorts[port], stpMessage, FRAME_SIZE);
    }
}

bool STPConfig::isItBetter(int _root, int _sender, int _cost) {
    if (root == _root) {
        if (cost == _cost)
            return _sender <= sender;
        return _cost < cost;
    }

    return _root < root;
}

void STPConfig::set(int _root, int _sender, int _cost, int port) {
    root = _root;
    _sender = sender;
    cost = _cost;
    rootPort = port;
}

bool STPConfig::isItDesignated(int _root, int _sender, int _cost) {
    if (_root > root)
        return false;
    
    if (cost+1 == _cost) {
        return _sender < sender;
    }

    return _cost < cost+1;
}

void STPConfig::makeStpFrame(char* frame, int src) {
    writeNumber(frame, STP, 0, 3);
    writeNumber(frame, src, 3, 3);
    writeNumber(frame, root, 6, 3);
    writeNumber(frame, cost+1, 9, 3);
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        cout << "missing args" << endl;
        exit(EXIT_FAILURE);
    }

    Router sw = Router(atoi(argv[1]), atoi(argv[2]));
    sw.run();

    return 0;
}