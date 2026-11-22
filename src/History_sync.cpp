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
 out.close();
    if (!out) { cout << "Error while writing file: " << filename << endl; return false; }
    cout << "Saved " << count << " cell(s) to " << filename << endl;
    return true;
}

bool loadFromFile(const string &filename) {
    ifstream in(filename.c_str());
    if (!in) { cout << "Could not open file for reading: " << filename << endl; return false; }
    Cell loaded[MAX_ROWS][MAX_COLS];
    int nextRows = activeRows, nextCols = activeCols;
    string line;
    int skipped = 0;
    while (getline(in, line)) {
        if (!line.empty() && line[line.length() - 1] == '\r') line.erase(line.length() - 1);
        istringstream input(line);
        string first;
        if (!(input >> first)) continue;
        if (first[0] == '#') continue;
        if (first == "SIZE") {
            int r, c;
            if ((input >> r >> c) && r >= 1 && r <= MAX_ROWS && c >= 1 && c <= MAX_COLS) {
                nextRows = r; nextCols = c;
            } else skipped++;
            continue;
        }
 string content;
        input >> ws;
        getline(input, content);
        int row, col;
        if (!parseCellName(first, row, col) || content.empty()) { skipped++; continue; }
        loaded[row][col].content = content;
        loaded[row][col].isFormula = content[0] == '=';
    }
    if (in.bad() || (in.fail() && !in.eof())) {
        cout << "Read failed. Current sheet was not changed." << endl;
        return false;
    }
    activeRows = nextRows; activeCols = nextCols;
    int count = 0;
    for (int row = 0; row < MAX_ROWS; row++) {
        for (int col = 0; col < MAX_COLS; col++) {
            grid[row][col] = loaded[row][col];
            if (row < activeRows && col < activeCols && !grid[row][col].content.empty()) count++;
        }
    }
    undoTop = -1; redoTop = -1;
    freeAllLists();
    for (int cell = 0; cell < activeTotalCells(); cell++) {
        int r = cell / activeCols, c = cell % activeCols;
        if (grid[r][c].isFormula) {
            bool references[MAX_TOTAL] = {};
            if (findReferences(grid[r][c].content, references)) {
                for (int source = 0; source < activeTotalCells(); source++) {
                    if (references[source]) addDependent(source, cell);
                }
            }
        }
    }
    for (int cell = 0; cell < activeTotalCells(); cell++) {
        int r = cell / activeCols, c = cell % activeCols;
        if (grid[r][c].isFormula) recalculateCell(cell);
    }
    cout << "Loaded " << count << " cell(s) (" << activeRows << "x" << activeCols
         << ") from " << filename << "; skipped " << skipped << " bad line(s)." << endl;
    return true;
}
