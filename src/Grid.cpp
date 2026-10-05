//grid.cpp
//MathSync- Grid Storage Module
#include<iostream>
#include<string>
#include<iomanip>
using namespace std;
const int ROWS= 5;
const int COLMS= 5;
struct cell{
   string content= "";
   bool isFormula= false;
};
CELL grid[ROWS][COLMS];
bool parsecellName(const string &cellName, int &row, int &colm){
   if (cellName.size()< 2)
     return false;
