# Keyball 開発環境 手動セットアップ手順

## 想定読者
- QMK/Vial のビルドをこれまで自分で行ってこなかったユーザー
- 配布スクリプトが動かなかった場合に、自力で問題を切り分けたいユーザー

## 前提条件
- `git` で本リポジトリをクローン済みであること（`keyball/` と `qmk_firmware/` と `vial-qmk/` が同じ階層に並んでいること）。
- 以下のコマンドがインストールされていること。
  - 共通: `git`, `make`, `arm-none-eabi-gcc`, `arm-none-eabi-binutils`
  - Ubuntu/WSL: `sudo apt install build-essential gcc-arm-none-eabi binutils-arm-none-eabi libnewlib-arm-none-eabi`
  - macOS(Homebrew): `brew install git make arm-none-eabi-gcc`
  - Windows(MSYS2 UCRT64/MINGW64): `pacman -S --needed git make mingw-w64-ucrt-x86_64-arm-none-eabi-gcc mingw-w64-ucrt-x86_64-arm-none-eabi-binutils`
- WSL を使用する場合は、リポジトリを `/home` など Linux 側のファイルシステムに置くこと（`/mnt/c` 直下ではシンボリックリンク作成に失敗する場合があります）。

Ubuntu 20.04 の最小インストールでは `universe` リポジトリが無効化されています。`gcc-arm-none-eabi` などが見つからない場合は、次のコマンドで `universe` を有効化してから再度 `apt install` を試してください。

```sh
sudo apt install -y software-properties-common
sudo add-apt-repository universe
sudo apt update
```

### 新規 Ubuntu 20.04 環境での事前準備例
最低限のセットアップを完全な初期状態から実施する場合は、次の順で進めると安全です。

```sh
sudo apt update
sudo apt install -y software-properties-common
sudo add-apt-repository universe
sudo apt update
sudo apt install -y git build-essential gcc-arm-none-eabi binutils-arm-none-eabi libnewlib-arm-none-eabi
git clone --recurse-submodules https://github.com/toxaO/keyball.git
cd keyball
git submodule status  # サブモジュールが正しく取得されているか確認
```

既に `git clone` を実行済みでサブモジュールを取り込んでいない場合は、リポジトリ直下で `git submodule update --init --recursive` を実行してください。

## シンボリックリンクの作成
Vial から本リポジトリの `keyball/` を参照できるよう、以下のリンクを作成します。

```sh
ln -s ../../keyball vial-qmk/keyboards/keyball
```

QMK でもビルドしたい場合は `qmk_firmware/keyboards/keyball` にも同様のリンクを作成してください。既に `keyboards/keyball` が存在する場合は削除してから再作成してください。

## QMK (任意)
`qmk` CLI を利用したビルドを行いたい場合のみ、仮想環境などを用意します。Vial のみをビルドする場合はこの節をスキップ可能です。

```sh
# Python 3.9 以上であれば他バージョンでも可（例: python3.11）
python3.10 -m venv .venv
source .venv/bin/activate
python -m pip install -U pip qmk
QMK_HOME_ABS="$(cd qmk_firmware && pwd)"
qmk setup -H "$QMK_HOME_ABS" -y
```

既存の `.venv` が Python 3.8 以前で作られている場合は、一度削除してから上記コマンドで作り直してください。再作成後に `python -V` を実行し、3.9 以上になっていることを確認すると安心です。

## Vial のビルド
以下のコマンドで Vial 用ファームウェアをビルドできます（`keymap` には `toxaO` や `user_dual` 等の実在するキーマップ名を指定してください）。

```sh
make -C vial-qmk SKIP_GIT=yes VIAL_ENABLE=yes keyball/keyball39:toxaO
make -C vial-qmk SKIP_GIT=yes VIAL_ENABLE=yes keyball/keyball44:user_dual
# user 配布用の 9 ターゲットをまとめてビルドする場合
bash scripts/build_user_maps.sh
# (必要なキーボード/キーマップの組み合わせを指定)
```

ビルド成果物は通常 `vial-qmk/.build/` 以下に生成されます。`.uf2` ファイルを取り出して `build/` へコピーするなどして利用してください。

## トラブルシュート
- `arm-none-eabi-gcc: not found` → GNU Arm Embedded Toolchain の導入が必要です。MSYS2 では `pacman`、Ubuntu では `apt` から導入可能です。
- `ln: failed to create symbolic link` → Windows ファイルシステム上で実行している場合は WSL 側（ext4）に移動して実行してください。
- `make: *** No rule to make target` → キーボード名・キーマップ名のタイプミスを確認してください。

## 他の選択肢
- セットアップを自動化したい場合は `scripts/setup_and_build.sh` を利用すると、依存の存在確認と `vial-qmk` へのシンボリックリンク準備をワンコマンドで実行できます（ビルドは含まれません）。その後のビルドには `scripts/build_vial_all.sh`（全 toxaOターゲット）や `scripts/build_user_maps.sh`（頒布用 user ターゲット）を併用してください。
- Docker イメージによるセットアップを推奨しています。詳細は `docs/docker_setup.md` を参照してください。
