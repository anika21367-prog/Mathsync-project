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
void undo() {
    if (undoTop == -1) { cout << "Nothing to undo." << endl; return; }
    Change change = undoStack[undoTop];
    applyContent(change.row, change.col, change.oldContent);
    undoTop--;
    redoStack[++redoTop] = change;
    cout << "Undid: " << cellName(change.row, change.col)
         << " back to " << show(change.oldContent) << endl;
    propagateUpdate(change.row * activeCols + change.col);
}

void redo() {
    if (redoTop == -1) { cout << "Nothing to redo." << endl; return; }
    Change change = redoStack[redoTop];
    applyContent(change.row, change.col, change.newContent);
    redoTop--;
    undoStack[++undoTop] = change;
    cout << "Redid: " << cellName(change.row, change.col)
         << " set to " << show(change.newContent) << endl;
    propagateUpdate(change.row * activeCols + change.col);
}

void showHistory() {
    if (undoTop == -1) cout << "History is empty." << endl;
    else {
        cout << "History (newest first):" << endl;
        int number = 1;
        for (int i = undoTop; i >= 0; i--) {
            Change change = undoStack[i];
            cout << number++ << ". " << cellName(change.row, change.col)
                 << ": " << show(change.oldContent)
                 << " -> " << show(change.newContent) << endl;
        }
    }
    cout << redoTop + 1 << " change(s) available to REDO." << endl;
}

bool saveToFile(const string &filename) {
    ofstream out(filename.c_str());
    if (!out) { cout << "Could not open file for writing: " << filename << endl; return false; }
    out << "# MathSync save file\nSIZE " << activeRows << " " << activeCols << "\n";
    int count = 0;
    for (int row = 0; row < activeRows; row++) {
        for (int col = 0; col < activeCols; col++) {
            if (!grid[row][col].content.empty()) {
                out << cellName(row, col) << " " << grid[row][col].content << "\n";
                count++;
            }
        }
    }
