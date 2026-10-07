/*Formula Evaluation module is under Pushpendra. To solve formulas using stack infix to postfix conversion.*/
#include <iostream>
#include <string>
#include <vector>
#include <stack>
#include <sstream>
#include <manip>
#include <cctype>
#include <stdexcept>

using namespace std;

const int ROWS = 5;
const int COLS = 5;

struct Cell 
{
  string content = "";
  bool isFormula = false;
}

Cell grid[ROWS][COLS];
bool parsecCellName(const string &cellName, int &row, int &col)
{
  if(cellName.size() < 2)
    return false;
  char colChar = toupper(cellName[0]);
  if(colChar < 'A' || colChar > ('A' + COLS - 1))
    return false;
  col = colChar - 'A';
  string rowPart = cellName.substr(1);
  for(char c : rowPart)
    {
      if(!isdigit(c))
        return false;
    }
  int rowNum = stoi(rowPart);
  if(rowNum < 1 || rowNum > ROWS)
    return false;
  row = rowNum - 1;
  return true;
}
void setCell(const string &cellName, const string &content)
{
  int row, col;
  if(!parseCellName(cellName, row, col))
  {
      cout << "Invalid Cell reference: " << cellName << endl;
      return;
  }
  grid[row][col].content = content;
  grid[row][col].isFormula = (!content.empty() && content[0] == '=');
  cout << cellName << " updated to " << endl;
}
