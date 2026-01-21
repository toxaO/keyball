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
- `bash scripts/setup_and_build.sh`でuserの一括ビルドが行えます。
- `meke -C vial-qmk SKIP_GIT=yes VIAL_ENABLE=yes keyball/keyball{39, 44, 61}:toxaO`でtoxaOのビルドが行えます。
- codexによるコード編集後はこれらのコマンドを使用して問題なくビルドできることを確認してください。

## 現在の状況に関して
- toxaOは自分個人用のキーマップを指す。
- userは頒布用のキーマップを指す。

## 現在の不具合
- 現在は確認されている不具合はない。

## 頒布のための対応事項
- 現在は対応事項はない。

## 実装予定機能
- 修飾キーそれぞれに対して、haptic driverの左右振動のon/offと振動エフェクトをoled上で設定可能にする。
- レイヤーごとにledのインデックスとhsv値をoled上で設定可能にし、インジケータとして利用可能にする。修飾キーも同様。
- 個人的なものだが、レイヤー順序をdefの次にNumP, PAD, 以降現在順になるように入れ替える。

## 将来的な対応事項
- デバッグ用の入力コードをuprintで出力している処理がコンソールの出力を汚してしまうため、デバッグ用の出力コードを分類して、モードに応じた出力をできるようにする必要がある。
- デバッグレベルに応じて情報を出力するようにして、カスタムキーコードの実装の際のデバッグにも利用できるようにする
- 設定変更値をコンソールに出力するようにする。(OLEDを装着していなくても値が確認できるように)
