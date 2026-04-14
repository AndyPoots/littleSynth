#!/usr/bin/env bash
# Package littleSynth Linux build into a tarball
set -euo pipefail

VERSION="${GITHUB_REF_NAME:-1.0.0}"
VERSION="${VERSION#v}"
DIST="dist"
STAGING="$DIST/littleSynth-Linux"

rm -rf "$DIST"
mkdir -p "$STAGING"

# Copy VST3 plugin
VST3_SRC="build/littleSynth_artefacts/Release/VST3/littleSynth.vst3"
if [ -d "$VST3_SRC" ]; then
    cp -R "$VST3_SRC" "$STAGING/"
    echo "Copied VST3"
else
    echo "ERROR: VST3 not found at $VST3_SRC"
    exit 1
fi

# Copy Standalone binary
STANDALONE_SRC="build/littleSynth_artefacts/Release/Standalone/littleSynth"
if [ -f "$STANDALONE_SRC" ]; then
    mkdir -p "$STAGING/Standalone"
    cp "$STANDALONE_SRC" "$STAGING/Standalone/"
    chmod +x "$STAGING/Standalone/littleSynth"
    echo "Copied Standalone"
else
    echo "ERROR: Standalone not found at $STANDALONE_SRC"
    exit 1
fi

# Add README
cat > "$STAGING/README.txt" <<EOF
littleSynth v${VERSION}

Contents:
  littleSynth.vst3  - VST3 plugin (copy to ~/.vst3/ or /usr/lib/vst3/)
  Standalone/        - Standalone application (run ./littleSynth)

Install VST3:
  mkdir -p ~/.vst3
  cp -R littleSynth.vst3 ~/.vst3/

https://github.com/andypoots/littleSynth
EOF

# Create tarball
TAR_PATH="$DIST/littleSynth-Linux.tar.gz"
tar -czf "$TAR_PATH" -C "$DIST" "littleSynth-Linux"

echo "Created $TAR_PATH"
