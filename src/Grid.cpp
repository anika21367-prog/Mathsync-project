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


