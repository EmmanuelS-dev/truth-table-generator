# Propositional Logic Truth Table Generator

A dynamic propositional logic expression evaluator and truth table generator built with **C** and **WebAssembly**.

![Static Web App](https://img.shields.io/badge/Architecture-C%20%2B%20WebAssembly-indigo)
![Deployment](https://img.shields.io/badge/Deployment-Static%20Client--Side-emerald)
![Dependencies](https://img.shields.io/badge/Dependencies-Zero%20Frameworks-blue)

---

## 🌟 Features

- **C WebAssembly Computational Engine**: The core logical parser and evaluator is written 100% in pure C and compiled to WebAssembly for near-native in-browser computational performance.
- **Dynamic Proposition Variable Detection**: Automatically scans expressions for single uppercase proposition variables (`P`, `Q`, `R`, `S`, etc.).
- **Algorithmic Truth Assignment**: Computes total rows algorithmically as $2^n$ ($n$ = count of distinct variables) and generates every binary valuation row using bitwise logic.
- **Formal Precedence Parsing**: Enforces strict formal logic operator precedence with parentheses support `(` and `)`.
- **Expression Classification**: Classifies every expression as:
  - 🟢 **TAUTOLOGY** (all truth assignments evaluate to true / 1)
  - 🔴 **CONTRADICTION** (all truth assignments evaluate to false / 0)
  - 🟡 **CONTINGENCY** (mixed true and false evaluation results)
- **Zero Backend Required**: Fully static client-side web application—deployable on GitHub Pages, Netlify, or Vercel without servers or databases.

---

## 📐 Supported Operators & Formal Precedence

Order of evaluation from highest precedence to lowest:

| Precedence | Operator Type | Syntax / Keywords | Associativity | Example |
| :---: | :--- | :--- | :---: | :--- |
| **1** (Highest) | Negation (NOT) | `NOT`, `!`, `~` | Right | `NOT P`, `!Q` |
| **2** | Conjunction (AND) | `AND`, `&`, `&&` | Left | `P AND Q`, `P & Q` |
| **3** | Disjunction (OR) | `OR`, `\|`, `\|\|` | Left | `P OR Q`, `P \| Q` |
| **4** | Implication (Conditional) | `->`, `=>` | Right | `P -> Q` |
| **5** (Lowest) | Biconditional (Equivalence) | `<->`, `<=>` | Left | `P <-> Q` |

Parentheses `(` and `)` override standard precedence rules.

---

## 🏗 Architecture

```
                                 User Interface
                           (HTML / CSS / JavaScript)
                                      │
                         User enters expression string
                                      │
                                      ▼
                           WebAssembly JS Bridge
                          (web/truth_table_wasm.js)
                                      │
                       cwrap('evaluate_logic', ...)
                                      │
                                      ▼
                        C Computational Engine
                             (src/logic.c)
                                      │
               1. Tokenizes & detects variables
               2. Generates 2^n truth assignments
               3. Recursive Descent RPN parsing
               4. Classifies: TAUTOLOGY / CONTRADICTION / CONTINGENCY
               5. Formats JSON result string
                                      │
                                      ▼
                      JSON Returned to JavaScript
                        (Safe memory free via free_result)
                                      │
                                      ▼
                        HTML Truth Table Rendered
```

---

## 📁 Project Structure

```
truth-table-generator/
│
├── src/
│   └── logic.c              # Core C logical engine & Wasm export API
│
├── web/
│   ├── index.html           # Main HTML5 UI page
│   ├── style.css            # Dark glassmorphic design system
│   ├── script.js            # WebAssembly bridge & DOM handler
│   ├── truth_table_wasm.js  # Generated Emscripten JS wrapper
│   └── truth_table_wasm.wasm# Compiled WebAssembly binary
│
├── build.ps1                # PowerShell build script for WebAssembly
├── truth_table.c            # Original CLI C program reference
└── README.md                # Documentation
```

---

## 🛠 Building from Source

### Prerequisites

1. Install [Emscripten SDK (emsdk)](https://emscripten.org/docs/getting_started/downloads.html).
2. Activate emsdk environment:
   ```bash
   emsdk install latest
   emsdk activate latest
   ```

### Compile Command

Run the automated PowerShell build script:
```powershell
.\build.ps1
```

Or run `emcc` manually:
```bash
emcc src/logic.c -O3 -s WASM=1 \
  -s EXPORTED_FUNCTIONS="['_evaluate_logic', '_free_result', '_malloc', '_free']" \
  -s EXPORTED_RUNTIME_METHODS="['cwrap', 'UTF8ToString', 'stringToUTF8']" \
  -o web/truth_table_wasm.js
```

---

## 🚀 Running Locally

To run the application locally, serve the `web/` folder using any static HTTP web server:

Using Python:
```bash
python -m http.server 8080 -d web
```

Using Node.js:
```bash
npx http-server web
```

Open `http://localhost:8080` in your web browser.

---

## 🌐 Static Deployment

Since the compiled application requires no server-side logic, you can deploy the contents of the `web/` directory directly to any static web host:

- **GitHub Pages**: Set source branch to `main` and root directory to `/web` (or deploy `web/` to `gh-pages` branch).
- **Vercel**: Set framework to *Other* and Root Directory to `web`.
- **Netlify**: Set Publish Directory to `web`.

---

## 💡 Example Expressions to Test

- `P AND Q` $\rightarrow$ Contingency (4 rows)
- `P OR NOT P` $\rightarrow$ Tautology (2 rows)
- `P AND NOT P` $\rightarrow$ Contradiction (2 rows)
- `(P AND Q) -> R` $\rightarrow$ Contingency (8 rows)
- `NOT(P OR Q) AND R` $\rightarrow$ Contingency (8 rows)
- `(P <-> Q) AND (R -> S)` $\rightarrow$ Contingency (16 rows)
