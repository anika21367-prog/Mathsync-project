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

void resizeGrid(int newRows, int newCols) {
    if (newRows < 1 || newRows > MAX_ROWS || newCols < 1 || newCols > MAX_COLS) {
        cout << "Invalid size. Rows must be 1.." << MAX_ROWS
             << " and Cols must be 1.." << MAX_COLS << "." << endl;
        return;
    }
    activeRows = newRows;
    activeCols = newCols;
    freeAllLists();
    for (int r = 0; r < activeRows; r++) {
        for (int c = 0; c < activeCols; c++) {
            if (grid[r][c].isFormula) {
                bool references[MAX_TOTAL] = {};
                int cellIdx = r * activeCols + c;
                if (findReferences(grid[r][c].content, references)) {
                    for (int src = 0; src < activeTotalCells(); src++) {
                        if (references[src]) addDependent(src, cellIdx);
                    }
                }
            }
        }
    }
    for (int r = 0; r < activeRows; r++) {
        for (int c = 0; c < activeCols; c++) {
            if (grid[r][c].isFormula) recalculateCell(r * activeCols + c);
        }
    }
    cout << "Grid resized to " << activeRows << "x" << activeCols
         << " (A1.." << static_cast<char>('A' + activeCols - 1) << activeRows << ")." << endl;
}

void viewGrid() {
    cout << left << setw(6) << "";
    for (int col = 0; col < activeCols; col++)
        cout << setw(14) << static_cast<char>('A' + col);
    cout << endl;
    for (int row = 0; row < activeRows; row++) {
        cout << setw(6) << (row + 1);
        for (int col = 0; col < activeCols; col++) {
            string text = grid[row][col].content;
            if (text.empty()) text = "-";
            if (text.length() > 12) text = text.substr(0, 11) + "~";
            cout << setw(14) << text;
        }
        cout << endl;
    }
}



