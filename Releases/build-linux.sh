#!/bin/bash
# littleSynth Linux Build Script
# Requires: CMake 3.22+, GCC 9+, ALSA dev headers, FreeType dev headers
# Run from the repo root directory

set -e

echo "=== littleSynth Linux Build ==="

# Install dependencies (Debian/Ubuntu)
# sudo apt install cmake build-essential libasound2-dev libfreetype6-dev libx11-dev libxrandr-dev libgl1-mesa-dev

if [ ! -d "ThirdParty/JUCE" ]; then
    git clone https://github.com/juce-framework/JUCE.git ThirdParty/JUCE
fi
if [ ! -d "ThirdParty/DaisySP" ]; then
    git clone https://github.com/electro-smith/DaisySP.git ThirdParty/DaisySP
fi

cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)

mkdir -p Releases/Linux

# Copy VST3
cp -r build/littleSynth_artefacts/Release/VST3/littleSynth.vst3 Releases/Linux/

# Copy Standalone
cp build/littleSynth_artefacts/Release/Standalone/littleSynth Releases/Linux/littleSynth

# --- Build AppImage ---
echo "=== Building AppImage ==="

APPDIR="Releases/Linux/littleSynth.AppDir"
mkdir -p "$APPDIR/usr/bin"
mkdir -p "$APPDIR/usr/lib"
mkdir -p "$APPDIR/DESKTOP"
mkdir -p "$APPDIR/usr/share/applications"

cp build/littleSynth_artefacts/Release/Standalone/littleSynth "$APPDIR/usr/bin/"

cat > "$APPDIR/littleSynth.desktop" << 'DESKTOP'
[Desktop Entry]
Name=littleSynth
Exec=littleSynth
Icon=littleSynth
Type=Application
Categories=AudioVideo;Audio;
DESKTOP

cp "$APPDIR/littleSynth.desktop" "$APPDIR/usr/share/applications/"

# AppRun
cat > "$APPDIR/AppRun" << 'APPRUN'
#!/bin/bash
SELF=$(readlink -f "$0")
HERE=${SELF%/*}
export PATH="${HERE}/usr/bin:${PATH}"
export LD_LIBRARY_PATH="${HERE}/usr/lib:${LD_LIBRARY_PATH}"
exec "${HERE}/usr/bin/littleSynth" "$@"
APPRUN
chmod +x "$APPDIR/AppRun"

if command -v appimagetool &>/dev/null; then
    appimagetool "$APPDIR" Releases/Linux/littleSynth-x86_64.AppImage
    echo "AppImage created: Releases/Linux/littleSynth-x86_64.AppImage"
else
    echo "appimagetool not found. Install it from https://github.com/AppImage/appimagetool"
    echo "Then run: appimagetool $APPDIR Releases/Linux/littleSynth-x86_64.AppImage"
    # Archive the AppDir as a fallback
    cd Releases/Linux && tar czf littleSynth-linux-x86_64.tar.gz littleSynth.AppDir && cd ../..
fi

# --- Build Flatpak ---
echo "=== Building Flatpak ==="

FLATDIR="Releases/Linux/flatpak-build"
mkdir -p "$FLATDIR"

cat > "$FLATDIR/com.littlesynth.littleSynth.json" << 'FLATMANIFEST'
{
    "app-id": "com.littlesynth.littleSynth",
    "runtime": "org.freedesktop.Platform",
    "runtime-version": "23.08",
    "sdk": "org.freedesktop.Sdk",
    "command": "littleSynth",
    "finish-args": [
        "--share=ipc",
        "--socket=x11",
        "--socket=pulseaudio",
        "--device=dri"
    ],
    "modules": [
        {
            "name": "littleSynth",
            "buildsystem": "cmake",
            "config-opts": ["-DCMAKE_BUILD_TYPE=Release"],
            "sources": [
                {
                    "type": "dir",
                    "path": "../../.."
                }
            ]
        }
    ]
}
FLATMANIFEST

echo "Flatpak manifest created at $FLATDIR/com.littlesynth.littleSynth.json"
echo ""
echo "To build the Flatpak, run:"
echo "  cd $FLATDIR"
echo "  flatpak-builder --force-clean build-dir com.littlesynth.littleSynth.json"
echo "  flatpak-builder --repo=repo --force-clean build-dir com.littlesynth.littleSynth.json"
echo "  flatpak build-bundle repo ../../Releases/Linux/littleSynth.flatpak com.littlesynth.littleSynth"

echo ""
echo "=== Build complete. Artefacts in Releases/Linux/ ==="
