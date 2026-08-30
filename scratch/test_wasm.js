const Module = require('../web/truth_table_wasm.js');

Module.onRuntimeInitialized = () => {
    const evaluate_logic = Module.cwrap('evaluate_logic', 'string', ['string']);
    const free_result = Module.cwrap('free_result', null, ['number']);

    const testExpressions = [
        "P AND Q",
        "P OR Q",
        "NOT P",
        "P -> Q",
        "P <-> Q",
        "(P AND Q) -> R",
        "NOT(P OR Q) AND R",
        "(P <-> Q) AND (R -> S)",
        "P OR NOT P",
        "P AND NOT P",
        "(P AND Q" // invalid test
    ];

    console.log("=== RUNNING WASM ENGINE VERIFICATION TESTS ===");
    for (const expr of testExpressions) {
        const jsonString = evaluate_logic(expr);
        const res = JSON.parse(jsonString);
        console.log(`Expression: "${expr}" => Success: ${res.success}`);
        if (res.success) {
            console.log(`  Vars (${res.varCount}): [${res.variables.join(', ')}], Rows: ${res.rowCount}, Classification: ${res.classification}`);
            console.log(`  Row 0 result: ${res.rows[0].result}`);
        } else {
            console.log(`  Expected Error: ${res.error}`);
        }
    }
};
