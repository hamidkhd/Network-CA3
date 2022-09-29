#include <map>
#include <vector>
#include <string>

class Interface {
public:
    void run();

private:
    std::map<int, int> routers; // id -> pipe
    std::map<int, int> systems; // id -> pipe
    std::vector<int> groups; // id
    std::vector<std::string> tokenizeInput(std::string input);
    void addRouter(std::string input);
    void addSystem(std::string input);
    void connect(std::string input);
    void sendFile(std::string input);
    void recvFile(std::string input);
    void connectRouter(std::string input);
    void runStp();
    void addGroup(std::string input);
    void listGroups();
    void joinGroup(std::string input);
    void listSystemGroups(std::string input);
    void sendMessageToGroup(std::string input);
    void leaveGroup(std::string input);
    void showSystemMessages(std::string messages);
};