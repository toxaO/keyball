# Keyball series

This directory includes source code of Keyball keyboard series:

| Name          | Description
|---------------|-------------------------------------------------------------
|[Keyball46](./keyball46)|A split keyboard with 46 vertically staggered keys and 34mm track ball.
|[Keyball61](./keyball61)|A split keyboard with 61 vertically staggered keys and 34mm track ball.
|[Keyball39](./keyball39)|A split keyboard with 39 vertically staggered keys and 34mm track ball.
|[ONE47](./one47)|A keyboard with 47 vertically keys and 34mm trackball. It will support BLE Micro Pro.
|[Keyball44](./keyball44)|A split keyboard with 44 vertically staggered keys and 34mm track ball.

* Keyboard Designer: [@Yowkees](https://twitter.com/Yowkees)  
* Hardware Supported: ProMicro like footprint
* Hardware Availability: See [Where to Buy](../../../README.md#where-to-buy)

See each directories for each keyboards in a table above.

## How to build

This repo includes `qmk_firmware/` as a submodule. We build against the official QMK tag, defaulting to 0.30.3.

Option A: Use the script (recommended)

1) Clone and enter the repo

```console
git clone https://github.com/Yowkees/keyball.git
cd keyball
```

2) Run the setup script (creates venv, sets QMK home, links keyboards, builds examples)

```console
bash scripts/setup_and_build.sh          # uses QMK 0.30.3 by default
QMK_TAG=0.30.4 bash scripts/setup_and_build.sh   # override QMK tag if needed
```

Artifacts are placed under `qmk_firmware/.build/`.

Option B: Manual build with QMK CLI

```console
# one-time
python3 -m pip install --user qmk
qmk setup -H "$PWD/qmk_firmware" -y
rm -rf qmk_firmware/keyboards/keyball && mkdir -p qmk_firmware/keyboards && ln -s ../../keyball qmk_firmware/keyboards/keyball

# build
qmk compile -kb keyball/keyball44 -km mymap
qmk compile -kb keyball/keyball39 -km mymap
```

There are three keymaps provided at least:

