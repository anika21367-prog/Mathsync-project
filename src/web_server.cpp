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
string indexHTML = R"HTML(<!DOCTYPE html>
<html>
<head>
<meta charset="utf-8">
<title>MathSync</title>
<style>
body { font-family: Arial, sans-serif; background:#f4f6f8; margin:24px; }
h1 { color:#203a56; }
.card { background:white; padding:18px; border-radius:10px; max-width:900px;
        box-shadow:0 2px 8px rgba(0,0,0,.12); margin-bottom:18px; }
table { border-collapse:collapse; width:100%; }
th, td { border:1px solid #cbd5e1; padding:8px; text-align:left; }
th { background:#203a56; color:white; }
td { background:white; min-width:110px; }
.rowhead { background:#e2e8f0; font-weight:bold; }
input { padding:8px; width:70%; border:1px solid #94a3b8; border-radius:6px; }
button { padding:8px 12px; margin:3px; border:none; border-radius:6px;
         background:#2563eb; color:white; cursor:pointer; }
button.secondary { background:#64748b; }
#log { background:#0f172a; color:#d1e7ff; padding:12px; height:180px;
       overflow:auto; border-radius:8px; font-family:monospace; white-space:pre-wrap; }
</style>
</head>
<body>
<h1>MathSync - Mini Spreadsheet</h1>

<div class="card">
  <input id="command" placeholder="Example: SET B1 =A1+5">
  <button onclick="send()">Run</button>
  <button class="secondary" onclick="quick('UNDO')">Undo</button>
  <button class="secondary" onclick="quick('REDO')">Redo</button>
  <button class="secondary" onclick="quick('HISTORY')">History</button>
  <button class="secondary" onclick="quick('VIEW')">Refresh</button>
  <button class="secondary" onclick="quick('SYNC sheet.txt')">Sync</button>
  <button class="secondary" onclick="quick('LOAD sheet.txt')">Load</button>
</div>
<div class="card">
  <table id="sheet"></table>
</div>

<div class="card">
  <h3>Output</h3>
  <div id="log">Welcome to MathSync.</div>
</div>
<script>
async function sendCommand(command) {
  const response = await fetch('/api/command', {
    method: 'POST',
    headers: {'Content-Type': 'application/json'},
    body: JSON.stringify({command: command})
  });
  const data = await response.json();
  document.getElementById('log').textContent = data.output;
  showGrid(data.grid);
}
