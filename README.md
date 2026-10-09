**#Mathsync-DSA project**
This is a terminal-based, collaborative mini spreadsheet built using C++ for Data Structures, synchronized through shared-file sync (SYNC/LOAD) rather than live networking.
**#The Objectives behind our project:**

To create a near-real-time collaborative system that lets multiple users work together on the same shared file.
Build a computation engine that can parse, process and evaluate formulas.
To sync user inputs anytime someone makes changes in the system spreadsheet.
To get to know real applications of core Data Structures and Object-Oriented Programming topics.

**Team Members**
Anika Sharma,
Pushpendra Gupta,
Riddhi Gupta,
Akarshak Singh

**Technologies Used:**
C++ programming language, fstream, Git/GitHub

**System Architecture**
Terminal Interface — user enters commands
Command Parser — reads and interprets commands
Core Engine:
Grid Storage (Array)
Formula Evaluator (Stack)
Dependency Tracker (Linked List + Queue)
Recalculation & History (Queue, Stack, Linked List)
Shared File Storage (SYNC / LOAD)
Flow: User enters a command → Command parser interprets it and routes it to the core engine → Core engine processes grid, formula, or dependency logic → Recalculation engine updates dependent cells in correct order → Changes synced to team via SYNC/LOAD.

**Set-up:**
This is a terminal-based system, so the project code only needs to be compiled with the g++ compiler, e.g.:
g++ main.cpp -o mathsync
./mathsync
The main task is to commit changes to the repository as each module is developed.
Major Features/Parts:
Grid Storage for formulas
Formula Evaluator (using stack)
Dependency Tracker (using linked list and queue)
Edit History and Undo/Redo (using stack and linked list)
Current Status
Phase 2 (implementation) in progress.
Grid Storage: core structure implemented (SET/VIEW), minor fixes pending
Formula Evaluator: stack-based infix-to-postfix conversion and evaluation implemented standalone; integration with grid pending
Dependency Tracker: dependency list structure and cell-name parsing implemented; dependency registration and propagation logic in progress
History/Undo/Sync: data structures set up (undo/redo stacks); core logic not yet implemented
Not yet started: integration of all modules into a single codebase
