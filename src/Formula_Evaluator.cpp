/*Formula Evaluation module is under Pushpendra. To solve formulas using stack infix to postfix conversion.*/
#include<iostream>
#include<string>
#include<sstream>
#include<iomainip>
#include<cmath>
using namespace std;

const int ROWS = 5;
const int COLS = 5;
const int MAX = 200;

struct Cell{
    string content;
    bool isFormula = false;
};
Cell grid[ROWS][COLS];

bool parseCellName(string name, int &row, int &col) {
    if (name.length() != 2) return false;
    char letter = name[0];
    if (letter >= 'a' && letter <= 'z') letter = letter - 'a' + 'A';
    if (letter < 'A' || letter >= 'A' + COLS) return false;
    if (name[1] < '1' || name[1] >= '1' + ROWS) return false;
    col = letter - 'A';
    row = name[1] - '1';
    return true;

bool isDigit(char ch) {
    return ch >= '0' && ch <= '9';
}
bool isLetter(char ch) {
    return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z');
}
bool isOperator(string token) {
    return token == "+" || token == "-" || token == "*" || token == "/";
}

int precedence(string token) {
    if (token == "~") return 3; 
    if (token == "*" || token == "/") return 2;
    if (token == "+" || token == "-") return 1;
    return 0;
}

bool readNumber(string text, double &number) {
    istringstream input(text);
    if (!(input >> number)) return false;
    input >> ws;
    return input.eof() && isfinite(number);
}

bool tokenize(string expression, string tokens[], int &count, string &error) {
    count = 0;
    if (expression.length() > MAX) {
        error = "Formula is too long (maximum 200 characters after '=')";
        return false;
    }
    int i = 0;
    while (i < int(expression.length())) {
        char ch = expression[i];
        if (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r') {
            i++;
        } else if (isDigit(ch) || ch == '.') {
            string number;
            while (i < int(expression.length()) &&
                   (isDigit(expression[i]) || expression[i] == '.')) {
                number += expression[i];
                i++;
            }
            double value;
            if (!readNumber(number, value)) {
                error = "Invalid number: " + number;
                return false;
            }
            tokens[count++] = number;
} else if (isLetter(ch)) {
            string name;
            name += ch;
            i++;
            while (i < int(expression.length()) && isDigit(expression[i])) {
                name += expression[i];
                i++;
            }
            int row, col;
            if (!parseCellName(name, row, col)) {
                error = "Invalid cell reference: " + name;
                return false;
            }
            tokens[count++] = name;
        } else if (ch == '+' || ch == '-' || ch == '*' || ch == '/' ||
                   ch == '(' || ch == ')') {
            tokens[count++] = string(1, ch);
            i++;
        } else {
            error = "Unexpected character: " + string(1, ch);
            return false;
        }
    }
    return true;

bool toPostfix(string tokens[], int count, string postfix[],
               int &length, string &error) {
    string operators[MAX];
    int top = -1;
    length = 0;
    bool needValue = true;
    for (int i = 0; i < count; i++) {
        string token = tokens[i];
        if (token == "(") {
            if (!needValue) { error = "Missing operator before '('"; return false; }
            operators[++top] = token;
        } else if (token == ")") {
            if (needValue) { error = "Missing value before ')'"; return false; }
            while (top >= 0 && operators[top] != "(") {
                postfix[length++] = operators[top--];
            }
            if (top == -1) { error = "Mismatched brackets"; return false; }
            top--;
        } else if (isOperator(token)) {
            if (needValue) {
                if (token != "-") { error = "Expected a number or cell"; return false; }
                operators[++top] = "~";
            } else {
                while (top >= 0 && operators[top] != "(" &&
                       precedence(operators[top]) >= precedence(token)) {
                    postfix[length++] = operators[top--];
                }
                operators[++top] = token;
                needValue = true;
            }
        } else {
            if (!needValue) { error = "Missing operator between values"; return false; }
            postfix[length++] = token;
            needValue = false;
        }
    }
    if (needValue) { error = "Empty or incomplete formula"; return false; }
    while (top >= 0) {
        if (operators[top] == "(") { error = "Mismatched brackets"; return false; }
        postfix[length++] = operators[top--];
    }
    return true;


bool evaluateExpression(string expression, double &result, string &error,
                        bool visiting[ROWS][COLS]);
bool getCellValue(string name, double &result, string &error,
                  bool visiting[ROWS][COLS]) {
    int row, col;
    if (!parseCellName(name, row, col)) {
        error = "Invalid cell reference: " + name;
        return false;
    }
    if (visiting[row][col]) {
        error = "Circular reference at " + name;
        return false;
    }
    string content = grid[row][col].content;
    if (content.empty()) { result = 0; return true; }
    if (!grid[row][col].isFormula) {
        if (!readNumber(content, result)) {
            error = name + " does not contain a number";
            return false;
        }
        return true;
    }
    visiting[row][col] = true;
    bool success = evaluateExpression(content.substr(1), result, error, visiting);
    visiting[row][col] = false; 
    return success;
}

bool evaluateExpression(string expression, double &result, string &error, bool visiting[ROWS][COLS]){
    string tokens[MAX], postfix[MAX];
    int count, length;
    if(!tokenize(expression, tokens, count, error)) return false;
    if(!toPostfix(tokens, count, postfix, length, error)) return false;

    double values[MAX];
    int top = -1;
    for (int i = 0; i < length; i++) {
        string token = postfix[i];
        if (token == "~") {
            if (top < 0) { error = "Missing value after '-'"; return false; }
            values[top] = -values[top];
        } else if (isOperator(token)) {
            if (top < 1) { error = "Invalid formula"; return false; }
            double right = values[top--];
            double left = values[top--];
            double answer;
         if (token == "+") answer = left + right;
            else if (token == "-") answer = left - right;
            else if (token == "*") answer = left * right;
            else {
                if (right == 0) { error = "Division by zero"; return false; }
                answer = left / right;
            }
            if (!isfinite(answer)) { error = "Numeric result out of range"; return false; }
            values[++top] = answer;


