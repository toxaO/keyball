# Repository Guidelines

## general
- 返信に関してはutf-8日本語で行うこと
- コメントの付記に関しても同様

## ディレクトリ構造
- `keyball` keyball本体
- `qmk_firmware` keyballへリンクを張ったqmkです。
- `scripts` セットアップ用のシェルスクリプトです。
- `.github/workflows` github actionsでビルドするための設定です。
- `vial-qmk` keyballへのリンクを張ったvialです。
- `build` ビルド済みのボード書き込み用のファイルが入っています。

- Keymaps live under `keyball/<board>/keymaps/<name>/` with `keymap.c` (optionally `rules.mk`, `config.h`). User-level shared code is under `keyball/lib_user/`.

## 現在の状況に関して
- mymapは自分用のキーマップを指す。
- vialは頒布用のキーマップを指す。一通りmymapでの機能の充実を終えてから、汎用性の低いユーザーレベルのカスタムを消去してvialマップを作成する方針。
- keyball44と39に関してはqmkとvialでビルド可能で動作確認済。61はビルドは可能だが動作は未確認。
- ビルドに関しては作業終了時にコマンド make -C vial-qmk SKIP_GIT=yes VIAL_ENABLE=yes keyball/keyball{39, 44, 61}:mymapで行ってください。
- keyball61はduplex matrixを採用しているため、対応が必要。
- vial.jsonのためのkeyboard layout editorで作成したjsonファイルを
- one shot modifierのon/offをLEDの点灯で表すためにrgblight_atを使用したい。RPCの実装が必要だが検証が必要。

## 現在の不具合
- 現在は確認されている不具合はない。

## 頒布のための対応事項
- 頒布用のキーマップを新たに作成する必要がある。現在使用しているmymapをベースにコピーしてvialというマップを作成して、defaultのキーマップを元にしたキー配列のマップを作成する必要がある。また、vial用にはmymapで使用しているのとは別のlib_userを使用する。mymapのlib_userは自分の使用環境に合わせたものになっているため、vial用にはより汎用的なものを作成する必要がある。
- ドキュメントの整備。

## 将来的な対応事項
- デバッグ用の入力コードをuprintで出力している処理がコンソールの出力を汚してしまうため、デバッグ用の出力コードを分類して、モードに応じた出力をできるようにする必要がある。デバッグレベルに応じて出力するようにして、カスタムキーコードの実装の際のデバッグにも利用できるようにする
