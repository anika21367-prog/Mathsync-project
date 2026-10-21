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

// Escape quotes, backslashes and new lines for JSON.
string jsonText(string text) {
    string result = "\"";
    for (size_t i = 0; i < text.length(); i++) {
        char ch = text[i];
        if (ch == '"' || ch == '\\') {
            result += '\\';
            result += ch;
        } else if (ch == '\n') result += "\\n";
        else if (ch == '\r') continue;
        else result += ch;
    }
    return result + "\"";
}

// Turn the grid into JSON: [["10","=A1+5",...], ...]
string gridJSON() {
    string json = "[";
    for (int row = 0; row < ROWS; row++) {
        if (row > 0) json += ",";
        json += "[";
        for (int col = 0; col < COLS; col++) {
            if (col > 0) json += ",";
            json += jsonText(grid[row][col].content);
        }
        json += "]";
    }
    return json + "]";
}
// Turn the grid into JSON: [["10","=A1+5",...], ...]
string gridJSON() {
    string json = "[";
    for (int row = 0; row < ROWS; row++) {
        if (row > 0) json += ",";
        json += "[";
        for (int col = 0; col < COLS; col++) {
            if (col > 0) json += ",";
            json += jsonText(grid[row][col].content);
        }
        json += "]";
    }
    return json + "]";
}
string httpResponse(string body, string type) {
    string header =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: " + type + "\r\n"
        "Connection: close\r\n"
        "Content-Length: " + to_string(body.length()) + "\r\n\r\n";
    return header + body;
}
// Very small JSON value reader for {"command":"..."}
// It is enough for this project because the browser sends only one field.
string readCommandJSON(string body) {
    size_t start = body.find("\"command\"");
    if (start == string::npos) return "";
    start = body.find("\"", start + 9);
    if (start == string::npos) return "";
    start++;
    string command;
    while (start < body.length() && body[start] != '"') {
        if (body[start] == '\\' && start + 1 < body.length()) {
            start++;
            if (body[start] == 'n') command += '\n';
            else command += body[start];
        } else {
            command += body[start];
        }
        start++;
    }
    return command;
}

