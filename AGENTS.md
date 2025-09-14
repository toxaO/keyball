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
- ビルドに関しては作業終了時にコマンド make -C vial-qmk SKIP_GIT=yes VIAL_ENABLE=yes keyball/keyball44:mymapで行ってください。

- keyball61はduplex matrixを採用しているため、対応が必要。

## 現在の不具合
- 現在確認されている不具合の一つに、defaultレイヤのーrow0, col1が初期状態でkc=0003になってしまう。vial上で書き換えれば問題ない。KC_Qのキーコードがkc=0014であるため、11少ないKCが送出されているのかと考え、kc_8(kc_0025)を配置してみたところKC_0(kc=0023)が登録されていた。原因不明。vialでの初期キーボードレイアウトを変更したら今度はKC_0015になった。

## 頒布のための対応事項
- 頒布用のキーマップではデフォルト表示のマップにはユーザーレベルのカスタムキーコードは含めないようにする。
- 頒布用のキーマップを新たに作成する必要がある。現在使用しているmymapをベースにコピーしてvialというマップを作成して、defaultのキーマップを元にしたキー配列のマップを作成する必要がある。また、vial用にはmymapで使用しているのとは別のlib_userを使用する。mymapのlib_userは自分の使用環境に合わせたものになっているため、vial用にはより汎用的なものを作成する必要がある。
- 現在のライブラリの参照やディレクトリ構造ではlib_userがlibと並列階層にあるため、user別にlib_user実装を分けることができない。現在のlib_userディレクトリの下にuser毎のディレクトリを作成して、その中にmymap用とvial用のlib_userを分けて配置する必要がある。
- 頒布用のキーマップではOSによってデフォルトレイヤーを0か1にする必要がある。macOSでは0、Windowsでは1をデフォルトレイヤーにする。
- AMLの遷移レイヤーをoled上で設定できるようにする。
- ドキュメントの整備。最上位のREADMEはkeyball公式のものに本ファームの仕様の一部やセットアップ方法を追記してあるが、ソースコード編集用手順の充実と、カスタムキーの一覧を追記、編集する必要がある。また、使用上の注意点も個々に記載。
- keyball/libやkeyball/lib_userにはAPIや実装例などをREADMEを追記する。
- vialのkey override機能が対応されていないので、対応する。

## 将来的な対応事項
- カスタムキーによるパラメータの変更はキーの位置を把握するのとShiftキーとの同時押しが必要であるため、操作が煩雑である。より簡単にパラメータを変更できるようにするため、OLED上にパラメータ変更用のUIを表示して、方向キーで選択して変更できるようにする。
- oled上の表示の向きを手前から見たときに数値を読みやすい状態にするため、右手側を時計回りに90度回転させたい。
- デバッグ用の入力コードをuprintで出力している処理がコンソールの出力を汚してしまうため、デバッグ用の出力コードを分類して、モードに応じた出力をできるようにする必要がある。デバッグレベルに応じて出力するようにして、カスタムキーコードの実装の際のデバッグにも利用できるようにする
- qmk0.26まではpointing device driverをkeyball付属のkeyball/drivers/pmw3360にあるものを使用していたが、qmk0.27以降対応では使用するdriverをqmk_firmware公式に存在するdriverに変更した。これによる影響で、以前はsplit keyboardのどちら側でもusbケーブルを指してもポインターを動かせたが、現在ではボール側に接続しないと動かなくなっている。また、独自RPCで動かすことも試してみたが、以前より動きが固く、また動く方向も回転してしまっていた。できるならばkeyball/drivers/pmw3360を再度poiinting device driverとして使用できるようにしたいが、qmk0.27以降でのpointing device driverの仕様変更が大きく、対応が必要。
