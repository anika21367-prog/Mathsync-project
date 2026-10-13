/*Dependency Tracker module is under Anika. Dependency Tracker is implemented using linked list and queues.*/
#include <iostream>
#include <string>
#include <iomanip>
using namespace std;

const int ROWS = 5;
const int COLS = 5;
const int TOTAL = ROWS * COLS;

// Standalone grid: replace this with your team's grid when merging.
struct Cell {
    string content;
    bool isFormula = false;
};
Cell grid[ROWS][COLS];

struct Node {
    int cell;                 // Number of the dependent cell
    Node *next;
};
Node *head[TOTAL] = {};       // One linked list for each cell

struct SimpleQueue {
    int items[TOTAL];
    int front = 0;
    int rear = 0;

    bool empty() {
        return front == rear;
    }
    void push(int value) {
        items[rear] = value;
        rear++;
    }
    int pop() {
        int value = items[front];
        front++;
        return value;
    }
};
// Each traversal adds a cell at most once, so TOTAL slots are enough.

// Cell numbers: A1=0, B1=1, ..., E1=4, A2=5, ..., E5=24.
// Return -1 for an invalid name. Accept A1 or a1, but not A01.
int cellNumber(string name) {
    if (name.length() != 2) return -1;

    char column = name[0];
    if (column >= 'a' && column <= 'z') {
        column = column - 'a' + 'A';
    }
    char row = name[1];
    if (column < 'A' || column >= 'A' + COLS) return -1;
    if (row < '1' || row >= '1' + ROWS) return -1;

    return (row - '1') * COLS + (column - 'A');
}

string cellName(int cell) {
    string name;
    name += char('A' + cell % COLS);
    name += char('1' + cell / COLS);
    return name;
}

// If B1 uses A1, add B1 to A1's list.
void addDependent(int source, int dependent) {
    Node *current = head[source];
    while (current != nullptr) {
        if (current->cell == dependent) return;
        current = current->next;
    }
    Node *newNode = new Node;
    newNode->cell = dependent;
    newNode->next = head[source];
    head[source] = newNode;
}
