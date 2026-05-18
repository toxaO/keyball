#!/usr/bin/env bash
#
# Keyball 用 セットアップ + ビルド 一発スクリプト（超丁寧コメント付き）
#
# 対象OS:
#   - Windows: MSYS2(MINGW64) シェル想定（Start Menu から MSYS2 MinGW 64-bit を起動）
#   - macOS  : Terminal.app + Homebrew
#   - Ubuntu : 22.04/24.04 系（apt）
#
# できること:
#   1) 依存コマンドの有無を確認（無ければ各OSごとの導入コマンドを案内）
#   2) vial-qmk 側の keyboards/keyball 参照を確認
#   3) Vial 側で現行キーマップ（toxaO / user_*）を一括ビルド
#
# 注意:
#   - 本リポジトリ直下で実行してください（keyball/ と qmk_firmware/ と vial-qmk/ が並んでいる前提）。
#   - 依存の導入（arm-none-eabi など）は管理者権限が必要な場合があります。下記メッセージの通りに実行してください。
#   - ビルド成果物は qmk_firmware/.build/ および vial-qmk/.build/ に生成されます（.uf2 など）。

set -Eeuo pipefail

VIAL_DIR="vial-qmk"         # Vial用のQMKツリー（このリポ内の同名ディレクトリ）
KEYBALL_DIR="keyball"       # 本リポのキーボード実装

say() { printf "\033[1;34m[INFO]\033[0m %s\n" "$*"; }
err() { printf "\033[1;31m[ERR ]\033[0m %s\n" "$*"; }

# --- OS 判定（MSYS2 / macOS / Linux）-----------------------------------------
os_detect() {
  case "${MSYSTEM:-}" in
    MINGW64|UCRT64) echo "msys2"; return;;
  esac
  case "$(uname -s)" in
    Darwin) echo "mac"; return;;
    Linux)  echo "linux"; return;;
  esac
  echo "unknown"
}

OSKIND="$(os_detect)"
say "Detected OS: $OSKIND"

# --- ルート構成の確認 ---------------------------------------------------------
[ -d "$VIAL_DIR" ] || { err "$VIAL_DIR が見つかりません。リポジトリ直下で実行してください"; exit 1; }
[ -d "$KEYBALL_DIR" ] || { err "$KEYBALL_DIR が見つかりません"; exit 1; }

# --- 依存コマンドの有無を確認（入っていなければ導入コマンドを案内）-------------
need_cmds=("git" "make" "arm-none-eabi-gcc")
missing=()
for c in "${need_cmds[@]}"; do
  if ! command -v "$c" >/dev/null 2>&1; then missing+=("$c"); fi
done

if [ ${#missing[@]} -gt 0 ]; then
  err "不足しているコマンド: ${missing[*]}"
  echo "導入例:"
  if [ "$OSKIND" = "mac" ]; then
    echo "  brew install git make arm-none-eabi-gcc"
  elif [ "$OSKIND" = "linux" ]; then
    echo "  sudo apt-get update && sudo apt-get install -y git make gcc-arm-none-eabi binutils-arm-none-eabi"
  elif [ "$OSKIND" = "msys2" ]; then
    echo "  pacman -S --needed git make"
    echo "  pacman -S --needed mingw-w64-ucrt-x86_64-arm-none-eabi-gcc mingw-w64-ucrt-x86_64-arm-none-eabi-binutils"
    echo "  （MSYS2はUCRT64またはMINGW64シェルで実行してください）"
  else
    echo "  お使いの環境に合わせて GNU Arm Embedded Toolchain 等を導入してください"
  fi
  exit 1
fi

# --- キーボード本体をVialツリーへリンク ---------------------------------------
# 目的: 本リポの keyball/ を vial-qmk から参照可能にするための symlink を確認する。
# 注意: vial-qmk/keyboards/keyball 自体を丸ごと置き換えるとサブモジュールが
# dirty になるため、必要な中身だけを確認・作成する。
link_into_tree() {
  local tree_dir="$1"
  local base="$tree_dir/keyboards/keyball"
  local entries=("config.h" "keyball" "keyball39" "keyball44" "keyball61" "lib" "lib_user")

  mkdir -p "$base"
  for entry in "${entries[@]}"; do
    local from="$base/$entry"
    local to="../../../$KEYBALL_DIR/$entry"
    if [ "$entry" = "keyball" ]; then
      to="../../keyball"
    fi

    if [ -L "$from" ]; then
      if [ "$(readlink "$from")" != "$to" ]; then
        rm "$from"
        ln -s "$to" "$from"
        say "symlink: $from -> $to"
      fi
    elif [ -e "$from" ]; then
      say "exists: $from"
    else
      ln -s "$to" "$from"
      say "symlink: $from -> $to"
    fi
  done
}

link_into_tree "$VIAL_DIR"

# --- Vial 側ビルド -------------------------------------------------------------
# 公式の推奨手順に合わせて make を直接使用。
#   -C vial-qmk   : このディレクトリでMakeを実行
#   SKIP_GIT=yes  : インターネット接続前提のバージョン取得をスキップ（ローカルのみで完結）
#   VIAL_ENABLE=yes: Vial 機能を有効化
#   最後の引数     : QMKのターゲット名（keyboard:keymap）

keyboards=("keyball39" "keyball44" "keyball61")
keymaps=("toxaO" "user_right" "user_left" "user_dual")

for kb in "${keyboards[@]}"; do
  for km in "${keymaps[@]}"; do
    target="keyball/${kb}:${km}"
    say "[Vial] build -> $target (SKIP_GIT=yes VIAL_ENABLE=yes)"
    make -C "$VIAL_DIR" SKIP_GIT=yes VIAL_ENABLE=yes "$target"
  done
done

say "Vial build done. artifacts → $VIAL_DIR/.build/"
say "All done! 生成された .uf2 を各 .build/ ディレクトリで確認してください。"
