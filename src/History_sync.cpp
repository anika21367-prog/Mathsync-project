/*history module is under Akarshak Singh. This needs to be built using linked list*/
#include <iostream>
#include <string>
#include <vector>
#include <stack>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <cctype>

using namespace std;
const int ROWS = 5;
const int COLS = 5;

struct Cell {
    string content = "";
    bool isFormula = false;
};

Cell grid[ROWS][COLS];

bool parseCellName(const string &cellName, int &row, int &col) {
    if (cellName.size() < 2) return false;

    char colChar = toupper(cellName[0]);
    if (colChar < 'A' || colChar > ('A' + COLS - 1)) return false;
    col = colChar - 'A';

    string rowPart = cellName.substr(1);
    for (char c : rowPart) {
        if (!isdigit(c)) return false;
    }

    int rowNum = stoi(rowPart);
    if (rowNum < 1 || rowNum > ROWS) return false;
    row = rowNum - 1;

    return true;
}

struct Change {
    string cellName;
    string oldContent;
    string newContent;
};

stack<Change> undoStack;
stack<Change> redoStack;
