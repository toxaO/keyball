# Repository Guidelines

## general
- 返信に関してはutf-8日本語で行うこと
- コメントの付記に関しても同様

## ディレクトリ構造
- `keyball` keyball本体
- `keyball/lib` keyball関係のkbレベルのコードが収納されています。
- `keyball/lib_user` ユーザレベルのコードが収納されています。
- `keyball/lib_user/user` user各自で好きにいじってください。
  `keyball/lib_user/toxaO` toxaOが普段使用しているコードです。サンプルにどうぞ。
- `qmk_firmware` keyballへリンクを張ったqmkです。
- `scripts` セットアップ用のシェルスクリプトです。
- `.github/workflows` github actionsでビルドするための設定です。
- `vial-qmk` keyballへのリンクを張ったvialです。
- `build` ビルド済みのボード書き込み用のファイルが入っています。

## ビルドに関して
- コマンドは"make -C vial-qmk SKIP_GIT=yes VIAL_ENABLE=yes keyball/keyball{39, 44, 61}:user"で行えます。

## 現在の状況に関して
- toxaOは自分個人用のキーマップを指す。
- userは頒布用のキーマップを指す。
- ビルドに関しては作業終了時にコマンド make -C vial-qmk SKIP_GIT=yes VIAL_ENABLE=yes keyball/keyball{39, 44, 61}:toxaOで行ってください。
- one shot modifierのon/offをLEDの点灯で表すためにrgblight_atを使用したい。RPCの実装が必要だが検証が必要。

## 現在の不具合
- 現在は確認されている不具合はない。

## 頒布のための対応事項
- 詳細ドキュメントの整備。
- setup_and_build.shの動作確認。

## 将来的な対応事項
- デバッグ用の入力コードをuprintで出力している処理がコンソールの出力を汚してしまうため、デバッグ用の出力コードを分類して、モードに応じた出力をできるようにする必要がある。
- デバッグレベルに応じて情報を出力するようにして、カスタムキーコードの実装の際のデバッグにも利用できるようにする
- 設定変更値をコンソールに出力するようにする。(OLEDを装着していなくても値が確認できるように)
