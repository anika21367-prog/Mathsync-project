/* Dependency Tracker module is under Anika (Resizable Grid Upgrade) */
#include "mathsync.h"
#include <iostream>
#include <string>
#include <iomanip>
using namespace std;

struct Node { int cell; Node *next; };
Node *head[MAX_TOTAL] = {};

struct SimpleQueue {
    int items[MAX_TOTAL];
    int front = 0, rear = 0;
    bool empty() { return front == rear; }
    void push(int value) { if (rear < MAX_TOTAL) items[rear++] = value; }
    int pop() { return items[front++]; }
};

void addDependent(int source, int dependent) {
    if (source < 0 || source >= activeTotalCells() ||
        dependent < 0 || dependent >= activeTotalCells()) return;
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

void removeDependent(int source, int dependent) {
    Node *current = head[source], *previous = nullptr;
    while (current != nullptr) {
        if (current->cell == dependent) {
            if (previous == nullptr) head[source] = current->next;
            else previous->next = current->next;
            delete current;
            return;
        }
        previous = current;
        current = current->next;
    }
}

void removeOldDependencies(int cell) {
    for (int source = 0; source < activeTotalCells(); source++) removeDependent(source, cell);
}

bool findReferences(string formula, bool references[]) {
    int i = 1;
    while (i < int(formula.length())) {
        char ch = formula[i];
        bool letter = (ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z');
        if (letter) {
            string name;
            name += ch;
            i++;
            while (i < int(formula.length()) && formula[i] >= '0' && formula[i] <= '9') {
                name += formula[i];
                i++;
            }
            int cell = cellNumber(name);
            if (cell == -1) {
                cout << "Invalid or out-of-bounds reference: " << name << endl;
                return false;
            }
            references[cell] = true;
        } else i++;
    }
    return true;
}

bool canReach(int start, int target) {
    bool visited[MAX_TOTAL] = {};
    SimpleQueue q;
    q.push(start);
    visited[start] = true;
    while (!q.empty()) {
        int cell = q.pop();
        if (cell == target) return true;
        Node *current = head[cell];
        while (current != nullptr) {
            int nextCell = current->cell;
            if (nextCell < activeTotalCells() && !visited[nextCell]) {
                visited[nextCell] = true;
                q.push(nextCell);
            }
            current = current->next;
        }
    }
    return false;
}

bool createsCycle(int cell, bool references[]) {
    for (int source = 0; source < activeTotalCells(); source++) {
        if (references[source] && canReach(cell, source)) return true;
    }
    return false;
}

void recalculateCell(int cell) {
    int row = cell / activeCols, col = cell % activeCols;
    double result;
    string error;
    if (evaluateFormula(grid[row][col].content, result, error)) {
        cout << "Updated " << cellName(cell) << " = " << formatNumber(result) << endl;
    } else {
        cout << "Error in " << cellName(cell) << ": " << error << endl;
    }
}

void propagateUpdate(int changed) {
    bool affected[MAX_TOTAL] = {};
    SimpleQueue q;
    affected[changed] = true;
    q.push(changed);
    while (!q.empty()) {
        int cell = q.pop();
        Node *current = head[cell];
        while (current != nullptr) {
            int dependent = current->cell;
            if (dependent < activeTotalCells() && !affected[dependent]) {
                affected[dependent] = true;
                q.push(dependent);
            }
            current = current->next;
        }
    }
    int waiting[MAX_TOTAL] = {};
    for (int source = 0; source < activeTotalCells(); source++) {
        if (affected[source]) {
            Node *current = head[source];
            while (current != nullptr) {
                if (current->cell < activeTotalCells()) waiting[current->cell]++;
                current = current->next;
            }
        }
    }
    SimpleQueue ready;
    ready.push(changed);
    int updated = 0;
    while (!ready.empty()) {
        int cell = ready.pop();
        if (cell != changed) { recalculateCell(cell); updated++; }
        Node *current = head[cell];
        while (current != nullptr) {
            int dependent = current->cell;
            if (dependent < activeTotalCells()) {
                waiting[dependent]--;
                if (waiting[dependent] == 0) ready.push(dependent);
            }
            current = current->next;
        }
    }
    if (updated == 0) cout << "No dependent cells to update." << endl;
}

void setCell(string name, string content) {
    int cell = cellNumber(name);
    if (cell == -1) {
        cout << "Invalid cell. Use A1 to "
             << static_cast<char>('A' + activeCols - 1) << activeRows << "." << endl;
        return;
    }
    int row = cell / activeCols, col = cell % activeCols;
    string oldContent = grid[row][col].content;
    if (oldContent == content) {
        cout << cellName(cell) << " already contains " << show(content) << endl;
        return;
    }
    if (!canRecordChange()) {
        cout << "History is full. Edit not applied (maximum 100 changes)." << endl;
        return;
    }
    bool formula = !content.empty() && content[0] == '=';
    bool references[MAX_TOTAL] = {};
    if (formula) {
        if (!findReferences(content, references)) return;
        if (createsCycle(cell, references)) {
            cout << "Rejected: circular reference." << endl;
            return;
        }
    }
    recordChange(cell, oldContent, content);
    removeOldDependencies(cell);
    grid[row][col].content = content;
    grid[row][col].isFormula = formula;
    for (int source = 0; source < activeTotalCells(); source++) {
        if (references[source]) addDependent(source, cell);
    }
    cout << cellName(cell) << " updated to " << content << endl;
    if (formula) recalculateCell(cell);
    propagateUpdate(cell);
}

void showDependents(string name) {
    int cell = cellNumber(name);
    if (cell == -1) {
        cout << "Invalid cell. Use A1 to "
             << static_cast<char>('A' + activeCols - 1) << activeRows << "." << endl;
        return;
    }
    cout << cellName(cell) << " -> ";
    Node *current = head[cell];
    while (current != nullptr) {
        if (current->cell < activeTotalCells()) cout << cellName(current->cell) << " -> ";
        current = current->next;
    }
    cout << "NULL" << endl;
}

void freeAllLists() {
    for (int cell = 0; cell < MAX_TOTAL; cell++) {
        while (head[cell] != nullptr) {
            Node *old = head[cell];
            head[cell] = head[cell]->next;
            delete old;
        }
    }
}
