# Package littleSynth Windows build into a ZIP
$ErrorActionPreference = "Stop"

$version = if ($env:GITHUB_REF_NAME) { $env:GITHUB_REF_NAME -replace '^v','' } else { "1.0.0" }
$dist = "dist"
$staging = "$dist\littleSynth-Windows"

Remove-Item -Recurse -Force $dist -ErrorAction SilentlyContinue
New-Item -ItemType Directory -Path $staging -Force | Out-Null

# Copy VST3 plugin
$vst3Src = "build\littleSynth_artefacts\Release\VST3\littleSynth.vst3"
if (Test-Path $vst3Src) {
    Copy-Item -Recurse $vst3Src $staging
    Write-Host "Copied VST3"
} else {
    Write-Error "VST3 not found at $vst3Src"
    exit 1
}

# Copy Standalone exe
$exeSrc = Get-ChildItem -Recurse -Filter "littleSynth.exe" |
    Select-Object -First 1 -ExpandProperty FullName
if ($exeSrc) {
    $standaloneDir = "$staging\Standalone"
    New-Item -ItemType Directory -Path $standaloneDir -Force | Out-Null
    Copy-Item $exeSrc $standaloneDir
    Write-Host "Copied Standalone"
} else {
    Write-Error "Standalone .exe not found"
    exit 1
}

# Add README
$readme = @"
littleSynth v$version

Contents:
  littleSynth.vst3  - VST3 plugin (copy to C:\Program Files\Common Files\VST3\)
  Standalone\        - Standalone application

https://github.com/andypoots/littleSynth
"@
Set-Content -Path "$staging\README.txt" -Value $readme

# Create ZIP
$zipPath = "$dist\littleSynth-Windows.zip"
Compress-Archive -Path "$staging\*" -DestinationPath $zipPath -Force
Write-Host "Created $zipPath"
