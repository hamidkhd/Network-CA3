#include "Interface.hpp"

#include <iostream>
#include <iterator>
#include <sstream>
#include <unistd.h>
#include <sys/stat.h>

using namespace std;

constexpr char ADD_ROUTER[] = "add_router"; // add_router <id> <portCount>
constexpr char ADD_SYSTEM[] = "add_system"; // add_system <id>
constexpr char CONNECT[] = "connect"; // connect <system_id> <router_id> <port_number>
constexpr char SEND[] = "send"; // send <src system> <dst system> <file name>
constexpr char RECEIVE[] = "recv"; // recv <src system> <dst system> <file name>
constexpr char CONNECT_ROUTER[] = "connect_router"; // connect_router <s1_id> <s1_port> <s2_id> <s2_port>
constexpr char RUN_STP[] = "run_stp";
constexpr char ADD_GROUP[] = "add_group"; // add_group <group_id>
constexpr char LIST_GROUPS[] = "list_groups"; // list_groups
constexpr char JOIN_GROUP[] = "join_group"; // join_group <system_id> <group_id>
constexpr char MY_GROUPS[] = "my_groups"; // my_groups <system_id>
constexpr char SEND_MSG_GROUP[] = "send_msg_group"; // send_msg_group <system_source_id> <group_id> <message ...>
constexpr char LEAVE_GROUP[] = "leave_group"; // leave_group <system_id> <group_id>
constexpr char SHOW_MSGS[] = "show_messages"; // show_messages <system_id>


void Interface::run() {
    mkdir("fifos", 0777);

    string input;
    while (getline(cin, input)) {
        const string commandType = tokenizeInput(input)[0];

        if (commandType == ADD_ROUTER) {
            addRouter(input);
        }
        else if (commandType == ADD_SYSTEM) {
            addSystem(input);
        }
        else if (commandType == CONNECT) {
            connect(input);
        }
        else if (commandType == SEND) {
            sendFile(input);
        }
        else if (commandType == RECEIVE) {
            recvFile(input);
        }
        else if (commandType == CONNECT_ROUTER) {
            connectRouter(input);
        }
        else if (commandType == RUN_STP) {
            runStp();
        }
        else if (commandType == ADD_GROUP) {
            addGroup(input);
        }
        else if (commandType == LIST_GROUPS) {
            listGroups();
        }
        else if (commandType == JOIN_GROUP) {
            joinGroup(input);
        }
        else if (commandType == MY_GROUPS) {
            listSystemGroups(input);
        }
        else if (commandType == SEND_MSG_GROUP) {
            sendMessageToGroup(input);
        }
        else if (commandType == LEAVE_GROUP) {
            leaveGroup(input);
        }
        else if (commandType == SHOW_MSGS) {
            showSystemMessages(input);
        }

        usleep(20000);
    }
}



vector<string> Interface::tokenizeInput(string input) {
    stringstream inputStringStream(input);
    return vector<string>(istream_iterator<string>(inputStringStream),
                          istream_iterator<string>());
}



void Interface::addRouter(string input) {
    vector<string> tokenizedInput = tokenizeInput(input);
    int id = stoi(tokenizedInput[1]);
    int portsCount = stoi(tokenizedInput[2]);

    int p[2];
    pipe(p);
    if (fork() == 0) {
        close(p[1]);
        dup2(p[0], STDIN_FILENO);
        close(p[0]);
        execl("./router.out", "router", to_string(id).c_str(), to_string(portsCount).c_str(), NULL);
    }
    close(p[0]);
    routers[id] = p[1];
}

void Interface::addSystem(string input) {
    vector<string> tokenizedInput = tokenizeInput(input);
    int id = stoi(tokenizedInput[1]);

    int p[2];
    pipe(p);
    if (fork() == 0) {
        close(p[1]);
        dup2(p[0], STDIN_FILENO);
        close(p[0]);
        execl("./system.out", "system", to_string(id).c_str(), NULL);
    }
    close(p[0]);
    systems[id] = p[1];
}

void Interface::connect(string input) {
    vector<string> tokenizedInput = tokenizeInput(input);
    int systemId = stoi(tokenizedInput[1]);
    int routerId = stoi(tokenizedInput[2]);
    int portNumber = stoi(tokenizedInput[3]);

    string systemReadPipe = "s" + to_string(routerId) + "-" + to_string(portNumber) + "-out";
    string systemWritePipe = "s" + to_string(routerId) + "-" + to_string(portNumber) + "-in";
    string systemCommand = "connect#" + systemWritePipe + "#" + systemReadPipe + "#";
    
    write(systems[systemId], systemCommand.c_str(), systemCommand.size());
}

