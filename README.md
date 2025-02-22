# PLShell - Interactive Parsing Shell 🖥️📖

PLShell is an interactive shell for grammar analysis and parsing. It supports **FIRST, FOLLOW**, **LL(1) table construction**, **SLR(1) automaton generation**, and **input validation**.  
Designed for both **analysis and education**, it provides **step-by-step explanations** to help visualize parsing processes.

## 🚀 Features
✅ **Load grammars from files (`load grammar.txt`)**  
✅ **Compute LL(1) components**:
- `first`: Compute the First set of a given string.
- `follow`: Compute the Follow set of a given non-terminal.
- `predsymbols`: Compute the Prediction Symbols of a rule.
- `ll1`: Checks whether the grammar is LL(1) and display the table.
✅ **Coming soon: Generate SLR(1) automaton** and visualize states  
✅ **Coming soon: Parse and validate input strings**  
✅ **Verbose mode: Add `-v` or `--verbose` to any command to get a step-by-step explanation, useful for learning purposes.**
✅ **Shell environment**: Run multiple commands interactively.

---

## 📥 Compiling
### 🔧 Requirements
- **C++20 or later**  
- **Boost.ProgramOptions** (for command parsing)  

## Usage
Once inside `pl-shell` you can execute commands:
- Load a grammar
~~~
load my_grammar.txt
~~~
- Compute First set:
~~~
first ABC
first ABC -v
~~~
- Compute Follow set:
~~~
follow A
follow A -v
~~~
- Compute Prediction Symbols:
~~~
predsymbols A BC
predsymbols A BC -v
~~~
- Check if the grammar is LL(1):
~~~
ll1
ll1 -v
~~~
