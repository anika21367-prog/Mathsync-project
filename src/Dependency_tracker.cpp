/*Dependency Tracker module is under Anika. Dependency Tracker is implemented using linked list and queues.*/
#include<iostream>
#include<queue>
#include<vector>
#include<string>
#include<iomanip>
#include<cctype>
using namespace std;
const int ROWS=5;
const int COLS=5;

struct Cell{
    string cont="";
    bool isFormula=false;
};
 Cell grid[ROWS][COLS];
bool parseCellname(const string &cellname, int &row, int &col){
    if (cellname.size()<2)
    return false;

    char colChar=toupper(cellname[0]);
    if(colChar<'A'||colChar>('A'+COLS-1)) return false;
    col=colChar-'A';

    string rowPart=cellname.substr(1);
    for(char c: rowPart){
        if(!isdigit(c)) return false;
}
int rowNum=stoi(rowPart);
    if(rowNum<1||rowNum>ROWS) return false;
    row=rowNum-1;
    return true;
 }

 struct DependentNode{
    string cellname;
    DependentNode *next;
 };
 DependentNode *dependents[ROWS][COLS]={};

 DependentNode *&listOf(const string &cellname){
    int row, col;
    parseCellname(cellname, row, col);
    return dependents[row][col];
 }

 string makecellname(int row, int col){
    return string(1, 'A'+col)+to_string(row+1);
 }

 string upper(string s){
    for(char &c:s)c=toupper(c);
    return s;
}
