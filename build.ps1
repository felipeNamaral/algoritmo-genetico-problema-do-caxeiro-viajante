$ErrorActionPreference = 'Stop'
$compilerDirectory = 'C:\raylib\w64devkit\bin'
$raylibDirectory = 'C:\raylib\raylib\src'
$compiler = Join-Path $compilerDirectory 'g++.exe'
foreach ($required in @($compiler, "$raylibDirectory\raylib.h", "$raylibDirectory\libraylib.a")) {
    if (-not (Test-Path -LiteralPath $required)) {
        throw "Arquivo necessario nao encontrado: $required"
    }
}
$env:PATH = "$compilerDirectory;$env:PATH"
$outputDirectory = Join-Path $PSScriptRoot 'output'
New-Item -ItemType Directory -Path $outputDirectory -Force | Out-Null
$sources = @(Get-ChildItem -LiteralPath $PSScriptRoot -Filter '*.cpp' -File | ForEach-Object { $_.FullName })
if ($sources.Count -eq 0) { throw 'Nenhum arquivo .cpp encontrado na pasta do projeto.' }
& $compiler -std=c++17 -Wall -Wextra -g3 @sources -o "$outputDirectory\main.exe" "-I$raylibDirectory" "-L$raylibDirectory" -lraylib -lopengl32 -lgdi32 -lwinmm
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
Write-Host "Compilado: $outputDirectory\main.exe"
