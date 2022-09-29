#include <string>
#include <vector>

#include "../others/Constants.hpp"

class System {
public:
    System(int _id);
    void run();

private:
    int id;
    int writePipe;
    int readPipe;
    int maxSd;
    fd_set masterSet;
    std::vector<int> groups;
    std::vector<std::string> messageBuffer;
    void handleStdIn(std::string in);
    void handleFrame(char* frame);
    void recvFile(std::string fileName, int fsize);
    void sendFile(int dst, std::string fileName);
    void sendFileReq(int dst, std::string fileName);
    bool isFileReq(char* frame);
    void joinGroup(int groupId);
    void leaveGroup(int groupId);
    void sendIGMP(int groupId, int type);
    void listGroups();
    void sendMessageToGroup(int groupId, std::string message);
    void showMessages();
    void recvMulticastMessage(char* frame);
};