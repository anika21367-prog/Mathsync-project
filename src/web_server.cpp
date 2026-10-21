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
