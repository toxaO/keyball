#!/usr/bin/env bash
# macOS / Ubuntu 用
# 目的: venv + qmk セットアップ → keyboards/keyball の丸ごとリンク → 複数ターゲットを連続ビルド
# 実行場所: ルート（keyball/ と qmk_firmware/ が並んでいる）
# 注意: 本プロジェクトで使用できる QMK は「同梱の qmk_firmware（keyball ブランチ）」のみです。
#       上流の任意バージョンや別ブランチではビルドできない／挙動が一致しない可能性があります。
# 使い方:
#   bash scripts/setup_and_build.sh
#   QMK_FLOAT=1 bash scripts/setup_and_build.sh   # ← qmk_firmware を keyball ブランチの最新へ進めてからビルド
set -euo pipefail

QMK_DIR="qmk_firmware"
KEYBALL_DIR="keyball"
VENV_DIR=".venv"

# 連続ビルドするターゲット
BUILDS=(
  "keyball/keyball39:mymap"
  "keyball/keyball44:mymap"
)

say(){ printf "\033[1;34m[INFO]\033[0m %s\n" "$*"; }
err(){ printf "\033[1;31m[ERR ]\033[0m %s\n" "$*"; }

# 前提ディレクトリ確認
[ -d "$QMK_DIR" ]    || { err "$QMK_DIR が見つかりません"; exit 1; }
[ -d "$KEYBALL_DIR" ]|| { err "$KEYBALL_DIR が見つかりません"; exit 1; }

# QMK submodule 初期化（固定コミットで取得）
say "Submodules init/sync (pinned)…"
git -C "$QMK_DIR" submodule sync --recursive || true
git -C "$QMK_DIR" submodule update --init --recursive --depth 1 || true

# 任意: QMK を keyball ブランチ最新へ“浮かせる”（再現性は下がる）
if [ "${QMK_FLOAT:-0}" = "1" ]; then
  say "Float qmk_firmware to branch 'keyball' (latest)"
  git -C "$QMK_DIR" fetch origin keyball
  git -C "$QMK_DIR" checkout keyball || git -C "$QMK_DIR" checkout -b keyball origin/keyball
  git -C "$QMK_DIR" pull --ff-only origin keyball || true
fi

# ARM ツールチェイン確認
if ! command -v arm-none-eabi-gcc >/dev/null 2>&1; then
  say "Install toolchain:"
  case "$(uname -s)" in
    Darwin)  echo "  brew install arm-none-eabi-gcc"; exit 1;;
    Linux)   echo "  sudo apt-get update && sudo apt-get install -y gcc-arm-none-eabi binutils-arm-none-eabi"; exit 1;;
    *)       err "未対応OS。GNU Arm Embedded Toolchain を導入してください"; exit 1;;
  esac
fi

# venv 準備 & qmk CLI
if [ ! -d "$VENV_DIR" ]; then
  say "Create venv: $VENV_DIR"
  python3 -m venv "$VENV_DIR"
fi
# shellcheck disable=SC1091
source "$VENV_DIR/bin/activate"
python3 -m pip install -U pip qmk

# qmk home 設定（冪等）
QMK_HOME_ABS="$(cd "$QMK_DIR" && pwd)"
say "qmk setup -H $QMK_HOME_ABS"
qmk setup -H "$QMK_HOME_ABS" -y || true

# keyboards/keyball → ../../keyball へ「丸ごとリンク」（lib_user も見せる）
LINK_FROM="$QMK_DIR/keyboards/keyball"
LINK_TO="../../$KEYBALL_DIR"
[ -e "$LINK_FROM" ] || [ -L "$LINK_FROM" ] && rm -rf "$LINK_FROM"
mkdir -p "$QMK_DIR/keyboards"
ln -s "$LINK_TO" "$LINK_FROM"
say "symlink: $LINK_FROM -> $LINK_TO"

# 連続ビルド
for pair in "${BUILDS[@]}"; do
  KB="${pair%%:*}"
  KM="${pair##*:}"
  say "Build -> KB=$KB  KM=$KM"
  qmk clean -q -n -y || true
  qmk compile -kb "$KB" -km "$KM"
done

say "Done. artifacts → $QMK_DIR/.build/"