* `via` - Standard version with [Remap](https://remap-keys.app/) or VIA to change keymap
* `test` - Easy-to-use version for checking operation at build time
* `default` - Base version for creating your own customized firmware

## Build on GitHub Actions / キーマップ作成とActionsビルド

1. リポジトリをFork（GitHub上でこのリポジトリを自分のアカウントへフォーク）
2. ローカルへクローンし、リモートを自分のForkへ向ける
   - `git clone https://github.com/<your-account>/keyball.git`
   - `cd keyball`
   - `git remote -v` で origin が自分のForkを指していることを確認
3. （任意）作業ブランチを作る
   - `git checkout -b mymap-work`
4. キーマップを作成/編集
5. コミットして自分のForkへPush
   - `git add -A && git commit -m "Add my keymap"`
   - `git push -u origin mymap-work`
6. GitHub上の自分のForkを開く
7. Actionsタブを開く
8. 左のWorkflowsから "Build a firmware on demand" を選択
9. 右側の "Run workflow" を押してフォームを開く
10. 対象ブランチを選択（例: mymap-work）
11. Keyboard を選ぶ（例: keyball/keyball44）
12. Keymap を入力（例: mymap）
13. （任意）QMK tag を指定（既定: 0.30.3）
13. Run workflow を押す
14. ビルド完了まで待つ（数分）
15. 最新のワークフロー実行を開く（詳細へ）
16. Artifacts セクションから生成物（UF2等）をダウンロード
# Keyball
## 追加: lib/keyball の主な機能まとめ

以下は `keyball/lib/keyball` に実装された共通機能の要点です（既存の本文は保持し、追記のみ）。

- ポインタ調整
  - 低速域ゲイン/しきい値（move shaping）をOS別に保存。
  - `MVGL`（低速ゲイン調整）、`MVTH1`（しきい値1調整）。

- スクロール調整
  - ST（スクロールステップ）で速度段階を変更。`SCRL_STI` / `SCRL_STD`
  - スクロール方向の反転は `SCRL_INV`（OS別に保存）。
  - プリセット切替 `SCRL_PST`：macOS={120,120} 固定、その他={120,1}↔{1,1} をトグル。

- スワイプ機能
  - `SW_ST`（発火しきい値）、`SW_DZ`（デッドゾーン）、`SW_RT`（リセット遅延）、`SW_FRZ`（凍結）
  - フック `keyball_on_swipe_fire(tag, dir)` で任意のアクションを実行可能。

- OLED 表示/デバッグ
  - 通常モード: レイヤ情報などを簡潔に表示。
  - デバッグモード: 複数ページで設定値・スワイプ・スクロールの内部状態を表示。

- EEPROM プロファイル（kbpf）
  - OS別に CPI, ST, 反転, スクロールパラメータ(interval/value), プリセット種別を保存。
  - スワイプ関連（しきい値、デッドゾーン、リセットms、フリーズ）も保存。
  - 本更新では内部構造のフィールド名を分かりやすく整理。既存データの自動移行は行わず、必要に応じて手動で `KBC_RST`/EEPROM初期化を実施してください。

- 設定保存
  - `KBC_SAVE` で現OSスロットをEEPROMへ保存、`KBC_RST` で既定に戻す。

備考:
- 既存の `SCRL_DVI`/`SCRL_DVD` は後方互換のため残していますが、ドキュメントとキーマップでは `SCRL_STI`/`SCRL_STD` の使用を推奨します。
- 既定のSTは `KEYBALL_SCROLL_STEP_DEFAULT`（従来 `KEYBALL_SCROLL_DIV_DEFAULT`）。
## ローカルビルド手順（CUI向け）

以下はCUIに慣れた方向けの導入とビルドの概要です。

- 重要: 本リポジトリでは、同梱の `qmk_firmware` サブモジュールを公式 QMK のタグへチェックアウトして使用します（既定 0.30.3）。

- 前提
  - git / Python 3.10+ が導入済みであること
  - pipx もしくは pip で QMK CLI を導入可能なこと

- 手順（手動セットアップ）
  - リポジトリを取得
    - `git clone https://github.com/<your-account>/keyball.git`
    - `cd keyball`
  - QMK CLI を導入（いずれか）
    - pipx を使う場合: `pipx install qmk`
    - pip を使う場合: `python3 -m pip install --user qmk`
  - QMK のホームを本リポジトリ同梱の `qmk_firmware` に指定
    - `qmk setup -H "$PWD/qmk_firmware"`
      - 初回は依存取得等で時間がかかる場合があります
  - サブモジュールの取得（必要時）
    - `git -C qmk_firmware fetch --tags`
    - `git -C qmk_firmware checkout v0.30.3`（必要に応じて任意タグへ）
    - `git -C qmk_firmware submodule sync --recursive`
    - `git -C qmk_firmware submodule update --init --recursive --depth 1`
  - キーボードディレクトリのリンク（必要時）
    - `ln -s ../../keyball "$PWD/qmk_firmware/keyboards/keyball"`（scripts参照）
  - ビルド
    - keyball44/mymap: `qmk compile -kb keyball/keyball44 -km mymap`
    - keyball39/mymap: `qmk compile -kb keyball/keyball39 -km mymap`

- 手順（スクリプト利用）
  - ルート直下で実行: `bash scripts/setup_and_build.sh`
    - していること（要約）
    - QMK を公式タグへチェックアウト（既定: 0.30.3、`QMK_TAG` で上書き可）
    - venv 構築＋QMK CLI 導入＋ `qmk setup -H` の実行
    - `qmk_firmware/keyboards/keyball -> ../../keyball` のシンボリックリンク作成
    - `keyball39:mymap` と `keyball44:mymap` を連続ビルド

- 備考
  - 現在、ビルド確認済みなのは `keyball39` と `keyball44` の `mymap` のみです。
  - 生成物（UF2等）は `qmk_firmware/.build/` 配下に出力されます。
  - Linuxで書き込みやデバイスアクセスを行う場合、必要に応じてudev設定等も整備してください（本書では割愛）。
  - GitHub Actions によるビルドも利用可能です（本README上部「Actions」の手順参照）。
