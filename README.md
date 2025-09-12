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

1. Check out this repository.

    ```console
    $ git clone https://github.com/Yowkees/keyball.git keyball
    ```

2. Check out [qmk/qmk_firmware](https://github.com/qmk/qmk_firmware/) repository in another place.

    ```console
    $ git clone https://github.com/qmk/qmk_firmware.git --depth 1 --recurse-submodules --shallow-submodules -b 0.22.14 qmk
    ```

    Currently Keyball firmwares are verified to compile with QMK 0.22.14

3. Create a symbolic link to this `keyball/` directory from [qmk/qmk_firmware]'s `keyboards/` directory.

    ```console
    $ ls
    keyball/ qmk/

    $ cd qmk/keyboards
    $ ln -s ../../keyball/qmk_firmware/keyboards/keyball keyball
    $ ls keyball/
    drivers/  keyball39/  keyball44/  keyball46/  keyball61/  lib/  one47/  readme.md
    $ cd ..
    ```

4. `make` your Keyball firmware.

    ```console
    # Build Keyball39 firmware with "default" keymap
    $ make SKIP_GIT=yes keyball/keyball39:default

    # Build Keyball44 firmware with "default" keymap
    $ make SKIP_GIT=yes keyball/keyball44:default

    # Build Keyball61 firmware with "default" keymap
    $ make SKIP_GIT=yes keyball/keyball61:default
    ```

There are three keymaps provided at least:

* `via` - Standard version with [Remap](https://remap-keys.app/) or VIA to change keymap
* `test` - Easy-to-use version for checking operation at build time
* `default` - Base version for creating your own customized firmware

## How to create your keymap

1. Fork this Yowkees/keyball repository
2. Checkout forked repository
3. (OPTIONAL) Create a new branch
4. Add a your keymap, or make some changes
5. Commit changes and push it to your forked repository
6. Open your forked repository with web browser
7. Click and open "Actions" tab
8. Click "Build a firmware on demand" in Workflows on left panel
9. Press "Run workflow" button on right side, then you will see forms
10. (OPTIONAL) Select a your working branch
11. Select a "Keyboard" from drop-down list
12. Enter the "keymap" you want to build
13. Click "Run workflow"
14. Wait a minute until the firmware build is finished
15. Click a latest workflow run and open details
16. Download built firmware in "Artifacts" section
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
