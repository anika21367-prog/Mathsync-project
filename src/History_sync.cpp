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

void applyContent(int row, int col, string content) {
    grid[row][col].content = content;
    grid[row][col].isFormula = !content.empty() && content[0] == '=';
}

void setCell(string name, string content) {
    int row, col;
    if (!parseCellName(name, row, col)) {
        cout << "Invalid cell. Use A1 to E5." << endl;
        return;
    }
    string oldContent = grid[row][col].content;
    if (oldContent == content) {
        cout << cellName(row, col) << " already contains " << show(content) << endl;
        return;
    }
    if (undoTop == MAX_HISTORY - 1) {
        cout << "History is full. Edit not applied (maximum 100 changes)." << endl;
        return;
    }
    Change change;
    change.row = row;
    change.col = col;
    change.oldContent = oldContent;
    change.newContent = content;
    applyContent(row, col, content);
    undoStack[++undoTop] = change;
    redoTop = -1; 
    cout << cellName(row, col) << " updated to " << show(content) << endl;
}
Change undoStack[MAX_HISTORY];
Change redoStack[MAX_HISTORY];
int undoTop = -1;
int redoTop = -1;
void undo() {
    if (undoTop == -1) {
        cout << "Nothing to undo." << endl;
        return;
    }
    Change change = undoStack[undoTop];
    applyContent(change.row, change.col, change.oldContent);
    undoTop--;
    redoStack[++redoTop] = change;
    cout << "Undid: " << cellName(change.row, change.col)
         << " back to " << show(change.oldContent) << endl;
}

void redo() {
    if (redoTop == -1) {
        cout << "Nothing to redo." << endl;
        return;
    }
    Change change = redoStack[redoTop];
    applyContent(change.row, change.col, change.newContent);
    redoTop--;
    undoStack[++undoTop] = change;
    cout << "Redid: " << cellName(change.row, change.col)
         << " set to " << show(change.newContent) << endl;
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
    if (!out) {
        cout << "Could not open file for writing: " << filename << endl;
        return false;
    }
  cout << "# MathSync save file\n";
    int count = 0;
    for (int row = 0; row < ROWS; row++) {
        for (int col = 0; col < COLS; col++) {
            if (!grid[row][col].content.empty()) {
                out << cellName(row, col) << " " << grid[row][col].content << "\n";
                count++;
            }
        }
    }
cout.close();
    if (!out) {
        cout << "Error while writing file: " << filename << endl;
        return false;
    }
    cout << "Saved " << count << " cell(s) to " << filename << endl;
    return true;
}
bool loadFromFile(const string &filename) {
    ifstream in(filename.c_str());
    if (!in) {
        cout << "Could not open file for reading: " << filename << endl;
        return false;
    }
