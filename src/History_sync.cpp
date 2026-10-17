/*history module is under Akarshak Singh. This needs to be built using linked list*/
#include <iostream>
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
 Cell loaded[ROWS][COLS];
    string line;
    int skipped = 0;
    while (getline(in, line)) {
        if (!line.empty() && line[line.length() - 1] == '\r') line.erase(line.length() - 1);
        istringstream input(line);
        string name, content;
        if (!(input >> name)) continue;
        if (name[0] == '#') continue;
        input >> ws;
        getline(input, content);
        int row, col;
        if (!parseCellName(name, row, col) || content.empty()) {
            skipped++;
            continue;
        }
        loaded[row][col].content = content;
        loaded[row][col].isFormula = content[0] == '=';

    if (in.bad() || (in.fail() && !in.eof())) {
        cout << "Read failed. Current sheet was not changed." << endl;
        return false;
    }
 int count = 0;
    for (int row = 0; row < ROWS; row++) {
        for (int col = 0; col < COLS; col++) {
            grid[row][col] = loaded[row][col];
            if (!grid[row][col].content.empty()) count++;
        }
    }
    undoTop = -1;
    redoTop = -1;
    cout << "Loaded " << count << " cell(s) from " << filename
         << "; skipped " << skipped << " bad line(s)." << endl;
    return true;
}

void viewGrid() {
    cout << left << setw(6) << "";
    for (int col = 0; col < COLS; col++) cout << setw(16) << char('A' + col);
    cout << endl;
    for (int row = 0; row < ROWS; row++) {
        cout << setw(6) << row + 1;
        for (int col = 0; col < COLS; col++) {
            string text = grid[row][col].content;
            if (text.empty()) text = "-";
            cout << setw(16) << text;
        }
        cout << endl;
    }
}
int main() {
    cout << "MathSync - History and File Saving" << endl;
    cout << "Commands: SET A1 10, CLEAR A1, UNDO, REDO, HISTORY,\n"
         << "SAVE sheet.txt, LOAD sheet.txt, VIEW, EXIT" << endl;
    cout << "Use uppercase commands. File names must not contain spaces." << endl;
    string line;
    while (true) {
        cout << "\nEnter command: ";
        if (!getline(cin, line)) break;
        istringstream input(line);
        string command, name;
        if (!(input >> command)) continue;
        if (command == "SET") {
            string content;
            if (!(input >> name)) { cout << "Usage: SET A1 10" << endl; continue; }
            input >> ws;
            getline(input, content); 
            if (content.empty()) { cout << "Use CLEAR to empty a cell." << endl; continue; }
            setCell(name, content);
        } else if (command == "CLEAR") {
            if (input >> name) setCell(name, "");
            else cout << "Usage: CLEAR A1" << endl;
        } else if (command == "UNDO") undo();
        else if (command == "REDO") redo();
        else if (command == "HISTORY") showHistory();
        else if (command == "VIEW") viewGrid();
        else if (command == "EXIT") break;
        else if (command == "SAVE" || command == "LOAD") {
            string filename;
            if (!(input >> filename)) { cout << "Please give a file name." << endl; continue; }
            if (command == "SAVE") saveToFile(filename);
            else loadFromFile(filename);
        } else cout << "Unknown command." << endl;
    }
    return 0;
}
