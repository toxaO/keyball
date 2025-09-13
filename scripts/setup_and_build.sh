#!/usr/bin/env bash
# macOS / Ubuntu 用
# 目的: venv + qmk セットアップ → keyboards/keyball の丸ごとリンク → 複数ターゲットを連続ビルド
# 実行場所: ルート（keyball/ と qmk_firmware/ が並んでいる）
# 注意: 公式 QMK のタグをチェックアウトしてビルドします（既定は 0.30.3）。
# 使い方:
#   bash scripts/setup_and_build.sh
#   QMK_TAG=0.30.4 bash scripts/setup_and_build.sh   # ← 使いたい QMK タグを上書き
set -euo pipefail

QMK_DIR="qmk_firmware"
KEYBALL_DIR="keyball"
VENV_DIR=".venv"
QMK_TAG_DEFAULT="0.30.3"   # 既定で使用する公式 QMK タグ
QMK_TAG="${QMK_TAG:-$QMK_TAG_DEFAULT}"

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
say "Prepare qmk_firmware (checkout tag: $QMK_TAG)"

# qmk_firmware が git 管理でない場合はエラー
if [ ! -d "$QMK_DIR/.git" ]; then
  err "$QMK_DIR は Git リポジトリではありません（サブモジュール / クローンが必要）"
  exit 1
fi

# タグ取得（origin に無ければ upstream=official を追加して取得）
( \
  cd "$QMK_DIR" && \
  git fetch --tags origin || true && \
  if ! git rev-parse -q --verify "refs/tags/$QMK_TAG" >/dev/null; then
    if ! git remote get-url upstream >/dev/null 2>&1; then
      say "Add official remote as 'upstream'"
      git remote add upstream https://github.com/qmk/qmk_firmware.git
    fi
    git fetch --tags upstream || true
  fi && \
  # v接頭辞も試す
  TAG_TO_USE="$QMK_TAG" && \
  if ! git rev-parse -q --verify "refs/tags/$TAG_TO_USE" >/dev/null; then
    if git rev-parse -q --verify "refs/tags/v$QMK_TAG" >/dev/null; then
      TAG_TO_USE="v$QMK_TAG"
    fi
  fi && \
  say "Checkout QMK tag: ${TAG_TO_USE}" && \
  git checkout -f "${TAG_TO_USE}" && \
  git submodule sync --recursive || true && \
  git submodule update --init --recursive --depth 1 || true \
)

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
if [ -e "$LINK_FROM" ] || [ -L "$LINK_FROM" ]; then
  rm -rf "$LINK_FROM"
fi
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
