/*history module is under Akarshak Singh. This needs to be built using linked list*/
include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <iomanip>
using namespace std;

const int ROWS = 5;
const int COLS = 5;
const int MAX_HISTORY = 100;

struct Cell {
    string content;
    bool isFormula = false;
};
Cell grid[ROWS][COLS];

bool parseCellName(string name, int &row, int &col) {
    if (name.length() != 2) return false;
    char letter = name[0];
    if (letter >= 'a' && letter <= 'z') letter = letter - 'a' + 'A';
    if (letter < 'A' || letter >= 'A' + COLS) return false;
    if (name[1] < '1' || name[1] >= '1' + ROWS) return false;
    
    row = name[1] - '1';
    col = letter - 'A';
    return true;
}

string cellName(int row, int col) {
    string name;
    name += char('A' + col);
    name += char('1' + row);
    return name;
}

string show(string content) {
    if (content.empty()) return "(empty)";
    return content;
}

struct Change {
    int row;
    int col;
    string oldContent;
    string newContent;
};

Change undoStack[MAX_HISTORY];
Change redoStack[MAX_HISTORY];
int undoTop = -1;
int redoTop = -1;
