//grid.cpp
//MathSync- Grid Storage Module
#include <iostream>
#include <string>
#include <iomanip>
#include <cctype>
using namespace std;
const int ROWS = 5;
const int COLS = 5;
const int TOTAL = ROWS * COLS;
struct cell {
    string content = "";
    bool isFormula = false;
};
cell grid[ROWS][COLS];
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
string cellName(int cell) {
    string name;
    name += char('A' + cell % COLS);
    name += char('1' + cell / COLS);
    return name;
}
void setCell(string name, string content) {
    int cell = cellNumber(name);
    if (cell == -1) {
        cout << "Invalid cell. Use A1 to E5." << endl;
        return;
    }
    int row = cell / COLS;
    int col = cell % COLS;
    grid[row][col].content = content;
    if (!content.empty() && content[0] == '=')
        grid[row][col].isFormula = true;
    else
        grid[row][col].isFormula = false;
    cout << cellName(cell) << " updated to "
         << content << endl;
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
int main() {
    string command;
    cout << "MathSync - Grid Module" << endl;
    cout << "Commands: SET, VIEW, EXIT" << endl;
    while (true) {
        cout << "\nEnter command: ";
        cin >> command;
        if (command == "SET") {
            string name, content;
            cin >> name >> content;
            setCell(name, content);
        }
        else if (command == "VIEW") {
            viewGrid();
        }
        else if (command == "EXIT") {
            break;
        }
        else {
            cout << "Unknown command." << endl;
        }
    }
    return 0;
}
