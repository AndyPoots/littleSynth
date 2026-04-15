@echo off
REM littleSynth Windows Build Script
REM Requires: CMake 3.22+, Visual Studio 2019+ with C++ desktop workload
REM Run from the repo root directory

echo === littleSynth Windows Build ===

if not exist ThirdParty\JUCE (
    git clone https://github.com/juce-framework/JUCE.git ThirdParty\JUCE
)
if not exist ThirdParty\DaisySP (
    git clone https://github.com/electro-smith/DaisySP.git ThirdParty\DaisySP
)

cmake -B build -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release

mkdir Releases\Windows 2>nul
copy /Y build\littleSynth_artefacts\Release\VST3\littleSynth.vst3\Contents\x86_64-win\littleSynth.vst3 Releases\Windows\littleSynth.vst3
copy /Y build\littleSynth_artefacts\Release\Standalone\littleSynth.exe Releases\Windows\littleSynth.exe

echo === Build complete. Artefacts in Releases\Windows\ ===
pause
