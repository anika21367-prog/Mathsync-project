#ifndef MATHSYNC_H
#define MATHSYNC_H
#include <string>
using namespace std;

const int ROWS = 5;
const int COLS = 5;
const int TOTAL = ROWS * COLS;
const int MAX_HISTORY = 100;

struct Cell {
    string content;
    bool isFormula = false;
};

extern Cell grid[ROWS][COLS];

// grid.cpp
int cellNumber(string name);
bool parseCellName(string name, int &row, int &col);
string cellName(int cell);
void viewGrid();

// formula.cpp
bool parseCellName(string name, int &row, int &col);
bool evaluateFormula(const string &formula, double &result, string &error);
string formatNumber(double number);
void evaluateCell(string name);

// dependency.cpp
void addDependent(int source, int dependent);
void removeOldDependencies(int cell);
void freeAllLists();
bool findReferences(string formula, bool references[]);
bool createsCycle(int cell, bool references[]);
void recalculateCell(int cell);
void propagateUpdate(int changed);
void setCell(string name, string content);
void showDependents(string name);

// history.cpp
string show(string content);
bool canRecordChange();
void recordChange(int cell, string oldContent, string newContent);
void undo();
void redo();
void showHistory();
bool saveToFile(const string &filename);
bool loadFromFile(const string &filename);
#endif
