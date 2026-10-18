// main.cpp - MathSync command parser
#include "mathsync.h"
#include <iostream>
#include <sstream>
using namespace std;

int main() {
    cout << "MathSync - DSA Project" << endl;
    cout << "Commands: SET A1 10, SET B1 =A1+5, EVAL B1, DEPS A1," << endl;
    cout << "CLEAR A1, UNDO, REDO, HISTORY, SYNC sheet.txt," << endl;
    cout << "LOAD sheet.txt, VIEW, EXIT" << endl;

    string line;
    while (true) {
        cout << "\nEnter command: ";
        if (!getline(cin, line)) break;
        istringstream input(line);
        string command, name;
        if (!(input >> command)) continue;

