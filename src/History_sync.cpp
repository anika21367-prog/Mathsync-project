#include "mathsync.h"
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <iomanip>
using namespace std;

string cellName(int row, int col) {
    return string(1, static_cast<char>('A' + col)) + to_string(row + 1);
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

void applyContent(int row, int col, string content) {
    grid[row][col].content = content;
    grid[row][col].isFormula = !content.empty() && content[0] == '=';
}
bool canRecordChange() { return undoTop < MAX_HISTORY - 1; }

void recordChange(int cell, string oldContent, string newContent) {
    Change change;
    change.row = cell / activeCols;
    change.col = cell % activeCols;
    change.oldContent = oldContent;
    change.newContent = newContent;
    undoStack[++undoTop] = change;
    redoTop = -1;
}
