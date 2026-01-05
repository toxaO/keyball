# Docker を利用した Keyball ビルド環境

## 概要
Docker コンテナ内で依存をまとめて管理することで、ホスト環境に余計なパッケージを導入することなく Vial/QMK のビルドを再現できます。`docker/Dockerfile` には必要なツールチェーンが含まれており、ホストからはリポジトリをマウントするだけで利用可能です。

## 事前準備
- Docker Engine (Linux) または Docker Desktop (macOS/Windows + WSL2) をインストールしてください。
- 本リポジトリをローカルにクローンしておきます。

## イメージの作成
リポジトリ直下で以下を実行します。

```sh
docker build -t keyball-build -f docker/Dockerfile .
```

既存イメージを作り直す場合は `--no-cache` を付けて `docker build --no-cache -t keyball-build -f docker/Dockerfile .` のように実行すると更新内容が確実に反映されます。

## コンテナの起動
作業ディレクトリを `/work` にマウントし、bash セッションを開きます。

```sh
docker run --rm -it -v "$(pwd)":/work keyball-build
```

Windows PowerShell から実行する場合は `$(pwd)` を `${PWD}` に、MSYS2 では `$(pwd)` を ``pwd`` に置き換えてください。

## コンテナ内での作業例
```sh
# リポジトリ直下に移動済みの想定
./scripts/build_vial_all.sh              # すべての Vial .uf2 を build/ に集約
# 頒布用 user ターゲットを一括で作る場合
./scripts/build_user_maps.sh
# もしくは個別ビルド
make -C vial-qmk SKIP_GIT=yes VIAL_ENABLE=yes keyball/keyball39:toxaO
```

生成されたファイルはマウントした `build/` ディレクトリに保存され、ホスト側から直接参照できます。

## 注意事項
- コンテナは USB デバイスに直接アクセスできないため、ファーム書き込みまではホスト側で実施してください。必要に応じて `--device` オプションでマウントすることも可能です。
- GitHub Actions など CI/CD での再利用を想定する場合は `docker/Dockerfile` をベースにカスタマイズしてください。
- 既存の QMK 環境を持っているユーザーは Docker を使わずともホストで `make` を実行できます。詳細は `README.md` や `docs/manual_setup.md` を参照してください。
- Debian/Ubuntu 系では `pip` によるシステム領域の書き込みが制限されることがあります。本 Dockerfile では `PIP_BREAK_SYSTEM_PACKAGES=1` を設定し、`./scripts/setup_and_build.sh` 実行時に `python3 -m pip install` が止まらないよう調整してあります。また、`libnewlib-arm-none-eabi` を同梱しているので `assert.h` など新lib 系ヘッダ不足でビルドが止まる心配はありません。
