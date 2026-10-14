//grid.cpp
//MathSync- Grid Storage Module
#include<iostream>
#include<string>
#include<iomanip>
using namespace std;
const int ROWS= 5;
const int COLS= 5;
const int TOTAL= ROWS * COLS
struct cell{
   string content= "";
   bool isFormula= false;
};
CELL grid[ROWS][COLS];
int cellNumber(string name){
   if (name.length()!=2)
      return -1;
   char column= toupper(name[0]);
   char row= name[1];
   if (column < 'A' || column >= 'A' + COLS)
      return -1;
   if (row < '1' || row >= '1' + ROWS)
      return -1;
   return (row - '1') * COLS + (column - 'A');
}
string cellName(int cell){
   string name;
   name += char('A' + cell % COLS);
   name += char('1' + cell / COLS);
   return name;
}
void setCell(string name, string content){
   int cell = cellNumber(name);

    if (cell == -1)
    {
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
   


