# PowerShell Build Script for Truth Table Generator WebAssembly Module

$ErrorActionPreference = "Stop"

Write-Host "==========================================================" -ForegroundColor Cyan
Write-Host " Building Truth Table Generator WebAssembly Engine        " -ForegroundColor Cyan
Write-Host "==========================================================" -ForegroundColor Cyan

# Locate emcc compiler
$emccPath = Get-Command emcc -ErrorAction SilentlyContinue | Select-Object -ExpandProperty Path
if (-not $emccPath) {
    if (Test-Path "C:\Users\emman\emsdk\upstream\emscripten\emcc.exe") {
        $emccPath = "C:\Users\emman\emsdk\upstream\emscripten\emcc.exe"
    } elseif ($env:EMSDK -and (Test-Path "$env:EMSDK\upstream\emscripten\emcc.exe")) {
        $emccPath = "$env:EMSDK\upstream\emscripten\emcc.exe"
    } else {
        Write-Error "Emscripten (emcc) compiler not found. Please activate emsdk before running build.ps1."
        exit 1
    }
}

Write-Host "Using compiler: $emccPath" -ForegroundColor Yellow

# Ensure target output directory exists
if (-not (Test-Path "web")) {
    New-Item -ItemType Directory -Path "web" | Out-Null
}

# Compile C logic into WebAssembly
$compileCmd = "& '$emccPath' src/logic.c -O3 -s WASM=1 -s EXPORTED_FUNCTIONS=`"['_evaluate_logic', '_free_result', '_malloc', '_free']`" -s EXPORTED_RUNTIME_METHODS=`"['cwrap', 'UTF8ToString', 'stringToUTF8']`" -o web/truth_table_wasm.js"

Write-Host "Running: $compileCmd" -ForegroundColor Gray
Invoke-Expression $compileCmd

if ((Test-Path "web/truth_table_wasm.js") -and (Test-Path "web/truth_table_wasm.wasm")) {
    $jsSize = (Get-Item "web/truth_table_wasm.js").Length / 1KB
    $wasmSize = (Get-Item "web/truth_table_wasm.wasm").Length / 1KB
    Write-Host "`nSUCCESS! Build complete." -ForegroundColor Green
    Write-Host "  - web/truth_table_wasm.js   ($([math]::Round($jsSize, 2)) KB)" -ForegroundColor White
    Write-Host "  - web/truth_table_wasm.wasm ($([math]::Round($wasmSize, 2)) KB)" -ForegroundColor White
    Write-Host "`nTo serve locally:" -ForegroundColor Yellow
    Write-Host "  python -m http.server 8080 -d web" -ForegroundColor White
    Write-Host "  or: npx http-server web`n" -ForegroundColor White
} else {
    Write-Error "Build failed. Generated files not found."
}
