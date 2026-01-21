# Keyball（本ファーム概要）

<img src="images/oled_setting.jpeg" alt="keyball39" width="600"/>

## 概要
このリポジトリは Keyball シリーズ用ファームウェア（QMK+vialベース）です。<br>
[keyball公式](https://github.com/Yowkees/keyball "Yowkees/keyball")を元に、RP2040系として個人的に作成したものです。<br>
トラブル等は公式には問い合わせず、こちらのリポジトリのissueへお願いします。<br>
機能としては、vial対応、スワイプ（マウスジェスチャ）、振動モーター対応、擬似フリック入力、OLEDでの挙動調整などを追加しています。<br>
ドキュメントやソースコードに関しては生成AIを使用している箇所が多々あります。確認はしておりますが、誤りや不備がある場合は伝えていただけると幸いです。

## クイックスタート（Ubuntu 20.04 / WSL2）
```sh
sudo apt update
sudo apt install -y software-properties-common
sudo add-apt-repository universe
sudo add-apt-repository ppa:deadsnakes/ppa
sudo apt update
sudo apt install -y git build-essential gcc-arm-none-eabi binutils-arm-none-eabi libnewlib-arm-none-eabi
git clone --recurse-submodules https://github.com/toxaO/keyball.git
cd keyball
bash scripts/setup_and_build.sh  # 初回セットアップ（ビルドは行わない）
# user版は筐体別・レイアウト別に `user_dual` / `user_left` / `user_right` を指定してください
make -C vial-qmk SKIP_GIT=yes VIAL_ENABLE=yes keyball/keyball39:user_dual
# すべての user レイアウトをまとめてビルドする場合
bash scripts/build_user_maps.sh
```

`scripts/setup_and_build.sh` は `make` / `arm-none-eabi-gcc` の存在確認と、`vial-qmk` へのシンボリックリンク作成のみを行います（ビルドは行いません）。未導入のまま実行すると不足パッケージの警告が表示されます。

※ `git clone --recurse-submodules` を忘れた場合は、クローン後に `git submodule update --init --recursive` で補完してください。
※ Docker でのセットアップを推奨しています。詳細は `docs/docker_setup.md` を参照してください。

## リポジトリ構造
- `keyball` … キーボード本体の実装
- `keyball/lib` … キーボード共有（KBレベル）のライブラリ
- `keyball/lib_user` … ユーザーレベルの実装
  - `keyball/lib_user/user` … 頒布用キーマップ（誰でも編集可）
  - `keyball/lib_user/toxaO` … toxaO 用の個人設定（サンプル）
- `qmk_firmware` … 公式 QMK へのリンク用ツリー
- `vial-qmk` … Vial 対応 QMK ツリー
- `scripts` … セットアップスクリプト群
- `build` … ビルド済み .uf2 の置き場
- `docs/releases` … バージョン別のリリースノート

## キーマップとビルドバリアント
- 頒布用キーマップは `keyball/lib_user/user` に配置されています。自身の設定を共有する場合はここを編集し、筐体/配列ごとに `keyball/keyball{39,44,61}:user_{dual,left,right}` を指定してビルドしてください。
- `keyball/lib_user/toxaO` は toxaO の常用設定です。挙動のサンプルとして参考にしてください。
- `build/` 配下には頒布用の最新 .uf2 を随時更新して格納します。

## 対応ボード（RP2040系）
- RP2040 へ載せ替えが必要です。ATmega32U4系promicro(普通のkeyballで使用しているボード)のままでは使用できません）
- 必要ボード: 下記いずれか
  - [AliExpressの本品](https://ja.aliexpress.com/item/1005005980167753.html?channel=twinner "promicro rp2040 商品リンク")
  - RP2040 ProMicro 互換（4MBで可）

### ハプティックドライバ（DRV2605L）(オプション)
- I2C 接続の DRV2605L モジュール（例: Adafruit DRV2605L Haptic Driver）+ LRA/ERM モーターを左右に搭載すると振動フィードバックを利用できます。
- 動作確認パーツ
    - [DRV2605Lモジュール](https://amzn.asia/d/evmf2u0 "Amazon DRV2605L"))
    - [LRAモーター](https://amzn.asia/d/5kwE7qT "Amazon LRAモーター")
- 任意のケーブルでRP2040 側の SDA/SCL/3V/GND を DRV2605L に接続し、必要に応じて ENABLE ピンをプルアップしてください（私はプルアップせずに使用していますが問題ないようです）。
- OLED 設定の「SW_Hp conf」「Ly_Hp conf」「AML haptic」各ページから、スワイプ/レイヤー/AML 遷移時の振動モード番号や有効・無効をで調整できます。設定値は `KBC_SAVE` で EEPROM（kbpf）へ保存され、PC 再接続後も保持されます。

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
| `SW_VOL`   | 音量/メディア操作スワイプ  |再生/停止                             |次の曲         |音量上昇                    |音量減少                 |前の曲        |
| `SW_BRO`   | ブラウザ/履歴スワイプ      |アドレスバー                          |戻る           |ズームアウト（Win/Linux=Ctrl+-、macOS=Cmd+-）|ズームイン（Win/Linux=Ctrl+Shift+=、macOS=Cmd+;）|進む          |
| `SW_TAB`   | タブ切替スワイプ           | 新規タブ                             |前のタブ       |タブを閉じる                |最後に閉じたタブ         |次のタブ      |
| `SW_WIN`   | ウィンドウ位置操作         |Snapメニュー / Fn+Ctrl+F（Win=Win+Z） |左半分（Win+← / Fn+Ctrl+←） |下半分/最小化（Win+↓ / Fn+Ctrl+↓） |最大化（Win+↑ / Fn+Ctrl+↑） |右半分（Win+→ / Fn+Ctrl+→） |
| `SW_UTIL`  | ユーティリティスワイプ     |Esc → LANG2                           |Undo（Win/Linux=Ctrl+Z, macOS=Cmd+Z） |ペースト                    |コピー                   |Redo（Win=Ctrl+Y, macOS=Cmd+Shift+Z, Linux=Ctrl+Shift+Z） |
| `SW_ARR`   | 矢印キー                   |動作なし                              |左キー         |下キー                      |上キー                   |右キー        |
| `SW_EX1`   | 各自拡張用                 |F13                                   |F14            |F15                         |F16                      |F17           |
| `SW_EX2`   | 各自拡張用                 |F18                                   |F19            |F20                         |F21                      |F22           |

### ハプティックフィードバック
- `HAPTIC_DRIVER = drv2605l` により、スワイプ／AML／レイヤー切り替え時に DRV2605L 経由で左右別の振動を再生できます。
- OLED の `SW_Hp conf` ページではスワイプの初回/2回目以降の振動モードや連続振動認識リセット時間（Idle）、テスト再生を操作できます。(初回は強く、連続で動作させる2回目以降は弱くするなどの使用を想定した機能です)
- `Ly_Hp conf` ページでは各レイヤーごとに左・右の振動の有効/無効とモード番号を設定可能です。
- `AML haptic` ページでは Auto Mouse Layer の遷移時(in/out)で振動するモードとon/offを設定できます。
### 擬似フリック入力
- Flick_系のキーを押しながら、上下左右にスワイプまたはMULTI_{A, B, C}を押下することで、方向に応じた文字入力が可能。
- MULTI_Aは左、Bは右、Cの単体タップは上、Cのダブルタップは下に対応しています。

| キーコード | 説明           |タップ  |左        |下          |上        |右        |
|:-----------|:---------------|:-------|:---------|:-----------|:---------|:---------|
| `FLICK_A`  | 擬似フリックA  |a       |b         |なし        |@         |c         |
| `FLICK_D`  | 擬似フリックD  |d       |e         |)           |(         |f         |
| `FLICK_G`  | 擬似フリックG  |g       |h         |なし        |なし      |i         |
| `FLICK_J`  | 擬似フリックJ  |j       |k         |なし        |なし      |l         |
| `FLICK_M`  | 擬似フリックM  |m       |n         |なし        |なし      |o         |
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
- 設定は**KBC_SAVEキーで保存**されます。KBC_RSTキーで初期化されます。**OS毎に設定が保存されます。**
- ご自分でビルドされる方は、lib/keyball/keyball.hで設定されているマクロをkeyball/.../keymap/user/config.hで上書きすることで初期値を変更できます。
- ページ構成は以下の通りです（RGBLIGHT/HAPTICが無効な場合は該当ページが省略されます）。

| No. | 画面ラベル         | 主な項目                                                                 |
|:----|:-------------------|:-------------------------------------------------------------------------|
| 1   | `mouse conf`       | Sp/GaL/Th1/Th2/DZ（ポインタ移動量）                                     |
| 2   | `AML conf`         | en/TO/TH/TG（Auto Mouse Layer 設定）                                      |
| 3   | `AML haptic`       | IN/INf/OUT/OUTf（AML入退場時の振動）                                     |
| 4   | `Scrl conf`        | Sp/Dz/Iv/ScLy/LNo/Md/H_Ga（スクロール全般）                                |
| 5   | `SSNP conf`        | Mode/Thr/Rst（スクロールスナップ）                                        |
| 6   | `Scrl moni`        | スクロール生値・内部状態モニタ                                            |
| 7   | `Swipe conf`       | Th/Dz/Rt/Frz（スワイプ判定パラメータ）                                    |
| 8   | `Swipe moni`       | アクティブ状態・方向・累積値                                               |
| 9   | `RGB conf`         | light/HUE/SAT/VAL/Mode（RGBライト）                                       |
| 10  | `SW_Hp conf`       | En/1st/2nd~/Idle/Test（スワイプ時ハプティック）                           |
| 11  | `Ly_Hp conf`       | Layer/左右有効/左右エフェクト（レイヤー別ハプティック）                  |
| 12  | `LED moni`         | LEDインデックスモニタ                                                      |
| 13  | `layer conf`       | def（デフォルトレイヤー）                                                 |
| 14  | `Send moni`        | 直近に送出したキー/レイヤ/Mods/位置                                        |

### mouse config（ページ1）
- Sp: (Mouse speed)ポインタの速度
- GaL: (Gain low speed)低速域のゲイン。低速域ではポインタの速度がここで指定した割合になります。
- Th1/Th2: (Threshold1/2)低速域のしきい値。Th1以下ではMoSpの(Glo)%、Th1～Th2では線形補間されたゲイン、Th2以上ではMoSpの速度になります。
- DZ: (DEAD ZONE)この値以下のボールの動作は無視されます。

### AML config（ページ2）
- en: (enable) オートマウスレイヤーの有効/無効
- TO: (Time Out)設定の時間(ms)が経過するまでマウスレイヤーに留まります。500ms単位で増減。9500より大きく設定しようとすると、HOLDとして60秒間の設定となります。
- AMLはマウスキー以外のボタンを押すことで解除されます。（マウスキー：MS_BTN1やスワイプキー。util_user.cで追加設定できます。）
- TH: AUTO MOUSE LAYERの閾値。ポインタの移動量がこれを超えるとAMLが有効になります。
- TG_L: (Target Layer)AMLが有効になったときの切り替え先レイヤー番号。

### AML haptic config（ページ3）
- IN/OUT: Auto Mouse Layer の遷移/解除で振動させるかどうか。
- INf/OUTf: それぞれに割り当てる DRV2605L のエフェクト番号（1〜123程度）。

### Scroll config（ページ4）
- Sp: (Scroll Speed)スクロールのステップ。1～7で設定可能。
- Dz: (DEAD ZONE)この値以下のボールの動作は無視されます。
- Iv: (Invert)スクロールの反転フラグ（OS別）。
- ScLy/LNo: 任意のレイヤーでスクロールモードを常時有効化するトグルとレイヤー番号。
- Md: (Mode) m:macモード、f:高精度モード、n:標準モード。macでは高精度モードを使用できません。
- H_Ga: (Horizontal Gain)スクロール最終ゲインの水平成分。垂直成分に対する割合(%)。左右方向のスクロールを遅くしたい場合に調整します。

### SSNP config（ページ5）
- Mode: ver/ hor/ free。一方向へのスクロールを優先し、一定以上の量を超えた場合にフリースクロールになります。freeは常にフリースクロールです。
- Thr:  (Threshold) ver/horモードでフリースクロールに切り替わる閾値。
- Rst: フリースクロールでいる時間。

### Scroll monitor（ページ6）
- スクロール変換前後の生値（x/y/h/v）と内部加速度（ah/av）、テンション値(t)をリアルタイムで表示します。チューニング時の目視用です。

### Swipe config（ページ7）
- Th: (Swipe Threshold) スワイプと判定する移動量の閾値
- Dz: (Dead Zone) この値以下のボールの動作は無視されます。
- Rt: (Reset Time) スワイプ動作のリセット時間(ms)。この時間内に閾値を超える動作がなければ、スワイプ動作用の移動量はリセットされます。
- Frz: (Freeze) スワイプ中にポインタを固定するかどうか。

### Swipe monitor（ページ8）
- アクティブ状態（Ac）、現在のタグ（Md）、方向（Dir）、各方向の蓄積値（U/D/R/L）を確認できます。誤判定時のデバッグに利用してください。

### RGB config（ページ9、RGBLIGHT_ENABLE 時のみ）
- light on/off（offの時は以下の設定を変更できません）
- HUE (色相)。
- SAT (彩度)。
- VAL (明度)。
- Mode アニメーションモード。(一部モードではHUE, SAT, VALは変更できません)

### Swipe haptic config（ページ10、HAPTIC_ENABLE 時のみ）
- En: スワイプ時のハプティックを有効／無効。
- 1st: 1方向目のエフェクト番号。
- 2nd~: 連続スワイプ時に使用するエフェクト番号。
- Idle: 連続スワイプとして認識される時間。
- Test: カーソルを合わせて Shift+左右 でエフェクトをテスト再生。

### Layer haptic config（ページ11、HAPTIC_ENABLE 時のみ）
- Ly: 設定対象のレイヤー番号。
- L/R: 左右の振動を個別に ON/OFF。
- Mode: それぞれのエフェクト番号。

### LED monitor（ページ12）
- 現在の LED インデックスを表示し、該当 LED のみを青色で点灯させます。配線チェックや個別点灯確認に利用できます。
  `keyball_led_set_hsv_at()` / `keyball_led_set_hsv_range()` を使用すると自動的に RPC 同期され、slave側のLEDも点灯させられます。
  ```c
  #include "lib/keyball/keyball_led.h"

  void indicate_layer_change(void) {
      keyball_led_set_hsv_at(HSV_WHITE, 10);        // LED10 を白に
      keyball_led_set_hsv_range(HSV_RED, 20, 3);    // LED20～22 を赤に
  }
  ```
  これらの関数は内部で `rgblight_sethsv_*` を呼びつつ、分割構成では `KEYBALL_LED_SYNC` RPC により反対側へも同じ指示を送ります。より細かい制御を行いたい場合は `keyball_led_monitor.c` の実装を参考に独自の同期処理を追加してください。

### Default layer config（ページ13）
- def: (Default Layer) デフォルトレイヤーとするレイヤー番号。`default_layer_set` と連動します。

### Send monitor（ページ14）
- 直近に送出したキーコード・レイヤー・(row,col)・修飾キー状態を表示します。キースキャンの確認やデバッグに利用できます。


## 詳細ドキュメント
- APIや詳細仕様は `keyball/lib/keyball/README.md` を参照。

## セットアップとビルド
- シェルスクリプトによる導入（MSYS2/macOS/Ubuntu想定）
  - `bash scripts/setup_and_build.sh`
  - 依存確認→`vial-qmk` へのシンボリックリンク作成（ビルドは行いません）
- Vial 上でキーマップの編集が可能です（Vial対応ビルドを使用）。
- Docker でのセットアップを推奨しています。詳細は `docs/docker_setup.md` を参照してください。

## ビルド済み生成物
- `build/` ディレクトリにビルド済みファームが入っています（必要に応じて更新）。

## 問い合わせ
- 公式ではなく、こちらへお願いします: https://x.com/toxa_craft
- issueでも可能。
