/*Formula Evaluation module is under Pushpendra. To solve formulas using stack infix to postfix conversion.*/
#include<iostream>
#include<string>
#include<sstream>
#include<iomainip>
#include<cmath>
using namespace std;

const int ROWS = 5;
const int COLS = 5;
const int MAX = 200;

struct Cell{
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
    col = letter - 'A';
    row = name[1] - '1';
    return true;
