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
- vial.jsonのレイアウトがまだ正しく作成されていない。39と44に関してはとりあえずrightは正しい。

## 現在の不具合
- 現在確認されている不具合の一つに、レイヤー0のrow0, col1がeeprom初期状態でkeymap.cで指定しているものとkcがずれる。vial上で書き換えれば動作に問題はない。KC_Q(kc0014)を指定するとKC_R(kc0015)が入力される。また、vialに接続するとkc0001に変更される。
- leftの時に表示されるレイアウトがおかしい。44での初期キーコードがズレる問題と同じで、キーボードレイアウトの設定問題から発生しているのか。keyboard layout editor上では正常に表示される。

## 頒布のための対応事項
- 頒布用のキーマップを新たに作成する必要がある。現在使用しているmymapをベースにコピーしてvialというマップを作成して、defaultのキーマップを元にしたキー配列のマップを作成する必要がある。また、vial用にはmymapで使用しているのとは別のlib_userを使用する。mymapのlib_userは自分の使用環境に合わせたものになっているため、vial用にはより汎用的なものを作成する必要がある。
- ドキュメントの整備。

## 将来的な対応事項
- デバッグ用の入力コードをuprintで出力している処理がコンソールの出力を汚してしまうため、デバッグ用の出力コードを分類して、モードに応じた出力をできるようにする必要がある。デバッグレベルに応じて出力するようにして、カスタムキーコードの実装の際のデバッグにも利用できるようにする
- qmk0.26まではpointing device driverをkeyball付属のkeyball/drivers/pmw3360にあるものを使用していたが、qmk0.27以降対応では使用するdriverをqmk_firmware公式に存在するdriverに変更した。これによる影響で、以前はsplit keyboardのどちら側でもusbケーブルを指してもポインターを動かせたが、現在ではボール側に接続しないと動かなくなっている。また、独自RPCで動かすことも試してみたが、以前より動きが固く、また動く方向も回転してしまっていた。できるならばkeyball/drivers/pmw3360を再度poiinting device driverとして使用できるようにしたいが、qmk0.27以降でのpointing device driverの仕様変更が大きく、対応が必要。
