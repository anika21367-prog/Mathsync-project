// web_server.cpp - small browser frontend for MathSync
// Compile instead of main.cpp:
//   g++ web_server.cpp grid.cpp formula.cpp dependency.cpp history.cpp -o mathsync-web
// Then open: http://localhost:8080
ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#include "mathsync.h"
#include <iostream>
#include <sstream>
#include <string>
#include <cstdlib>
#include <cctype>
using namespace std;

string runCommand(string command) {
    ostringstream output;
    streambuf *old = cout.rdbuf(output.rdbuf());
    istringstream input(command);
    string action, name;
    if (!(input >> action)) cout << "Empty command." << endl;
    else if (action == "SET") {
        string content;
        if (input >> name) {
            input >> ws;
            getline(input, content);
            if (!content.empty()) setCell(name, content);
            else cout << "Use CLEAR to empty a cell." << endl;
        } else cout << "Usage: SET A1 10" << endl;
    } else if (action == "CLEAR") {
        if (input >> name) setCell(name, "");
        else cout << "Usage: CLEAR A1" << endl;
    } else if (action == "EVAL") {
        if (input >> name) evaluateCell(name);
        else cout << "Usage: EVAL B1" << endl;
    } else if (action == "DEPS") {
        if (input >> name) showDependents(name);
        else cout << "Usage: DEPS A1" << endl;
    } else if (action == "RESIZE") {
        int r, c;
        if (input >> r >> c) resizeGrid(r, c);
        else cout << "Usage: RESIZE 12 10" << endl;
    } else if (action == "UNDO") undo();
    else if (action == "REDO") redo();
    else if (action == "HISTORY") showHistory();
    else if (action == "SYNC" || action == "LOAD") {
        string filename;
        if (input >> filename) {
            if (filename != "sheet.txt") cout << "Only sheet.txt is allowed." << endl;
            else if (action == "SYNC") saveToFile(filename);
            else loadFromFile(filename);
        } else cout << "Please give a file name." << endl;
    } else if (action == "VIEW") viewGrid();
    else cout << "Unknown command." << endl;
    cout.rdbuf(old);
    return output.str();
}

string jsonText(const string &text) {
    string result = "\"";
    const char hex[] = "0123456789abcdef";
    for (size_t i = 0; i < text.length(); i++) {
        unsigned char ch = static_cast<unsigned char>(text[i]);
        if (ch == '"' || ch == '\\') { result += '\\'; result += static_cast<char>(ch); }
        else if (ch == '\n') result += "\\n";
        else if (ch == '\r') result += "\\r";
        else if (ch == '\t') result += "\\t";
        else if (ch < 32) { result += "\\u00"; result += hex[ch >> 4]; result += hex[ch & 15]; }
        else result += static_cast<char>(ch);
    }
    return result + "\"";
}

string stateJSON() {
    string json = "{\"rows\":" + to_string(activeRows) +
                  ",\"cols\":" + to_string(activeCols) +
                  ",\"maxRows\":" + to_string(MAX_ROWS) +
                  ",\"maxCols\":" + to_string(MAX_COLS) + ",\"grid\":[";
    for (int row = 0; row < activeRows; row++) {
        if (row > 0) json += ",";
        json += "[";
        for (int col = 0; col < activeCols; col++) {
            if (col > 0) json += ",";
            string content = grid[row][col].content;
            string display = content, errorMsg = "";
            bool isFormula = !content.empty() && content[0] == '=';
            if (isFormula) {
                double result = 0;
                string error;
                if (evaluateFormula(content, result, error)) display = formatNumber(result);
                else { display = "ERR"; errorMsg = error; }
            }
            json += "{\"content\":" + jsonText(content) +
                    ",\"display\":" + jsonText(display) +
                    ",\"isFormula\":" + string(isFormula ? "true" : "false") +
                    ",\"error\":" + jsonText(errorMsg) + "}";
        }
        json += "]";
    }
    json += "],\"deps\":{";
    bool firstDep = true;
    for (int r = 0; r < activeRows; r++) {
        for (int c = 0; c < activeCols; c++) {
            if (!firstDep) json += ",";
            firstDep = false;
            string name = string(1, static_cast<char>('A' + c)) + to_string(r + 1);
            string chain = runCommand("DEPS " + name);
            while (!chain.empty() && (chain.back() == '\n' || chain.back() == '\r')) chain.pop_back();
            json += jsonText(name) + ":" + jsonText(chain);
        }
    }
    json += "},\"history\":" + jsonText(runCommand("HISTORY")) + "}";
    return json;
}

