/**
 * Truth Table Generator - WebAssembly & JavaScript Bridge
 */

let evaluate_logic_wasm = null;
let free_result_wasm = null;
let wasmReady = false;

// Initialize Emscripten WebAssembly Module
if (typeof Module !== 'undefined') {
    Module.onRuntimeInitialized = () => {
        try {
            evaluate_logic_wasm = Module.cwrap('evaluate_logic', 'string', ['string']);
            free_result_wasm = Module.cwrap('free_result', null, ['number']);
            wasmReady = true;
            console.log('WebAssembly Logic Engine initialized successfully.');
            
            // Auto-evaluate default expression on load
            evaluateExpression();
        } catch (err) {
            showError('Failed to wrap WebAssembly functions: ' + err.message);
        }
    };
} else {
    showError('WebAssembly JavaScript glue file (truth_table_wasm.js) not found.');
}

// DOM Elements
const form = document.getElementById('evaluator-form');
const input = document.getElementById('expression-input');
const errorBox = document.getElementById('error-box');
const errorMessage = document.getElementById('error-message');
const resultsContainer = document.getElementById('results-container');

const metricVars = document.getElementById('metric-vars');
const metricVarCount = document.getElementById('metric-var-count');
const metricRowCount = document.getElementById('metric-row-count');
const metricClassification = document.getElementById('metric-classification');

const tableHeadRow = document.getElementById('table-head-row');
const tableBody = document.getElementById('table-body');

// Event Listeners
form.addEventListener('submit', (e) => {
    e.preventDefault();
    evaluateExpression();
});

// Quick Example Chips
document.querySelectorAll('.chip').forEach(chip => {
    chip.addEventListener('click', () => {
        const expr = chip.getAttribute('data-expr');
        if (expr) {
            input.value = expr;
            evaluateExpression();
        }
    });
});

/**
 * Main Evaluation Routine
 */
function evaluateExpression() {
    const exprText = input.value.trim();
    
    if (!exprText) {
        showError('Please enter a logical expression.');
        return;
    }

    if (!wasmReady || !evaluate_logic_wasm) {
        showError('WebAssembly engine is still loading. Please wait a moment...');
        return;
    }

    try {
        // Execute C computational logic engine in WebAssembly
        const jsonStr = evaluate_logic_wasm(exprText);
        
        if (!jsonStr) {
            showError('Engine returned null result.');
            return;
        }

        const data = JSON.parse(jsonStr);

        if (!data.success) {
            showError(data.error || 'Syntax or parsing error in logical expression.');
            return;
        }

        // Render metrics and table
        renderResults(data);
    } catch (err) {
        showError('An unexpected error occurred during evaluation: ' + err.message);
    }
}

/**
 * Render Metrics and HTML Truth Table
 */
function renderResults(data) {
    // Hide error box
    errorBox.classList.add('hidden');

    // Update Summary Metrics
    metricVars.textContent = data.variables.join(', ');
    metricVarCount.textContent = data.varCount;
    metricRowCount.textContent = Number(data.rowCount).toLocaleString();

    // Update Classification Badge
    metricClassification.textContent = data.classification;
    metricClassification.className = 'badge-classification';
    if (data.classification === 'TAUTOLOGY') {
        metricClassification.classList.add('badge-tautology');
    } else if (data.classification === 'CONTRADICTION') {
        metricClassification.classList.add('badge-contradiction');
    } else {
        metricClassification.classList.add('badge-contingency');
    }

    // Build Table Header
    let headHtml = '';
    data.variables.forEach(v => {
        headHtml += `<th>${v}</th>`;
    });
    headHtml += `<th class="result-header">RESULT</th>`;
    tableHeadRow.innerHTML = headHtml;

    // Build Table Rows
    let bodyHtml = '';
    data.rows.forEach(row => {
        bodyHtml += '<tr>';
        row.values.forEach(val => {
            const pillClass = val === 1 ? 'val-true' : 'val-false';
            bodyHtml += `<td><span class="${pillClass}">${val}</span></td>`;
        });
        const resPillClass = row.result === 1 ? 'val-true' : 'val-false';
        bodyHtml += `<td class="result-cell"><span class="${resPillClass}">${row.result}</span></td>`;
        bodyHtml += '</tr>';
    });
    tableBody.innerHTML = bodyHtml;

    // Display Results Container
    resultsContainer.classList.remove('hidden');
}

/**
 * Display Error Message
 */
function showError(msg) {
    errorMessage.textContent = msg;
    errorBox.classList.remove('hidden');
    resultsContainer.classList.add('hidden');
}
