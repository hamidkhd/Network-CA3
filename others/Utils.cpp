#include <iostream>
#include <sys/stat.h>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <vector>

using namespace std;

int readNumber(char* frame, int start, int len) {
    string numstr = "";
    int i = start;
    while (i - start < len)
        numstr += frame[i++];

    return stoi(numstr);
}

int getFileSizeFromFrame(char* frame) {
    int endIndex = readNumber(frame, 6, 4) + 10;
    int startIndex = 10;
    while (frame[startIndex++] != '#');
    return readNumber(frame, startIndex, endIndex - startIndex);

}

string getFileNameFromFrame(char* frame) {
    int i = 10;
    string name = "";
    while (frame[i] != '#')
        name += frame[i++];
    
    return name;
}

void writeNumber(char* frame, uint num, uint start, uint len) {
    string numstr = to_string(num);
    uint i = start;
    while (i - start < len - numstr.length())
        frame[i++] = '0';
    for (char c: numstr)
        frame[i++] = c;
}

// returns size of written data
int writeFileInfo(char* frame, string name, int size) {
    int i = 10;
    for (char c: name)
        frame[i++] = c;

    frame[i++] = '#';

    for (char c: to_string(size))
        frame[i++] = c;

    return i - 10;
}

void writeMessage(char* frame, string message) {
    int i = 13;
    for (char c: message)
        frame[i++] = c;
}

string readMessage(char* frame) {
    int size = readNumber(frame, 9, 4);
    string msg = "";
    for (int i = 0; i < size; i++)
        msg += frame[13+i];
    return msg;
}

vector<string> tokenizeByChar(string str, char d){
    vector<string> tokenizedStr;
    string tmp = "";
    for (uint i = 0; i < str.length(); i++) {
        if (str[i] == d) {
            tokenizedStr.push_back(tmp);
            tmp = "";
            continue;
        }
        tmp += str[i];
    }

    return tokenizedStr;
}