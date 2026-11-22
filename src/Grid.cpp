#include "mathsync.h"
#include <iostream>
#include <iomanip>
#include <cctype>
#include <cstdlib>
using namespace std;

int activeRows = 10;
int activeCols = 10;
Cell grid[MAX_ROWS][MAX_COLS];

int activeTotalCells() { return activeRows * activeCols; }


