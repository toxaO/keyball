# Keyball（本ファーム概要）

<img src="images/oled_setting.jpeg" alt="keyball39" width="600"/>

## 概要
このリポジトリは Keyball シリーズ用ファームウェア（QMK+vialベース）です。<br>
[keyball公式](https://github.com/Yowkees/keyball "Yowkees/keyball")を元に、RP2040系として個人的に作成したものです。<br>
トラブル等は公式には問い合わせず、こちらのリポジトリのissueへお願いします。<br>
機能としては、vial対応、スワイプ（マウスジェスチャ）、擬似フリック入力、OLEDでの挙動調整などを追加しています。<br>

## クイックスタート（Ubuntu 20.04 / WSL2）
完全に新しい環境で検証する場合は、以下の順で進めると最短です。

```sh
sudo apt update
sudo apt install -y software-properties-common
sudo add-apt-repository universe
sudo add-apt-repository ppa:deadsnakes/ppa
sudo apt update
sudo apt install -y git build-essential python3.10 python3.10-venv python3.10-distutils python3-pip gcc-arm-none-eabi binutils-arm-none-eabi libnewlib-arm-none-eabi
git clone --recurse-submodules https://github.com/toxaO/keyball.git
cd keyball
bash scripts/setup_and_build.sh  # 初回セットアップ（ビルドは行わない）
make -C vial-qmk SKIP_GIT=yes VIAL_ENABLE=yes keyball/keyball39:user
```

`scripts/setup_and_build.sh` は `make` / `pip` / `arm-none-eabi-gcc` に加えて Python 3.9 以上を利用します。上記の `sudo apt install ...` を実行してからセットアップスクリプトを走らせてください（未導入のまま実行すると不足パッケージの警告が表示されます）。既存の `.venv` が古い Python で作られている場合は、スクリプトが自動的に削除して再作成します。

※ 仮想環境を手動で再作成したい場合は、リポジトリ直下で `rm -rf .venv && python3.10 -m venv .venv && source .venv/bin/activate && python -m pip install -U pip qmk` を実行し、`python -V` が 3.10 以上になっていることを確認してください。

※ `git clone --recurse-submodules` を忘れた場合は、クローン後に `git submodule update --init --recursive` で補完してください。

## 対応ボード（RP2040系）
- RP2040 へ載せ替えが必要です。ATmega32U4系promicro(普通のkeyballで使用しているボード)のままでは使用できません）
- 必要ボード: 下記いずれか
  - [AliExpressの本品](https://ja.aliexpress.com/item/1005005980167753.html?channel=twinner "promicro rp2040 商品リンク")
  - RP2040 ProMicro 互換（4MBで可）

## 対応手順
- 対応のボードのシルク（端子の記載）をよく確認の元、お手持ちのkeyballの仕様に合うようにコンスルーかピンヘッダを使用して取り付けてください。
- ※一度コンスルー（ピンヘッダ）をはんだ付けしてしまうと取り外すのが大変なので、取り付け向きに関してはよく確認してください。※
- 確認しているボードでは、リセットスイッチが下側（取り付けると隠れてしまう側）に来るように取り付けることになります。
- 取り付ける前に、RP2040へファームを書き込んでください。新品のrp2040ボードは、BOOTボタンを押しながらPCに接続するとストレージとして認識されるので、そこにファームウェアのファイル(.uf2ファイル)をコピーしてください。
- 一度ファームウェアを書き込み基板に取り付けた後は、keyballのリセットスイッチ（タクトスイッチ）を素早く2回押すことで、PCに書き込み可能モード（ストレージとしての認識）になります。
- またはキーコード(QK_BOOT)を使用しても良いです。
- ※取り付ける向きは画像の面が上を向くようにしてください。（リセットボタンが隠れます）※

<img src="images/rp2040promicro.jpeg" alt="rp2040promicro" width="150"/>

## !!ファームの使用の前に!!
### karabiner-elementsの導入の推奨（macユーザ向け）
- このファームをmacで使用する場合、[karabiner-elements](https://karabiner-elements.pqrs.org/ "karabiner-elements")を使用して、right-controlをFnに変更することを強く推奨します。
- QMKではmacのキーボードに存在するFnキーに対応するキーコードがありません。そのためこのファームでは右コントロールキーをFnキーとして認識するようにkarabinerで変換することを前提として一部のキーを設定しています。詳細に関してはlib_user/user/keycode_user.hやlib_user/user/feature/macro_user.cを参照してください。（基本的なKC_系に関しては手を加えていないので、気にしなくて大丈夫です）
- vial上ではRALT(KC_F*)、ビルドできる方はキーマップにキーコードF1を割り当てると、macではRCTL(KC_F1)からkarabinerの変換を得てFn(KC_F1)として動作するようになります。その他のOSではF1はKC_F1として動作します。
### mac mouse fixの導入の推奨（macユーザ向け）
- [mac mouse fix](https://macmousefix.com/ja/ "mac mouse fix")も導入することを推奨します。
- スクロール動作が滑らかになり、MS_BTN4やMS_BTN5に機能を登録することでズームなどを実現できます。
### macとその他のOSに対するショートカットキーに関して
- vialでRALT(KC_C)などを割り当てた場合、macではLGUI(KC_C)、その他ではLCTL(KC_C)として動作するようになっています。（典型的なキーコードのみの対応となっています）

## このファームで利用できる主な機能
### Vial 対応（キーマップの動的編集が可能）
- [vial web app](https://vial.rocks/ "vial web")か[vial デスクトップアプリ](https://get.vial.today/manual/ "vial 公式")をご利用ください

### スワイプ（マウスジェスチャ）操作の対応
- SW系のキーを押しながら、上下左右にスワイプすることで、アプリ切替や音量調整などが可能。
- swipe_user.cの実装を変更することで、機能を追加できます。

| キーコード | 説明                       |タップ                                |左             |下                          |上                       |右            |
|:-----------|:---------------------------|:-------------------------------------|:--------------|:---------------------------|:------------------------|:-------------|
| `SW_APP`   | アプリ切替スワイプ         |タスクビュー<br>ミッションコントロール| 右デスクトップ|タスクビュー<br>アプリビュー|copilot<br>スポットライト|左デスクトップ|
| `SW_VOL`   | 音量/メディア操作スワイプ  |再生/停止                             |前の曲         |音量上昇                    |音量減少                 |次の曲        |
| `SW_BRO`   | ブラウザ/履歴スワイプ      |アドレスバー                          |戻る           |ズームアウト（Ctrl/Cmd -）|ズームイン（Ctrl/Cmd +）|進む          |
| `SW_TAB`   | タブ切替スワイプ           | 新規タブ                             |前のタブ       |タブを閉じる                |最後に閉じたタブ         |次のタブ      |
| `SW_WIN`   | ウィンドウ位置操作         |ウィンドウを最大化                    |左半分に       |下半分に                    |上半分に                 |右半分に      |
| `SW_UTIL`  | ユーティリティスワイプ     |Esc → LANG2                           |Undo/Redo<br>(Win=Ctrl+Y, Mac=Cmd+Z, Linux=Ctrl+Z) |ペースト                    |コピー                   |Redo<br>(Win/既定=Ctrl+Y, Mac=Cmd+Shift+Z, Linux=Ctrl+Shift+Z) |
| `SW_ARR`   | 矢印キー                   |動作なし                              |左キー         |下キー                      |上キー                   |右キー        |
| `SW_EX1`   | 各自拡張用                 |F13                                   |F15            |F16                         |F14                      |F17           |
| `SW_EX2`   | 各自拡張用                 |F17                                   |F21            |F20                         |F18                      |F19           |

### 擬似フリック入力
- Flick_系のキーを押しながら、上下左右にスワイプまたはMULTI_{A, B, C}を押下することで、方向に応じた文字入力が可能。
- MULTI_Aは左、Bは右、Cの単体タップは上、Cのダブルタップは下に対応しています。

| キーコード | 説明           |タップ  |左        |下          |上        |右        |
|:-----------|:---------------|:-------|:---------|:-----------|:---------|:---------|
| `FLICK_A`  | 擬似フリックA  |a       |b         |なし        |なし      |c         |
| `FLICK_D`  | 擬似フリックD  |d       |e         |なし        |なし      |f         |
| `FLICK_G`  | 擬似フリックG  |g       |h         |なし        |なし      |c         |
| `FLICK_J`  | 擬似フリックJ  |j       |k         |なし        |なし      |c         |
| `FLICK_M`  | 擬似フリックM  |m       |l         |なし        |なし      |c         |
| `FLICK_P`  | 擬似フリックP  |p       |q         |なし        |r         |s         |
| `FLICK_T`  | 擬似フリックT  |t       |u         |なし        |なし      |v         |
| `FLICK_W`  | 擬似フリックW  |w       |x         |なし        |y         |z         |

### マルチキー入力
- どのスワイプキーを押しているかによって押した際の機能が変わります。
- multi_user.cの実装を変更することで、機能を追加できます。

| キーコード | 説明           |スワイプタグなし  |スワイプタグあり                            |
|:-----------|:---------------|:-----------------|:-------------------------------------------|
| `MULTI_A`  | マルチキーA　  |undo              |左スワイプ                                  |
| `MULTI_B`  | マルチキーB　  |redo              |右スワイプ                                  |
| `MULTI_C`  | マルチキーC　  |動作なし          |上スワイプ<br>ダブルタップで下スワイプ      |
| `MULTI_D`  | マルチキーD　  |動作なし          |動作なし                                    |

### 設定用カスタムキー
| キーコード  | 説明                    |
|:------------|:------------------------|
| `KBC_RST`   | 設定リセット            |
| `KBC_SAVE`  | 設定保存                |
| `STG_TOG`   | OLEDの設定画面を開く    |
| `SSNP_VRT`  | 垂直スクロールスナップ  |
| `SSNP_HOR`  | 水平スクロールスナップ  |
| `SSNP_FRE`  | スクロールスナップフリー|
| `SCSP_DEC`  | スクロール速度減少      |
| `SCSP_INC`  | スクロール速度増加      |
| `MOSP_DEC`  | マウスポインタ速度減少  |
| `MOSP_INC`  | マウスポインタ速度増加  |

### OLED表示
- lib_user/user/user/oled_user.cで通常画面の表示内容を変更できます。

| 関数                              | 説明                                                        |
|:----------------------------------|:------------------------------------------------------------|
|oled_render_info_layer()           |  現在のレイヤー                                               |
|oled_render_info_layer_default()   |  現在のデフォルトレイヤー                                   |
|oled_render_info_ball()            |  トラックボールの現在値                                     |
|oled_render_info_keycode()         |  送信キーコード                                             |
|oled_render_info_mods()            |  modifier keyの状態 順番にShift, Ctrl, Gui, alt             |
|oled_render_info_mods_oneshot()    |  one shot modifier keyの状態 順番にShift, Ctrl, Gui, alt    |
|oled_render_info_mods_lock()       |  modifier keyのlock状態 順番にShift, Ctrl, Gui, alt, Caps   |
|oled_render_info_cpi()             |  ポインターの速度                                           |
|oled_render_info_scroll_step()     |  スクロール速度                                             |
|oled_render_info_swipe_tag()       |  スワイプ状態                                               |
|oled_render_info_key_pos()         |  押したキーの位置                                           |

### OLED 上での設定
- SET_TOGキーを押すと、OLEDが設定モードに切り替わります。（再度押すと通常画面に戻ります。）
- 上下キーでカーソルの移動。左右キーでページ移動。Shift+左右キーで値の増減が可能です。
- 設定はKBC_SAVEキーで保存されます。KBC_RSTキーで初期化されます。
- ご自分でビルドされる方は、lib/keyball/keyball.hで設定されているマクロをkeyball/.../keymap/user/config.hで上書きすることで初期値を変更できます。

### mouse config
- MoSp: (Mouse speed)ポインタの速度
- Glo: (Gain low speed)低速域のゲイン。低速域ではポインタの速度がここで指定した割合になります。
- Th1/Th2: (Threshold1/2)低速域のしきい値。Th1以下ではMoSpの(Glo)%、Th1～Th2では線形補間されたゲイン、Th2以上ではMoSpの速度になります。
- DZ: (DEAD ZONE)この値以下のボールの動作は無視されます。

### AML config
- en: (enable) オートマウスレイヤーの有効/無効
- TO: (Time Out)設定の時間(ms)が経過するまでマウスレイヤーに留まります。500ms単位で増減。9500より大きく設定しようとすると、HOLDとして60秒間の設定となります。
- AMLはマウスキー以外のボタンを押すことで解除されます。（マウスキー：MS_BTN1やスワイプキー。util_user.cで追加設定できます。）
- TH: AUTO MOUSE LAYERの閾値。ポインタの移動量がこれを超えるとAMLが有効になります。
- TG_L: (Target Layer)AMLが有効になったときの切り替え先レイヤー番号。

### Scroll config
- Sp: (Scroll Speed)スクロールのステップ。1～7で設定可能。
- Dz: (DEAD ZONE)この値以下のボールの動作は無視されます。
- Inv: (Invert)スクロールの反転。
- ScLy: (Scroll Layer)任意のレイヤーでスクロールモードをを有効にする。
- LNo: (Layer number)スクロールモードを有効にするレイヤー番号。
- Mode: (Mode)モード。m:macモード。f:高精度モード。n:標準モード。macでは高精度モードを使用できません。
- H_Ga: (Horizontal Gain)スクロール最終ゲインの水平成分。垂直成分に対する割合(%)。左右方向のスクロールを遅くしたい場合に調整します。

### SSNP config
- Mode: ver/ hor/ free。一方向へのスクロールを優先し、一定以上の量を超えた場合にフリースクロールになります。freeは常にフリースクロールです。
- Thr:  (Threshold) ver/horモードでフリースクロールに切り替わる閾値。
- Rst: フリースクロールでいる時間。

### Swipe config
- St: (Swipe Threshold) スワイプと判定する移動量の閾値
- Dz: (Dead Zone) この値以下のボールの動作は無視されます。
- Rt: (Reset Time) スワイプ動作のリセット時間(ms)。この時間内に閾値を超える動作がなければ、スワイプ動作用の移動量はリセットされます。
- Frz: (Freeze) スワイプ中にポインタを固定するかどうか。

### RGB config
- light on/off（offの時は以下の設定を変更できません）
- HUE (色相)。
- SAT (彩度)。
- VAL (明度)。
- Mode アニメーションモード。(一部モードではHUE, SAT, VALは変更できません)

### layer config
- def: (Default Layer) デフォルトレイヤーとするレイヤー番号。


## 詳細ドキュメント
- APIや詳細仕様は `keyball/lib/keyball/README.md` を参照。

## セットアップとビルド
- シェルスクリプトによる導入（MSYS2/macOS/Ubuntu想定）
  - `bash scripts/setup_and_build.sh`
  - 依存確認→qmk CLI venv→シンボリックリンク作成→QMK & Vial ビルドまで自動実行
- GitHub Actions でのビルド
  - Actions → "Keyball manual build (QMK/Vial)"
  - 入力フォームで kb（例: keyball/keyball61）, km（例: mymap）, impl（qmk/vial）を選択
- Vial 上でキーマップの編集が可能です（Vial対応ビルドを使用）。

## ビルド済み生成物
- `build/` ディレクトリにビルド済みファームが入っています（必要に応じて更新）。

## 問い合わせ
- 公式ではなく、こちらへお願いします: https://x.com/toxa_craft
- issueでも可能。
