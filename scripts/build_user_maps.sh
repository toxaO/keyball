#!/usr/bin/env bash
set -euo pipefail

# user系レイアウト(dual/left/right)を全キーボードでビルドするだけのスクリプト
ROOT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT_DIR"

KEYBOARDS=(keyball39 keyball44 keyball61)
KEYMAPS=(user_dual user_left user_right)
MAKE_ARGS=(SKIP_GIT=yes VIAL_ENABLE=yes)

for kb in "${KEYBOARDS[@]}"; do
  for km in "${KEYMAPS[@]}"; do
    printf '==> build %s:%s\n' "$kb" "$km"
    make -C vial-qmk "${MAKE_ARGS[@]}" "keyball/${kb}:${km}"
  done
done
