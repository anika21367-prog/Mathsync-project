// web_server.cpp - small browser frontend for MathSync
// Compile instead of main.cpp:
//   g++ web_server.cpp grid.cpp formula.cpp dependency.cpp history.cpp -o mathsync-web
// Then open: http://localhost:8080
#include "mathsync.h"
#include <iostream>
#include <sstream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <winsock2.h>
#include <ws2tcpip.h>
using namespace std;

// Run one MathSync command and capture its console output.
string runCommand(string command) {
    ostringstream output;
    streambuf *old = cout.rdbuf();
    cout.rdbuf(output.rdbuf());

    
    istringstream input(command);
    string action, name;
    if (!(input >> action)) {
        cout << "Empty command." << endl;
    } else if (action == "SET") {
        string content;
        if (input >> name) {
            input >> ws;
            getline(input, content);
            if (!content.empty()) setCell(name, content);
            else cout << "Use CLEAR to empty a cell." << endl;
        } else cout << "Usage: SET A1 10" << endl;
    } else if (action == "CLEAR") {
        if (input >> name) setCell(name, "");
        else cout << "Usage: CLEAR A1" << endl;
    } else if (action == "UNDO") undo();
    else if (action == "REDO") redo();
    else if (action == "HISTORY") showHistory();
    else if (action == "SYNC" || action == "LOAD") {
        string filename;
        if (input >> filename) {
            if (action == "SYNC") saveToFile(filename);
            else loadFromFile(filename);
        } else cout << "Please give a file name." << endl;
    } else if (action == "VIEW") viewGrid();
    else cout << "Unknown command." << endl;

    cout.rdbuf(old);
    return output.str();
}
