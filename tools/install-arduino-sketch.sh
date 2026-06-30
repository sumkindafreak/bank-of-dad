#!/usr/bin/env bash
# Install Bank of Dad LVGL sketch to Arduino sketchbook (Linux/macOS)
set -euo pipefail
REPO_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
SKETCH_DIR="${1:-$HOME/Arduino/BankOfDadLVGL}"
LIBS_DIR="${HOME}/Arduino/libraries"

mkdir -p "$SKETCH_DIR"
cp -a "$REPO_ROOT/BankOfDadLVGL/." "$SKETCH_DIR/"
mkdir -p "$LIBS_DIR"
cp "$REPO_ROOT/arduino-libraries/lv_conf.h" "$LIBS_DIR/lv_conf.h"

echo "Installed to $SKETCH_DIR"
echo "lv_conf.h -> $LIBS_DIR/lv_conf.h"
grep -q "TOUCH_ROT_CW_90" "$SKETCH_DIR/touch_lvgl.h" && echo "[OK] touch_lvgl.h"
grep -q "points\[0\]" "$SKETCH_DIR/touch_lvgl.cpp" && echo "[OK] touch_lvgl.cpp"
