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
   char colmChar= toupper(cellName[0]);
   if (colmChar< 'A' || colmChar> ('A' + colms- 1))
      return false;
   colm= colmChar- 'A';
   string rowpart= cellName.substr(1);
   for (char c: rowpart){
       if (!isdigit(c))
          return false;
   }
   int rowNumb= stoi(rowPart);
   if (rowNumb< 1 || rowNumb> ROWS)
      return false;
   row= rowNumb- 1;
   return true;
}
void setCell(const string &cellName, const string &content){
   int row, colm;
   if (!parsecellName(cellName, row, colm)){
      cout<<"Invalid cell reference: "<< cellName<< endl;
      return;
   }
   grid[row][colm].content= content;
   grid[row][colm].isFormula= (!content.empty() && content[0]== '=');
   cout<< cellName<<"updated to<< content<< endl;
}
void viewGrid(){
   cout<<"     ";
   for (int c= 0; c< COLMS; c++){
       cout<< setw(10)<< left<< string(1, 'A+c');
   }
   cout<< endl;
   for (int r= 0; r< ROWS; r++){
       cout<< setw(5)<< left<< (r+1);
       for (int c= 0; c< COLMS; c++){
           string display= grid[r][c].content.empty() ? "-" : grid[r][c].content;
           cout<< setw(10)<< left<< display;
       }
   cout<< endl;
   }
}
