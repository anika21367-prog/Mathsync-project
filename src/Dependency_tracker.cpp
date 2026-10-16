/*Dependency Tracker module is under Anika. Dependency Tracker is implemented using a linked list and queues.*/
#include <iostream>
#include <string>
#include <iomanip>
using namespace std;

const int ROWS = 5;
const int COLS = 5;
const int TOTAL = ROWS * COLS;

    string content;
    bool isFormula = false;
};
Cell grid[ROWS][COLS];

struct Node {
    int cell;             
    Node *next;
};
Node *head[TOTAL] = {};   

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

void removeDependent(int source, int dependent) {
    Node *current = head[source];
    Node *previous = nullptr;
    while (current != nullptr) {
        if (current->cell == dependent) {
            if (previous == nullptr) {
                head[source] = current->next;
            } else {
                previous->next = current->next;
            }
            delete current;
            return;
        }
        previous = current;
        current = current->next;
    }
}

void removeOldDependencies(int cell) {
    for (int source = 0; source < TOTAL; source++) {
        removeDependent(source, cell);
    }
}

bool findReferences(string formula, bool references[]) {
    int i = 1;                 // Skip '='
    while (i < int(formula.length())) {
        char ch = formula[i];
        bool letter = (ch >= 'A' && ch <= 'Z') ||
                      (ch >= 'a' && ch <= 'z');
        if (letter) {
            string name;
            name += ch;
            i++;
            while (i < int(formula.length()) &&
                   formula[i] >= '0' && formula[i] <= '9') {
                name += formula[i];
                i++;
            }
            int cell = cellNumber(name);
            if (cell == -1) {
                cout << "Invalid or unsupported reference: " << name << endl;
                return false;
            }
            references[cell] = true;
        } else {
            i++;
        }
    }
    return true;
}

bool canReach(int start, int target) {
    bool visited[TOTAL] = {};
    SimpleQueue q;
    q.push(start);
    visited[start] = true;
    while (!q.empty()) {
        int cell = q.pop();
        if (cell == target) return true;
        Node *current = head[cell];
        while (current != nullptr) {
            int nextCell = current->cell;
            if (!visited[nextCell]) {
                visited[nextCell] = true;
                q.push(nextCell);
            }
            current = current->next;
        }
    }
    return false;
}

bool createsCycle(int cell, bool references[]) {
    for (int source = 0; source < TOTAL; source++) {
        if (references[source] && canReach(cell, source)) {
            return true;
        }
    }
    return false;
}

void recalculateCell(int cell) {
    int row = cell / COLS;
    int col = cell % COLS;
    cout << "Recalculating " << cellName(cell)
         << " (" << grid[row][col].content << ")" << endl;
}

void propagateUpdate(int changed) {
    bool affected[TOTAL] = {};
    SimpleQueue q;
    affected[changed] = true;
    q.push(changed);


    while (!q.empty()) {
        int cell = q.pop();
        Node *current = head[cell];
        while (current != nullptr) {
            int dependent = current->cell;
            if (!affected[dependent]) {
                affected[dependent] = true;
                q.push(dependent);
            }
            current = current->next;
        }
    }

    // Step 2: count how many affected inputs each cell must wait for.
    int waiting[TOTAL] = {};
    for (int source = 0; source < TOTAL; source++) {
        if (affected[source]) {
            Node *current = head[source];
            while (current != nullptr) {
                waiting[current->cell]++;
                current = current->next;
            }
        }
    }

    SimpleQueue ready;
    ready.push(changed);
    int updated = 0;
    while (!ready.empty()) {
        int cell = ready.pop();
        if (cell != changed) {
            recalculateCell(cell);
            updated++;
        }
        Node *current = head[cell];
        while (current != nullptr) {
            int dependent = current->cell;
            waiting[dependent]--;
            if (waiting[dependent] == 0) ready.push(dependent);
            current = current->next;
        }
    }
    if (updated == 0) cout << "No dependent cells to update." << endl;
}

void setCell(string name, string content) {
    int cell = cellNumber(name);
    if (cell == -1) {
        cout << "Invalid cell. Use A1 to E5." << endl;
        return;
    }
    bool formula = !content.empty() && content[0] == '=';
    bool references[TOTAL] = {};
    if (formula) {
        if (!findReferences(content, references)) return;
        if (createsCycle(cell, references)) {
            cout << "Rejected: circular reference." << endl;
            return;
        }
    }

    removeOldDependencies(cell);
    int row = cell / COLS;
    int col = cell % COLS;
    grid[row][col].content = content;
    grid[row][col].isFormula = formula;
    for (int source = 0; source < TOTAL; source++) {
        if (references[source]) addDependent(source, cell);
    }
    cout << cellName(cell) << " updated to " << content << endl;
    if (formula) recalculateCell(cell);
    propagateUpdate(cell);
}

void showDependents(string name) {
    int cell = cellNumber(name);
    if (cell == -1) {
        cout << "Invalid cell. Use A1 to E5." << endl;
        return;
    }
    cout << cellName(cell) << " -> ";
    Node *current = head[cell];
    while (current != nullptr) {
        cout << cellName(current->cell) << " -> ";
        current = current->next;
    }
    cout << "NULL" << endl;
}

void viewGrid() {
    cout << left << setw(6) << "";
    for (int col = 0; col < COLS; col++) {
        cout << setw(16) << char('A' + col);
    }
    cout << endl;
    for (int row = 0; row < ROWS; row++) {
        cout << setw(6) << row + 1;
        for (int col = 0; col < COLS; col++) {
            string text = grid[row][col].content;
            if (text.empty()) text = "-";
            cout << setw(16) << text;
        }
        cout << endl;
    }
}

void freeAllLists() {
    for (int cell = 0; cell < TOTAL; cell++) {
        while (head[cell] != nullptr) {
            Node *old = head[cell];
            head[cell] = head[cell]->next;
            delete old;
        }
    }
}

int main() {
    string command;
    cout << "MathSync - Dependency Tracker" << endl;
    cout << "Commands: SET B1 =A1+5, DEPS A1, VIEW, EXIT" << endl;
    cout << "Use uppercase commands and no spaces inside formulas." << endl;
    while (true) {
        cout << "\nEnter command: ";
        if (!(cin >> command)) break;
        if (command == "SET") {
            string name, content;
            if (!(cin >> name >> content)) break;
            setCell(name, content);
        } else if (command == "DEPS") {
            string name;
            if (!(cin >> name)) break;
            showDependents(name);
        } else if (command == "VIEW") {
            viewGrid();
        } else if (command == "EXIT") {
            break;
        } else {
            cout << "Unknown command." << endl;
            string rest;
            getline(cin, rest);
        }
    }
    freeAllLists();
    return 0;
}
