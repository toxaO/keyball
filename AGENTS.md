# Repository Guidelines

## Project Structure & Module Organization
- `keyball/` — Keyboard sources. Boards: `keyball39/`, `keyball44/`, `keyball61/`. Shared code in `lib/` (project libs) and `lib_user/` (user features).
- `qmk_firmware/` — Pinned QMK tree used for builds (includes `.build/` outputs).
- `scripts/` — Helper scripts (e.g., `scripts/setup_and_build.sh`).
- `.github/workflows/` — CI that compiles firmware via the QMK CLI.
- `vial-qmk` - to build vial uf2 file.

Keymaps live under `keyball/<board>/keymaps/<name>/` with `keymap.c` (optionally `rules.mk`, `config.h`). User-level shared code is under `keyball/lib_user/`.

## Coding Style & Naming Conventions
- Language: C for QMK; follow QMK’s `.clang-format` (4‑space indent, no tabs).
- Naming: snake_case for functions/vars; headers use `#pragma once`. Filenames lowercase with underscores (e.g., `keyball_move.c`).
- Place shared features in `keyball/lib/` or `keyball/lib_user/`; board‑specific code stays in the board directory.

## Commit & Pull Request Guidelines
- Commits: concise, imperative, scoped. Prefer Conventional Commits:
  - `feat(keyball44): add thumb layer toggles`
  - `fix(lib): correct wheel delta scaling`
  - `refactor(lib_user): tidy combo handling`
- PRs: include summary, affected boards/keymaps, rationale, and build proof (artifact path or CI green). Link issues. Add screenshots/layout JSON for VIA when relevant. Keep PRs focused; avoid mixing refactors with feature changes.

## Notes & Tips
- Artifacts land in `qmk_firmware/.build/` and sometimes repo root `.build/` (see CI). Set `QMK_FLOAT=1` with the script to temporarily advance QMK for testing.

- 返信に関してはutf-8日本語で行うこと
- コメントの付記に関しても同様

# 現在の状況に関して
- mymapは自分用のキーマップを指す。

- vialは頒布用のキーマップを指す。

- keyball44と39に関してはqmkとvialでビルド可能で動作確認済。61はこれから。
- ビルドに関しては作業終了時にコマンド make -C vial-qmk SKIP_GIT=yes VIAL_ENABLE=yes keyball/keyball39:mymapで行ってください。

- keyball61はduplex matrixを採用しているため、対応が必要。

## 現在の不具合
- 現在確認されている不具合の一つに、defaultレイヤのーrow0, col1が初期状態でkc=0003になってしまう。vial上で書き換えれば問題ない。KC_Qのキーコードがkc=0014であるため、11少ないKCが送出されているのかと考え、kc_8(kc_0025)を配置してみたところKC_0(kc=0023)が登録されていた。原因不明。vialでの初期キーボードレイアウトを変更したら今度はKC_0015になった。この現象に関しては原因の想定がつくまで一旦放置。そこまで害はない。
- leftの時に表示されるレイアウトがおかしい。44での初期キーコードがズレる問題と同じで、キーボードレイアウトの設定問題から発生しているかもしれないが、対応は後回しで良い。
- いっそレイアウトをvialで選択する方式ではなく、レイアウトごとにファームを作る方向性でも良い。

## 頒布のための対応事項
- 頒布用のキーマップを新たに作成する必要がある。現在使用しているmymapをベースにコピーしてvialというマップを作成して、defaultのキーマップを元にしたキー配列のマップを作成する必要がある。また、vial用にはmymapで使用しているのとは別のlib_userを使用する。mymapのlib_userは自分の使用環境に合わせたものになっているため、vial用にはより汎用的なものを作成する必要がある。
- デフォルトレイヤーの設定もOLED上でできるようにする。
- ドキュメントの整備。

## 将来的な対応事項
- デバッグ用の入力コードをuprintで出力している処理がコンソールの出力を汚してしまうため、デバッグ用の出力コードを分類して、モードに応じた出力をできるようにする必要がある。デバッグレベルに応じて出力するようにして、カスタムキーコードの実装の際のデバッグにも利用できるようにする
- qmk0.26まではpointing device driverをkeyball付属のkeyball/drivers/pmw3360にあるものを使用していたが、qmk0.27以降対応では使用するdriverをqmk_firmware公式に存在するdriverに変更した。これによる影響で、以前はsplit keyboardのどちら側でもusbケーブルを指してもポインターを動かせたが、現在ではボール側に接続しないと動かなくなっている。また、独自RPCで動かすことも試してみたが、以前より動きが固く、また動く方向も回転してしまっていた。できるならばkeyball/drivers/pmw3360を再度poiinting device driverとして使用できるようにしたいが、qmk0.27以降でのpointing device driverの仕様変更が大きく、対応が必要。
- スワイプ動作を利用したフリック入力機能。あと矢印入力機能。
