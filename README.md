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
## 追加: lib/keyball の主な機能まとめ（最新）

以下は `keyball/lib/keyball` に実装された共通機能の要点です（最新実装に追随）。

- ポインタ/スクロール/AML/スワイプ/RGB の調整
  - すべて OLED の設定ページで調整します（OS別に保存される項目あり）。
  - 保存は `KBC_SAVE`、初期化は `KBC_RST`。

- スクロール操作のクイックキー（キーボードレベル）
  - `SCRL_TO`: スクロールモードのトグル
  - `SCRL_MO`: 押下中のみスクロールモード有効
  - スクロールスナップ切替: `SSNP_VRT` / `SSNP_HOR` / `SSNP_FRE`（有効時）

- スワイプ実行（キーボードレベル）
  - `APP_SW` / `VOL_SW` / `BRO_SW` / `TAB_SW` / `WIN_SW`
  - 押下で開始・解放で終了。方向別アクションは user 側フック
    `keyball_on_swipe_fire/end/tap` で実装します。

- OLED 表示
  - 設定ページで Mouse/AML/Scroll/ScrollSnap/Raw/SwipeCfg/SwipeMon/RGB を確認・調整。

- EEPROM プロファイル（kbpf）
  - OS別: CPI, スクロール（ST/反転/プリセット/interval/value）、ポインタ移動量（Glo/Th1）
  - グローバル: スワイプ（閾値/デッドゾーン/リセットms/フリーズ）、AML（有効/TO/TG）
  - `KBC_SAVE` で一括保存。
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
