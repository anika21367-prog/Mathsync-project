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
          if (command == "SET") {
            string content;
            if (!(input >> name)) {
                cout << "Usage: SET A1 10" << endl;
                continue;
            }
            input >> ws;
            getline(input, content);
            if (content.empty()) cout << "Use CLEAR to empty a cell." << endl;
            else setCell(name, content);
        } else if (command == "EVAL") {
            if (input >> name) evaluateCell(name);
            else cout << "Usage: EVAL B1" << endl;
 } else if (command == "DEPS") {
            if (input >> name) showDependents(name);
            else cout << "Usage: DEPS A1" << endl;
        } else if (command == "CLEAR") {
            if (input >> name) setCell(name, "");
            else cout << "Usage: CLEAR A1" << endl;
        } else if (command == "UNDO") undo();
        else if (command == "REDO") redo();
        else if (command == "HISTORY") showHistory();
        else if (command == "SYNC" || command == "LOAD") {
            string filename;
            if (!(input >> filename)) {
                cout << "Please give a file name." << endl;
                continue;