string httpResponse(const string &body, const string &type, const string &status = "200 OK") {
    return "HTTP/1.1 " + status + "\r\nContent-Type: " + type +
           "\r\nCache-Control: no-store\r\nConnection: close\r\nContent-Length: " +
           to_string(body.size()) + "\r\n\r\n" + body;
}

bool readRequest(SOCKET clientSocket, string &request) {
    const size_t MAX_REQUEST = 65536;
    char buffer[4096];
    size_t headerEnd = string::npos;
    while (headerEnd == string::npos) {
        int count = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (count <= 0) return false;
        request.append(buffer, count);
        if (request.size() > MAX_REQUEST) return false;
        headerEnd = request.find("\r\n\r\n");
    }
    size_t contentLength = 0;
    string headers = request.substr(0, headerEnd);
    istringstream headerStream(headers);
    string line;
    getline(headerStream, line);
    while (getline(headerStream, line)) {
        size_t colon = line.find(':');
        if (colon == string::npos) continue;
        string name = line.substr(0, colon);
        for (size_t i = 0; i < name.size(); i++)
            name[i] = static_cast<char>(tolower(static_cast<unsigned char>(name[i])));
        if (name == "transfer-encoding") return false;
        if (name == "content-length") {
            istringstream value(line.substr(colon + 1));
            long long length;
            if (!(value >> length) || length < 0 || length > static_cast<long long>(MAX_REQUEST)) return false;
            contentLength = static_cast<size_t>(length);
        }
    }
    size_t required = headerEnd + 4 + contentLength;
    if (required > MAX_REQUEST) return false;
    while (request.size() < required) {
        int count = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (count <= 0) return false;
        request.append(buffer, count);
        if (request.size() > MAX_REQUEST) return false;
    }
    request.resize(required);
    return true;
}

bool sendAll(SOCKET clientSocket, const string &response) {
    size_t sent = 0;
    while (sent < response.size()) {
        size_t remaining = response.size() - sent;
        int chunk = remaining > 4096 ? 4096 : static_cast<int>(remaining);
        int count = send(clientSocket, response.data() + sent, chunk, 0);
        if (count <= 0) return false;
        sent += count;
    }
    return true;
}

