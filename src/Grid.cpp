//grid.cpp
//MathSync- Grid Storage Module
//grid.cpp is under Riddhi Gupta
#include "mathsync.h"
#include <iostream>
#include <iomanip>
#include <cctype>
using namespace std;

Cell grid[ROWS][COLS];

int cellNumber(string name) {
    if (name.length() != 2)
        return -1;

    char column = toupper(name[0]);
    char row = name[1];

    if (column < 'A' || column >= 'A' + COLS)
        return -1;

    if (row < '1' || row >= '1' + ROWS)
        return -1;

    return (row - '1') * COLS + (column - 'A');
}

bool parseCellName(string name, int &row, int &col) {
    if (name.length() != 2) return false;

    char letter = name[0];
    if (letter >= 'a' && letter <= 'z') letter = letter - 'a' + 'A';

    if (letter < 'A' || letter >= 'A' + COLS) return false;
    if (name[1] < '1' || name[1] >= '1' + ROWS) return false;

    col = letter - 'A';
    row = name[1] - '1';
    return true;
}

string cellName(int cell) {
    string name;
    name += char('A' + cell % COLS);
    name += char('1' + cell / COLS);
    return name;
}

void viewGrid() {
    cout << left << setw(6) << "";

    for (int col = 0; col < COLS; col++)
        cout << setw(16) << char('A' + col);

    cout << endl;

    for (int row = 0; row < ROWS; row++) {
        cout << setw(6) << row + 1;

        for (int col = 0; col < COLS; col++) {
            string text = grid[row][col].content;

            if (text.empty())
                text = "-";

            cout << setw(16) << text;
        }

        cout << endl;
    }
}
