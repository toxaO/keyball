# AGENTS Instructions

- QMK コマンドは常に Python の仮想環境上で実行すること。

  ```bash
  python -m venv .venv
  source .venv/bin/activate
  ```

- QMK のホームディレクトリを以下で設定する。

  ```bash
  qmk setup -H keyball/qmk_firmware
  ```

- ファームウェアをビルドする際はアクティブな仮想環境上で次を使う。

  ```bash
  qmk compile -kb keyball/keyball39 -km mymap
  ```
