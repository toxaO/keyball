# Keyball ビルド用 Docker イメージ
FROM debian:bookworm-slim

ENV DEBIAN_FRONTEND=noninteractive
ENV PIP_BREAK_SYSTEM_PACKAGES=1

RUN apt-get update \
 && apt-get install -y --no-install-recommends \
      build-essential \
      git \
      make \
      python3 \
      python3-pip \
      python3-venv \
      gcc-arm-none-eabi \
      binutils-arm-none-eabi \
      libnewlib-arm-none-eabi \
      ca-certificates \
 && apt-get clean \
 && rm -rf /var/lib/apt/lists/*

WORKDIR /work

# デフォルトで bash を起動してリポジトリを操作できるようにする
CMD ["bash"]
