# build_all.ps1

$BuildDir = "build"
$Config = "Release"

# 1. ビルドディレクトリの作成（なければ）
if (!(Test-Path $BuildDir)) {
    New-Item -ItemType Directory -Force -Path $BuildDir
}

# 2. CMake 構成
Write-Host "--- Configuring CMake ---" -ForegroundColor Cyan
cmake -B $BuildDir

if ($LASTEXITCODE -ne 0) {
    Write-Host "CMake configuration failed!" -ForegroundColor Red
    exit $LASTEXITCODE
}

# 3. ビルド実行
Write-Host "--- Building Project ($Config) ---" -ForegroundColor Cyan
cmake --build $BuildDir --config $Config

if ($LASTEXITCODE -ne 0) {
    Write-Host "Build failed!" -ForegroundColor Red
    exit $LASTEXITCODE
}

# 4. 実行ファイルの場所を表示
$ExePath = Join-Path $BuildDir $Config "ExcelerImGui.exe"
if (Test-Path $ExePath) {
    Write-Host "`nBuild Successful!" -ForegroundColor Green
    Write-Host "Executable: $ExePath"
    
    Write-Host "`nOpen the file...`n"
    ./build/Release/ExcelerImGui.exe
} else {
    Write-Host "`nExecutable not found at $ExePath" -ForegroundColor Yellow
}