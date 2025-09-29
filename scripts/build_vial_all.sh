#!/usr/bin/env bash
#
# Vial 用ビルド成果物を build/ に集約するスクリプト

# sh 等から呼ばれた場合でも bash で再実行する
if [ -z "${BASH_VERSION:-}" ]; then
  exec bash "$0" "$@"
fi

set -Eeuo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
VIAL_DIR="$REPO_ROOT/vial-qmk"
BUILD_DIR="$REPO_ROOT/build"

keyboards=("keyball39" "keyball44" "keyball61")
keymaps=("toxaO" "user_right" "user_left" "user_dual")

mkdir -p "$BUILD_DIR"

for kb in "${keyboards[@]}"; do
  for km in "${keymaps[@]}"; do
    target="keyball/${kb}:${km}"
    echo "[BUILD] $target"
    make -C "$VIAL_DIR" SKIP_GIT=yes VIAL_ENABLE=yes "$target"

    artifact="keyball_${kb}_${km}.uf2"
    candidates=(
      "$VIAL_DIR/.build/$artifact"
      "$VIAL_DIR/$artifact"
    )

    copied=false
    for src in "${candidates[@]}"; do
      if [ -f "$src" ]; then
        cp "$src" "$BUILD_DIR/$artifact"
        echo "  -> $BUILD_DIR/$artifact"
        copied=true
        break
      fi
    done

    if [ "$copied" = false ]; then
      echo "[ERROR] $target の .uf2 が見つかりません" >&2
      exit 1
    fi
  done

done
