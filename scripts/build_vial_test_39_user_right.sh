#!/usr/bin/env bash
#
# keyball39:user_right のテストビルドを行い、成果物を build/ に配置するスクリプト

if [ -z "${BASH_VERSION:-}" ]; then
  exec bash "$0" "$@"
fi

set -Eeuo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
VIAL_DIR="$REPO_ROOT/vial-qmk"
BUILD_DIR="$REPO_ROOT/build"
TARGET="keyball/keyball39:user_right"
ARTIFACT="keyball_keyball39_user_right.uf2"

mkdir -p "$BUILD_DIR"

echo "[BUILD] $TARGET"
make -C "$VIAL_DIR" SKIP_GIT=yes VIAL_ENABLE=yes "$TARGET"

for src in "$VIAL_DIR/.build/$ARTIFACT" "$VIAL_DIR/$ARTIFACT"; do
  if [ -f "$src" ]; then
    cp "$src" "$BUILD_DIR/$ARTIFACT"
    echo "  -> $BUILD_DIR/$ARTIFACT"
    exit 0
  fi
done

echo "[ERROR] $TARGET の .uf2 が見つかりません" >&2
exit 1