string indexHTML = R"HTML(<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>MathSync — Resizable C++ DSA Spreadsheet</title>
<style>
  :root {
    --navy-950:#091526; --navy-900:#0f2342; --navy-800:#16325c; --blue-700:#1d4ed8;
    --blue-600:#2563eb; --blue-500:#3b82f6; --blue-50:#eff6ff; --ice-50:#f4f8fd;
    --slate-900:#0f172a; --slate-700:#334155; --slate-300:#cbd5e1; --slate-200:#e2e8f0;
    --white:#ffffff; --cyan-50:#ecfeff; --cyan-600:#0891b2; --amber-50:#fffbeb;
    --amber-600:#d97706; --red-50:#fef2f2; --red-600:#dc2626; --green-500:#10b981;
  }
  * { box-sizing: border-box; }
  body { margin: 0; font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif; background: var(--ice-50); color: var(--slate-900); min-height: 100vh; display: flex; flex-direction: column; }
  .topbar { background: linear-gradient(135deg, var(--navy-950), var(--navy-800)); color: var(--white); padding: 13px 24px; display: flex; align-items: center; justify-content: space-between; border-bottom: 1px solid rgba(255,255,255,0.12); flex-wrap: wrap; gap: 12px; }
  .brand { display: flex; align-items: center; gap: 12px; }
  .brand-logo { width: 36px; height: 36px; border-radius: 9px; background: linear-gradient(135deg, var(--blue-500), var(--cyan-600)); display: grid; place-items: center; font-weight: 800; font-size: 15px; }
  .brand h1 { margin: 0; font-size: 17.5px; font-weight: 700; }
  .brand span { display: block; font-size: 11.5px; color: #93c5fd; }
  .status-pills { display: flex; align-items: center; gap: 10px; flex-wrap: wrap; }
  .pill { background: rgba(255,255,255,0.08); border: 1px solid rgba(147,197,253,0.25); color: #e0f2fe; padding: 5px 11px; border-radius: 999px; font-size: 12px; display: flex; align-items: center; gap: 6px; }
  .dot { width: 8px; height: 8px; border-radius: 50%; background: var(--green-500); }
  .ribbon { background: var(--white); border-bottom: 1px solid var(--slate-200); padding: 10px 24px; display: flex; flex-direction: column; gap: 10px; }
  .toolbar { display: flex; align-items: center; justify-content: space-between; flex-wrap: wrap; gap: 8px; }
  .btn-group { display: flex; align-items: center; gap: 6px; flex-wrap: wrap; }
  .divider { width: 1px; height: 22px; background: var(--slate-200); margin: 0 3px; }
  button { font-family: inherit; font-size: 12px; font-weight: 600; padding: 6px 11px; border-radius: 7px; border: 1px solid var(--slate-300); background: var(--white); color: var(--navy-900); cursor: pointer; transition: all 0.14s ease; }
  button:hover:not(:disabled) { background: var(--blue-50); border-color: var(--blue-500); color: var(--blue-700); }
  button.primary { background: var(--blue-600); border-color: var(--blue-600); color: var(--white); }
  button.primary:hover:not(:disabled) { background: var(--blue-700); color: var(--white); }
  button.danger { color: var(--red-600); border-color: #fecaca; background: var(--red-50); }
  button:disabled { opacity: 0.55; cursor: wait; }
  .formula-bar { display: flex; align-items: center; background: var(--ice-50); border: 1px solid var(--slate-300); border-radius: 8px; overflow: hidden; }
  .cell-badge { background: var(--navy-900); color: var(--white); font-family: "Consolas", monospace; font-weight: 700; font-size: 13px; padding: 8px 14px; min-width: 62px; text-align: center; }
  .fx-label { padding: 0 10px; font-family: Georgia, serif; font-style: italic; font-weight: 700; color: var(--blue-600); font-size: 15px; border-right: 1px solid var(--slate-200); }
  #formulaInput { flex: 1; border: none; background: transparent; padding: 8px 12px; font-family: "Consolas", monospace; font-size: 13.5px; outline: none; }
  .workspace { display: grid; grid-template-columns: 1.45fr 1fr; gap: 18px; padding: 18px 24px 28px; max-width: 1560px; width: 100%; margin: 0 auto; flex: 1; }
  @media (max-width: 1050px) { .workspace { grid-template-columns: 1fr; } }
  .panel { background: var(--white); border: 1px solid var(--slate-200); border-radius: 12px; box-shadow: 0 4px 14px rgba(15,35,66,0.05); display: flex; flex-direction: column; overflow: hidden; }
  .panel-header { padding: 11px 16px; background: linear-gradient(to right, var(--white), var(--ice-50)); border-bottom: 1px solid var(--slate-200); display: flex; align-items: center; justify-content: space-between; flex-wrap: wrap; gap: 8px; }
  .panel-title { font-size: 13.5px; font-weight: 700; color: var(--navy-900); }
  .legend { display: flex; gap: 10px; font-size: 11px; color: var(--slate-700); flex-wrap: wrap; }
  .legend span { display: inline-flex; align-items: center; gap: 4px; }
  .swatch { width: 10px; height: 10px; border-radius: 3px; display: inline-block; }
  .sheet-wrap { padding: 14px; overflow: auto; max-height: 540px; }
  table.grid-table { border-collapse: separate; border-spacing: 0; border: 1px solid var(--slate-300); border-radius: 8px; }
  .grid-table th { position: sticky; top: 0; z-index: 3; background: var(--navy-900); color: var(--white); font-size: 12px; font-weight: 700; padding: 8px 6px; text-align: center; border-right: 1px solid rgba(255,255,255,0.12); min-width: 86px; }
  .grid-table th:first-child { left: 0; z-index: 4; min-width: 44px; background: var(--navy-950); }
  .grid-table td.rowhead { position: sticky; left: 0; z-index: 2; background: var(--blue-50); color: var(--navy-900); font-weight: 700; font-size: 12px; text-align: center; border-top: 1px solid var(--slate-200); border-right: 1px solid var(--slate-300); user-select: none; }
  .grid-table td.cell-td { border-top: 1px solid var(--slate-200); border-right: 1px solid var(--slate-200); padding: 0; position: relative; min-width: 86px; background: var(--white); }
  .grid-table td.cell-td.is-formula { background: #f5f9ff; }
  .grid-table td.cell-td.is-precedent { background: var(--amber-50) !important; box-shadow: inset 0 0 0 2px var(--amber-600); }
  .grid-table td.cell-td.is-dependent { background: var(--cyan-50) !important; box-shadow: inset 0 0 0 2px var(--cyan-600); }
  .grid-table td.cell-td.is-error { background: var(--red-50) !important; }
  .grid-table td.cell-td.is-selected { box-shadow: inset 0 0 0 2.5px var(--blue-600) !important; z-index: 2; }
  .cell-tag { position: absolute; top: 2px; right: 4px; font-size: 9px; font-weight: 700; color: var(--blue-600); pointer-events: none; }
  .is-error .cell-tag { color: var(--red-600); }
  .cell-input { width: 100%; height: 38px; border: none; background: transparent; padding: 8px 8px 4px; font-family: "Consolas", monospace; font-size: 12.5px; color: var(--slate-900); outline: none; }
  .is-error .cell-input { color: var(--red-600); font-weight: 700; }
  .inspector-panel { background: var(--navy-950); color: #e2e8f0; border-color: var(--navy-800); }
  .inspector-panel .panel-header { background: var(--navy-900); border-bottom: 1px solid rgba(147,197,253,0.18); }
  .inspector-panel .panel-title { color: var(--white); }
  .dsa-grid { padding: 14px 16px; display: flex; flex-direction: column; gap: 11px; flex: 1; }
  .dsa-card { background: rgba(22,50,92,0.55); border: 1px solid rgba(147,197,253,0.2); border-radius: 9px; padding: 10px 12px; }
  .dsa-card-title { font-size: 11px; text-transform: uppercase; letter-spacing: 0.6px; color: #93c5fd; font-weight: 700; margin-bottom: 6px; display: flex; justify-content: space-between; }
  .dsa-card-title span.tag { background: rgba(59,130,246,0.25); color: #bfdbfe; padding: 2px 6px; border-radius: 4px; font-size: 10px; text-transform: none; }
  .chain-box { font-family: "Consolas", monospace; font-size: 12px; color: #f8fafc; background: rgba(9,21,38,0.75); padding: 7px 10px; border-radius: 6px; border-left: 3px solid var(--blue-500); word-break: break-word; }
  .token-row { display: flex; flex-wrap: wrap; gap: 5px; margin-top: 6px; }
  .tok { font-family: "Consolas", monospace; font-size: 11px; padding: 2px 7px; border-radius: 5px; background: rgba(37,99,235,0.28); border: 1px solid rgba(96,165,250,0.4); color: #dbeafe; }
  .tok.op { background: rgba(8,145,178,0.28); border-color: rgba(34,211,238,0.45); color: #cffafe; }
  #log { font-family: "Consolas", monospace; font-size: 12px; line-height: 1.45; background: #060e1a; color: #bae6fd; padding: 9px 11px; border-radius: 7px; border: 1px solid rgba(147,197,253,0.16); height: 130px; overflow-y: auto; white-space: pre-wrap; margin: 0; }
  .cli-bar { display: flex; gap: 6px; margin-top: 8px; }
  .cli-bar input { flex: 1; background: #060e1a; border: 1px solid rgba(147,197,253,0.28); color: var(--white); padding: 6px 10px; border-radius: 6px; font-family: "Consolas", monospace; font-size: 12px; outline: none; }
</style>
</head>
<body>
<header class="topbar">
  <div class="brand">
    <div class="brand-logo">M∑</div>
    <div>
      <h1>MathSync Engine</h1>
      <span>Resizable C++ Spreadsheet • Linked-List DAG • Shunting-Yard RPN • Dual Stacks</span>
    </div>
  </div>
  <div class="status-pills">
    <div class="pill"><span class="dot"></span> <span id="syncStatus">Live C++ Socket Sync</span></div>
    <div class="pill" id="gridSizePill">Grid: 10×10 (Max 30×26)</div>
  </div>
</header>
<section class="ribbon">
  <div class="toolbar">
    <div class="btn-group">
      <button onclick="quick('UNDO')">↶ Undo</button>
      <button onclick="quick('REDO')">↷ Redo</button>
      <button onclick="quick('HISTORY')">📜 History</button>
      <div class="divider"></div>
      <button onclick="stepSize(1, 0)">+ Row</button>
      <button onclick="stepSize(-1, 0)">- Row</button>
      <button onclick="stepSize(0, 1)">+ Col</button>
      <button onclick="stepSize(0, -1)">- Col</button>
      <button onclick="resizeTo(10, 10)">10×10</button>
      <button onclick="resizeTo(15, 12)">15×12</button>
      <button onclick="resizeTo(25, 20)">25×20</button>
      <div class="divider"></div>
      <button class="primary" onclick="quick('SYNC sheet.txt')">💾 Sync</button>
      <button onclick="quick('LOAD sheet.txt')">📂 Load</button>
      <button onclick="quick('VIEW')">⟳ Refresh</button>
    </div>
    <div class="btn-group">
      <button onclick="loadGroupLabDemo()">👥 Group Lab Demo</button>
      <button onclick="loadCascadeDemo()">🔗 Chain Demo</button>
      <button class="danger" onclick="testCycleDemo()">⚠ Cycle Test</button>
    </div>
  </div>
  <div class="formula-bar">
    <div class="cell-badge" id="activeCellBadge">A1</div>
    <div class="fx-label">fx</div>
    <input id="formulaInput" type="text" placeholder="Enter number or formula (e.g., =A1+B1+C1)..." autocomplete="off" spellcheck="false">
  </div>
</section>
<main class="workspace">
  <section class="panel">
    <div class="panel-header">
      <div class="panel-title" id="matrixTitle">Spreadsheet Matrix (10 × 10)</div>
      <div class="legend">
        <span><i class="swatch" style="background:#2563eb;"></i> Selected</span>
        <span><i class="swatch" style="background:#d97706;"></i> Precedents</span>
        <span><i class="swatch" style="background:#0891b2;"></i> Dependents</span>
        <span><i class="swatch" style="background:#dc2626;"></i> Error</span>
      </div>
    </div>
    <aside class="panel inspector-panel">
    <div class="panel-header">
      <div class="panel-title">Live C++ Data Structures Inspector</div>
      <span style="font-size:11px;color:#93c5fd;" id="inspectTarget">Cell: A1</span>
    </div>
    <div class="dsa-grid">
      <div class="dsa-card">
        <div class="dsa-card-title">1. Adjacency Linked List <span class="tag">dependency_tracker.cpp</span></div>
        <div class="chain-box" id="depChainBox">A1 -> NULL</div>
        <div style="font-size:11.5px;color:#cbd5e1;margin-top:6px;" id="refSummary">Upstream inputs: None • Downstream dependents: None</div>
      </div>
      <div class="dsa-card">
        <div class="dsa-card-title">2. Shunting-Yard & Postfix (RPN) Stack <span class="tag">formula_evaluator.cpp</span></div>
        <div class="chain-box" id="rpnBox">Select a formula cell (=...) to inspect postfix stack</div>
        <div class="token-row" id="rpnTokens"></div>
      </div>
      <div class="dsa-card">
        <div class="dsa-card-title">3. Undo / Redo Stack State <span class="tag">history.cpp</span></div>
        <div class="chain-box" id="historyBox" style="max-height:72px;overflow-y:auto;white-space:pre-wrap;">History is empty.</div>
      </div>
      <div class="dsa-card" style="flex:1;display:flex;flex-direction:column;">
        <div class="dsa-card-title">4. C++ Kernel Stdout & CLI <span class="tag">POST /api/command</span></div>
        <pre id="log">Connecting to MathSync C++ server...</pre>
        <form class="cli-bar" onsubmit="runCliInput(event)">
          <input id="cliCommand" type="text" placeholder="Try: RESIZE 15 12, SET B2 =A2*2, DEPS A2..." autocomplete="off">
          <button type="submit" class="primary">Run</button>
        </form>
      </div>
    </div>
  </aside>
</main>
<script>
let busy = false, polling = false, commandGeneration = 0, lastStateSnapshot = '', selectedCell = 'A1';
let currentState = { rows: 10, cols: 10, maxRows: 30, maxCols: 26, grid: [], deps: {}, history: '' };

function colToLetter(col) { return String.fromCharCode(65 + col); }
function parseCellCoords(cell) {
  return { col: cell.charCodeAt(0) - 65, row: parseInt(cell.slice(1), 10) - 1 };
}
function setBusy(value) {
  busy = value;
  document.querySelectorAll('button, .cell-input, #formulaInput').forEach(el => { el.disabled = value; });
}
function appendLog(text) {
  document.getElementById('log').textContent = (text && text.trim()) ? text.trim() : 'Command executed.';
}
async function sendCommand(command, silent = false) {
  if (busy) return null;
  commandGeneration++;
  setBusy(true);
  const controller = new AbortController();
  const timer = setTimeout(() => controller.abort(), 10000);
  try {
    const response = await fetch('/api/command', {
      method: 'POST', headers: { 'Content-Type': 'text/plain; charset=UTF-8' },
      body: command, cache: 'no-store', signal: controller.signal
    });
    if (!response.ok) throw new Error('HTTP ' + response.status);
    const data = await response.json();
    if (!silent) appendLog(data.output);
    applyState(data);
    return data;
  } catch (err) {
    appendLog('Server request failed. Verify mathsync-web.exe is running and click Refresh.');
    throw err;
  } finally { clearTimeout(timer); setBusy(false); }
}
function quick(command) { sendCommand(command).catch(() => {}); }
function resizeTo(r, c) { quick(`RESIZE ${r} ${c}`); }
function stepSize(dr, dc) {
  const nextR = Math.min(currentState.maxRows || 30, Math.max(1, (currentState.rows || 10) + dr));
  const nextC = Math.min(currentState.maxCols || 26, Math.max(1, (currentState.cols || 10) + dc));
  resizeTo(nextR, nextC);
}
function runCliInput(e) {
  e.preventDefault();
  const input = document.getElementById('cliCommand');
  const cmd = input.value.trim();
  if (!cmd) return;
  input.value = '';
  sendCommand(cmd).catch(() => {});
}
function extractPrecedents(formula) {
  if (!formula || !formula.startsWith('=')) return [];
  return [...new Set(formula.toUpperCase().match(/[A-Z]([1-9]|[1-2][0-9]|30)/g) || [])];
}
function extractDependents(cell) {
  const raw = (currentState.deps && currentState.deps[cell]) || '';
  return raw.split('->').map(s => s.trim()).filter(s => s && s !== cell && s !== 'NULL');
}
function buildShuntingYardPreview(formula) {
  if (!formula || !formula.startsWith('=')) return null;
  const rawTokens = formula.slice(1).trim().match(/[A-Za-z][0-9]*|\d*\.?\d+|[+\-*/()]/g);
  if (!rawTokens) return null;
  const prec = { '~': 3, '*': 2, '/': 2, '+': 1, '-': 1 };
  const ops = [], postfix = [];
  let needValue = true;
  for (const tok of rawTokens) {
    if (tok === '(') ops.push(tok);
    else if (tok === ')') {
      while (ops.length && ops[ops.length - 1] !== '(') postfix.push(ops.pop());
      if (ops.length) ops.pop();
    } else if (['+', '-', '*', '/'].includes(tok)) {
      if (needValue && tok === '-') ops.push('~');
      else {
        while (ops.length && ops[ops.length - 1] !== '(' && prec[ops[ops.length - 1]] >= prec[tok]) postfix.push(ops.pop());
        ops.push(tok); needValue = true;
      }
    } else { postfix.push(tok.toUpperCase()); needValue = false; }
  }
  while (ops.length) postfix.push(ops.pop());
  return postfix;
}
function updateInspector() {
  const { row, col } = parseCellCoords(selectedCell);
  if (row < 0 || row >= currentState.rows || col < 0 || col >= currentState.cols) selectedCell = 'A1';
  document.getElementById('activeCellBadge').textContent = selectedCell;
  document.getElementById('inspectTarget').textContent = 'Inspecting: ' + selectedCell;
  const coords = parseCellCoords(selectedCell);
  const cellData = (currentState.grid[coords.row] && currentState.grid[coords.row][coords.col]) || { content: '', display: '', isFormula: false, error: '' };
  const fInput = document.getElementById('formulaInput');
  if (document.activeElement !== fInput) fInput.value = cellData.content || '';
  document.getElementById('depChainBox').textContent = (currentState.deps && currentState.deps[selectedCell]) || (selectedCell + ' -> NULL');
  const precedents = extractPrecedents(cellData.content);
  const dependents = extractDependents(selectedCell);
  document.getElementById('refSummary').textContent =
    `Upstream inputs: ${precedents.length ? precedents.join(', ') : 'None'}  •  Downstream dependents: ${dependents.length ? dependents.join(', ') : 'None'}`;
  const rpnBox = document.getElementById('rpnBox');
  const rpnTokens = document.getElementById('rpnTokens');
  rpnTokens.replaceChildren();
  if (cellData.isFormula) {
    const postfix = buildShuntingYardPreview(cellData.content);
    if (cellData.error) rpnBox.textContent = 'Evaluator Error: ' + cellData.error;
    else if (postfix) {
      rpnBox.textContent = 'Postfix (RPN) Queue: ' + postfix.join(' ');
      postfix.forEach(t => {
        const span = document.createElement('span');
        span.className = 'tok' + (['+','-','*','/','~'].includes(t) ? ' op' : '');
        span.textContent = t;
        rpnTokens.appendChild(span);
      });
    }
  } else {
    rpnBox.textContent = cellData.content
      ? `Literal constant (${cellData.content}) — bypasses postfix stack`
      : 'Empty cell — select a formula cell (=...) to inspect postfix stack';
  }
  document.getElementById('historyBox').textContent = (currentState.history && currentState.history.trim()) || 'History is empty.';
  document.querySelectorAll('.cell-td').forEach(td => {
    const id = td.dataset.cell;
    td.classList.toggle('is-selected', id === selectedCell);
    td.classList.toggle('is-precedent', precedents.includes(id));
    td.classList.toggle('is-dependent', dependents.includes(id));
  });
}
function selectCell(cell) { selectedCell = cell; updateInspector(); }
function applyState(state) {
  currentState = state;
  lastStateSnapshot = JSON.stringify(state);
  document.getElementById('gridSizePill').textContent = `Grid: ${state.rows}×${state.cols} (Max ${state.maxRows}×${state.maxCols})`;
  document.getElementById('matrixTitle').textContent = `Spreadsheet Matrix (${state.rows} × ${state.cols})`;
  renderGrid(state.grid || []);
  updateInspector();
}
function renderGrid(grid) {
  const table = document.getElementById('sheet');
  table.replaceChildren();
  const header = document.createElement('tr');
  header.appendChild(document.createElement('th'));
  const columns = grid.length ? grid[0].length : 0;
  for (let col = 0; col < columns; col++) {
    const th = document.createElement('th');
    th.textContent = colToLetter(col);
    header.appendChild(th);
  }
  table.appendChild(header);
  for (let row = 0; row < grid.length; row++) {
    const tr = document.createElement('tr');
    const rowHead = document.createElement('td');
    rowHead.className = 'rowhead';
    rowHead.textContent = row + 1;
    tr.appendChild(rowHead);
    for (let col = 0; col < grid[row].length; col++) {
      const cell = colToLetter(col) + (row + 1);
      const data = grid[row][col];
      const content = data.content || '', display = data.display || '';
      const td = document.createElement('td');
      td.className = 'cell-td';
      td.dataset.cell = cell;
      if (data.isFormula) td.classList.add('is-formula');
      if (display === 'ERR') td.classList.add('is-error');
      if (data.isFormula) {
        const badge = document.createElement('span');
        badge.className = 'cell-tag';
        badge.textContent = display === 'ERR' ? 'ERR' : 'fx';
        td.appendChild(badge);
      }
      const editor = document.createElement('input');
      editor.className = 'cell-input';
    
    <div class="sheet-wrap"><table class="grid-table" id="sheet"></table></div>
  </section>
    editor.type = 'text';
      editor.value = display;
      editor.autocomplete = 'off';
      editor.spellcheck = false;
      editor.setAttribute('aria-label', cell);
      let finished = false;
      editor.addEventListener('focus', () => { finished = false; selectCell(cell); editor.value = content; });
      async function finishEdit(cancel) {
        if (finished) return;
        finished = true;
        const newContent = editor.value.trim();
        editor.value = display;
        if (cancel || newContent === content) return;
        const command = newContent === '' ? 'CLEAR ' + cell : 'SET ' + cell + ' ' + newContent;
        try { await sendCommand(command); } catch (e) {}
      }
      editor.addEventListener('keydown', (e) => {
        if (e.isComposing) return;
        if (e.key === 'Enter') { e.preventDefault(); finishEdit(false); editor.blur(); }
        else if (e.key === 'Escape') { e.preventDefault(); finishEdit(true); editor.blur(); }
      });
      editor.addEventListener('blur', () => finishEdit(false));
      td.appendChild(editor);
      tr.appendChild(td);
    }
    table.appendChild(tr);
  }
}
document.getElementById('formulaInput').addEventListener('keydown', (e) => {
  if (e.key === 'Enter') {
    e.preventDefault();
    const val = e.target.value.trim();
    const cmd = val === '' ? 'CLEAR ' + selectedCell : 'SET ' + selectedCell + ' ' + val;
    e.target.blur();
    sendCommand(cmd).catch(() => {});
  }
});
async function runBatch(commands, finalMessage) {
  for (const cmd of commands) await sendCommand(cmd, true);
  appendLog(finalMessage);
}
async function loadGroupLabDemo() {
  await sendCommand('RESIZE 12 8', true);
  await runBatch([
    'SET A1 18', 'SET B1 20', 'SET C1 17', 'SET D1 =A1+B1+C1', 'SET E1 =D1/3',
    'SET A2 15', 'SET B2 16', 'SET C2 19', 'SET D2 =A2+B2+C2', 'SET E2 =D2/3',
    'SET A3 20', 'SET B3 18', 'SET C3 18', 'SET D3 =A3+B3+C3', 'SET E3 =D3/3',
    'SET A4 =A1+A2+A3', 'SET B4 =B1+B2+B3', 'SET C4 =C1+C2+C3', 'SET D4 =D1+D2+D3'
  ], 'Loaded 12×8 Group Lab Demo.');
  selectCell('D4');
}
async function loadCascadeDemo() {
  await runBatch(['SET A1 10', 'SET B1 =A1*2', 'SET C1 =B1+5', 'SET D1 =C1*3', 'SET E1 =D1-B1'], 'Loaded Topological Cascade Demo.');
  selectCell('A1');
}
async function testCycleDemo() {
  await sendCommand('SET A1 =B1+1', true);
  await sendCommand('SET B1 =A1+1', false);
  selectCell('B1');
}
function isEditingCell() {
  const active = document.activeElement;
  return active && (active.classList.contains('cell-input') || active.id === 'formulaInput');
}
async function refreshSharedGrid() {
  if (busy || polling || isEditingCell()) return;
  polling = true;
  const generation = commandGeneration;
  const controller = new AbortController();
  const timer = setTimeout(() => controller.abort(), 8000);
  try {
    const response = await fetch('/api/grid', { cache: 'no-store', signal: controller.signal });
    if (!response.ok) throw new Error('Refresh failed');
    const data = await response.json();
    if (busy || isEditingCell() || generation !== commandGeneration) return;
    const snapshot = JSON.stringify(data);
    if (snapshot !== lastStateSnapshot) applyState(data);
  } catch (err) { console.warn('Auto-sync poll skipped:', err); }
  finally { clearTimeout(timer); polling = false; }
}
quick('VIEW');
setInterval(refreshSharedGrid, 2000);
</script>
</body>
</html>
)HTML";

void handleClient(SOCKET clientSocket) {
    DWORD timeout = 5000;
    setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char*>(&timeout), sizeof(timeout));
    setsockopt(clientSocket, SOL_SOCKET, SO_SNDTIMEO, reinterpret_cast<const char*>(&timeout), sizeof(timeout));
    string request;
    if (!readRequest(clientSocket, request)) { closesocket(clientSocket); return; }
    istringstream firstLine(request);
    string method, path;
    firstLine >> method >> path;
    string response;
    if (method == "POST" && path == "/api/command") {
        size_t bodyStart = request.find("\r\n\r\n");
        string output = runCommand(request.substr(bodyStart + 4));
        string state = stateJSON();
        response = httpResponse("{\"output\":" + jsonText(output) + "," + state.substr(1), "application/json; charset=utf-8");
    } else if (method == "GET" && path == "/api/grid") {
        response = httpResponse(stateJSON(), "application/json; charset=utf-8");
    } else if (method == "GET" && path == "/") {
        response = httpResponse(indexHTML, "text/html; charset=utf-8");
    } else if (method == "GET" && path == "/favicon.ico") {
        response = httpResponse("", "image/x-icon");
    } else {
        response = httpResponse("Not found.", "text/plain; charset=utf-8", "404 Not Found");
    }
    sendAll(clientSocket, response);
    shutdown(clientSocket, SD_SEND);
    closesocket(clientSocket);
}

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) { cout << "WSAStartup failed." << endl; return 1; }
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket == INVALID_SOCKET) { cout << "Could not create socket. Error: " << WSAGetLastError() << endl; WSACleanup(); return 1; }
    sockaddr_in address = {};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(8080);
    if (bind(serverSocket, reinterpret_cast<sockaddr*>(&address), sizeof(address)) == SOCKET_ERROR) {
        cout << "Could not start server on port 8080. Error: " << WSAGetLastError() << endl;
        closesocket(serverSocket); WSACleanup(); return 1;
    }
    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        cout << "Could not listen. Error: " << WSAGetLastError() << endl;
        closesocket(serverSocket); WSACleanup(); return 1;
    }
    cout << "MathSync web frontend running.\nOpen http://localhost:8080 in your browser.\nPress Ctrl+C to stop." << endl;
    while (true) {
        SOCKET clientSocket = accept(serverSocket, nullptr, nullptr);
        if (clientSocket == INVALID_SOCKET) { cout << "Accept failed. Error: " << WSAGetLastError() << endl; break; }
        handleClient(clientSocket);
    }
    closesocket(serverSocket);
    WSACleanup();
    return 0;
}
