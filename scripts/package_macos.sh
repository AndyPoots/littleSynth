#!/usr/bin/env bash
# Package littleSynth macOS build into a DMG
set -euo pipefail

VERSION="${GITHUB_REF_NAME:-1.0.0}"
VERSION="${VERSION#v}"
DIST="dist"
DMG_NAME="littleSynth-macOS"
STAGING="$DIST/$DMG_NAME"

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

# Copy Standalone app
APP_SRC="build/littleSynth_artefacts/Release/Standalone/littleSynth.app"
if [ -d "$APP_SRC" ]; then
    cp -R "$APP_SRC" "$STAGING/"
    echo "Copied Standalone"
else
    echo "ERROR: Standalone not found at $APP_SRC"
    exit 1
fi

# Add a README
cat > "$STAGING/README.txt" <<EOF
littleSynth v${VERSION}

Contents:
  littleSynth.vst3  - VST3 plugin (copy to ~/Library/Audio/Plug-Ins/VST3/)
  littleSynth.app   - Standalone application

https://github.com/andypoots/littleSynth
EOF

# Create DMG
DMG_PATH="$DIST/$DMG_NAME.dmg"
hdiutil create -volname "littleSynth" \
    -srcfolder "$STAGING" \
    -ov -format UDZO \
    "$DMG_PATH"

echo "Created $DMG_PATH"