void Interface::connectRouter(string input) {
    vector<string> tokenizedInput = tokenizeInput(input);
    int s1 = stoi(tokenizedInput[1]);
    int s1Port = stoi(tokenizedInput[2]);
    int s2 = stoi(tokenizedInput[3]);
    int s2Port = stoi(tokenizedInput[4]);

    string s1Pipe = "s" + to_string(s1) + "-" + to_string(s1Port) + "-in";
    string s2Pipe = "s" + to_string(s2) + "-" + to_string(s2Port) + "-in";

    string s1Command = "connects#" + s2Pipe + "#" + to_string(s1Port) + "#";
    string s2Command = "connects#" + s1Pipe + "#" + to_string(s2Port) + "#";

    write(routers[s1], s1Command.c_str(), s1Command.size());
    write(routers[s2], s2Command.c_str(), s2Command.size());
}

void Interface::sendFile(string input) {
    vector<string> tokenizedInput = tokenizeInput(input);
    int srcId = stoi(tokenizedInput[1]);
    int dstId = stoi(tokenizedInput[2]);
    string fileName = tokenizedInput[3];

    string command = "send#" + to_string(dstId) + "#" + fileName + "#";

    write(systems[srcId], command.c_str(), command.size());
}

void Interface::recvFile(string input) {
    vector<string> tokenizedInput = tokenizeInput(input);
    int srcId = stoi(tokenizedInput[1]);
    int dstId = stoi(tokenizedInput[2]);
    string fileName = tokenizedInput[3];

    string command = "recv#" + to_string(dstId) + "#" + fileName + "#";

    write(systems[srcId], command.c_str(), command.size());
}

void Interface::runStp() {
    string command = "stp#";
    for (auto const& routerPair : routers) {
        write(routerPair.second, command.c_str(), command.size());
    }
}

void Interface::addGroup(string input) {
    vector<string> tokenizedInput = tokenizeInput(input);
    int gpId = stoi(tokenizedInput[1]);
    groups.push_back(gpId);
}

void Interface::listGroups() {
    cout << "Available Groups:" << endl;
    for (int i : groups)
        cout << "Group " << i << endl;
}

void Interface::joinGroup(string input) {
    vector<string> tokenizedInput = tokenizeInput(input);
    int systemId = stoi(tokenizedInput[1]);
    int gpId = stoi(tokenizedInput[2]);
    
    string command = "join#" + to_string(gpId) + "#";

    write(systems[systemId], command.c_str(), command.size());
}

void Interface::listSystemGroups(string input) {
    vector<string> tokenizedInput = tokenizeInput(input);
    int systemId = stoi(tokenizedInput[1]);
    
    string command = "listgp#";

    write(systems[systemId], command.c_str(), command.size());
}

void Interface::sendMessageToGroup(string input) {
    vector<string> tokenizedInput = tokenizeInput(input);
    int systemId = stoi(tokenizedInput[1]);
    int gpId = stoi(tokenizedInput[2]);
    string msg = "";
    for (uint i = 3; i < tokenizedInput.size(); i++) {
        msg += tokenizedInput[i];
        if (i != tokenizedInput.size() - 1)
            msg += " ";
    }
    
    string command = "msggp#" + to_string(gpId) + "#" + msg + "#";

    write(systems[systemId], command.c_str(), command.size());
}

void Interface::leaveGroup(string input) {
    vector<string> tokenizedInput = tokenizeInput(input);
    int systemId = stoi(tokenizedInput[1]);
    int gpId = stoi(tokenizedInput[2]);
    
    string command = "leavegp#" + to_string(gpId) + "#";

    write(systems[systemId], command.c_str(), command.size());
}

void Interface::showSystemMessages(string input) {
    vector<string> tokenizedInput = tokenizeInput(input);
    int systemId = stoi(tokenizedInput[1]);
    
    string command = "showmsg#";

    write(systems[systemId], command.c_str(), command.size());
}




int main(int argc, char *argv[]) {
    Interface interface = Interface();
    interface.run(); 

    return 0;
}