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


