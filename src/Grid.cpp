//grid.cpp
//MathSync- Grid Storage Module
//grid.cpp is under Riddhi Gupta
#include "mathsync.h"
#include <iostream>
#include <iomanip>
#include <cctype>
#include <cstdlib>
using namespace std;

int activeRows = 10;
int activeCols = 10;
Cell grid[MAX_ROWS][MAX_COLS];
int activeTotalCells() { return activeRows * activeCols; }

bool parseCellName(string name, int &row, int &col) {
    if (name.length() < 2 || name.length() > 3) return false;
    char letter = static_cast<char>(toupper(static_cast<unsigned char>(name[0])));
    if (letter < 'A' || letter >= 'A' + activeCols) return false;
    for (size_t i = 1; i < name.length(); i++) {
        if (!isdigit(static_cast<unsigned char>(name[i]))) return false;
    }
    int rowNum = atoi(name.substr(1).c_str());
    if (rowNum < 1 || rowNum > activeRows) return false;
    col = letter - 'A';
    row = rowNum - 1;
    return true;
}
int cellNumber(string name) {
    int row = -1, col = -1;
    if (!parseCellName(name, row, col)) return -1;
    return row * activeCols + col;
}

string cellName(int cell) {
    if (cell < 0 || cell >= activeTotalCells()) return "";
    int row = cell / activeCols;
    int col = cell % activeCols;
    return string(1, static_cast<char>('A' + col)) + to_string(row + 1);
}

