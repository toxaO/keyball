# Keyball Core Function Library

## Feature Overview / 機能概要

### Pointer movement adjustment / ポインタ移動量調整
Use the OLED setting view to adjust low-speed gain (Glo) and threshold (Th1)
for precise pointer control.

ポインタ移動の低速ゲイン（Glo）やしきい値（Th1）は、OLED の設定ページで調整できます。

### Scroll adjustment / スクロール調整
Adjust scroll sensitivity, inversion, and presets in the OLED setting view.
Quick toggles are available:
- `SCRL_TO`: Toggle scroll mode
- `SCRL_MO`: Enable scroll mode while pressed

スクロールの感度・反転・プリセットは OLED 設定で行います。クイック操作として次を用意しています。
- `SCRL_TO`: スクロールモードのトグル
- `SCRL_MO`: 押下中のみスクロールモード有効

### Swipe adjustment / スワイプ調整
Swipe thresholds, deadzone, reset, and freeze are adjusted in the OLED setting
view.

スワイプの閾値・デッドゾーン・リセット・フリーズは OLED 設定で調整できます。

**Example / 例:**

```c
// Start swipe mode while holding a key
case KC_SWIPE:
  if (record->event.pressed) {
    keyball_swipe_begin(0);
  } else {
    keyball_swipe_end();
  }
  return false;
```

```c
// React to swipe events
void keyball_on_swipe_fire(kb_swipe_tag_t tag, kb_swipe_dir_t dir) {
  if (dir == KB_SWIPE_UP) {
    tap_code(KC_VOLU);
  } else if (dir == KB_SWIPE_DOWN) {
    tap_code(KC_VOLD);
  }
}
```

### OLED setting mode / OLED設定モード（旧デバッグ）
`STG_TOG` で設定ビューを表示し、`STG_NP` / `STG_PP` でページ送り・戻しを行います。
Shift+←/→ でもページを移動できます（設定モード中）。

## API Reference / APIリファレンス

### Swipe API / スワイプAPI
- `keyball_swipe_begin(tag)` / `keyball_swipe_end()`
- `keyball_on_swipe_fire(tag, dir)` (weak callback)
- `keyball_swipe_set_step`, `keyball_swipe_set_deadzone`,
  `keyball_swipe_set_reset_ms`, `keyball_swipe_toggle_freeze`

## Swipe Integration Guide / スワイプ導入手順（詳細）

ここではユーザーレベル（`keyball/lib_user` 配下）におけるスワイプ導入の実例と注意点を、`BRO_SW`（旧 `BROWSE_SWIPE`）を例に詳しく説明します。

### 前提
- スワイプは「押下で開始（`keyball_swipe_begin(tag)`）し、解放で終了（`keyball_swipe_end()`）」のペアで運用します。
- 方向判定・しきい値はKBレベルで行い、発火時の実アクションは `keyball_on_swipe_fire(tag, dir)`（weak）で実装します。
- 分割環境では発火（アクション送出）はマスター側で行われます（実装済）。

### 1) キーコードの用意
- 任意のユーザーキーコード（例: `BRO_SW`）を `keyball/lib_user/keycode_user.h` の enum に定義します。
  - 本リポジトリでは既に `BRO_SW` が定義済みです。

### 2) キーマップにスワイプキーを配置
- 例：マウスレイヤ `_mMou/_wMou` に `BRO_SW` を割り当てます。
  - 下位レイヤの Mod-Tap 影響を避けるため、押下中だけ「全キーXXX」の `_SW_Block` レイヤをONにする方法を推奨します（解放でOFF）。

### 3) 押下/解放で begin/end を呼ぶ（単押し動作も）
- `keyball/lib_user/features/macro_key.c`（process_record_user）に以下のような分岐を追加します。

```c
case BRO_SW:
  if (record->event.pressed) {
    // 下位レイヤ無効化（SW_Block）
    layer_on(_SW_Block);
    // スワイプ開始（任意のタグ: 例ではKBS_TAG_BRO）
    keyball_swipe_begin(KBS_TAG_BRO);
    swipe_timer = timer_read();
  } else {
    // スワイプ終了
    keyball_swipe_end();
    layer_off(_SW_Block);
    // 単押し（タップ）扱い: TAPPING_TERM内かつ未発火なら代替アクション
    if (timer_elapsed(swipe_timer) < TAPPING_TERM && !keyball_swipe_fired_since_begin()) {
      // 例：ブラウザのアドレスバー呼び出し（OS別）
      tap_code16_os(C(KC_L), G(KC_L), G(KC_L), KC_NO, KC_NO);
    }
  }
  return false; // 以降の処理へ流さない
```

ポイント:
- 「単押し対応」は「押下〜解放の時間が短くて、かつスワイプ発火がなかった場合」に代替のタップを送る方式です。
- `_SW_Block` レイヤを併用することで、押下中に下位レイヤ（Mod-Tap等）の影響を遮断できます。

### 4) 方向ごとのアクションを実装
- `keyball/lib_user/features/swipe_user.c` に `keyball_on_swipe_fire(tag, dir)` を実装します。

```c
void keyball_on_swipe_fire(kb_swipe_tag_t tag, kb_swipe_dir_t dir) {
  switch (tag) {
  case KBS_TAG_BRO:
    switch (dir) {
    case KB_SWIPE_UP:    tap_code16_os(C(KC_C), G(KC_C), G(KC_C), KC_NO, KC_NO); break;
    case KB_SWIPE_DOWN:  tap_code16_os(C(KC_V), G(KC_V), G(KC_V), KC_NO, KC_NO); break;
    case KB_SWIPE_LEFT:  tap_code16_os(KC_WBAK, G(KC_LEFT),  G(KC_LEFT),  KC_NO, KC_NO); break;
    case KB_SWIPE_RIGHT: tap_code16_os(KC_WFWD, G(KC_RIGHT), G(KC_RIGHT), KC_NO, KC_NO); break;
    default: break;
    }
    break;
  default:
    break;
  }
}

// 終了時の後始末（必要に応じて）
void keyball_on_swipe_end(kb_swipe_tag_t tag) {
  layer_off(_SW_Block); // SW_Block レイヤが残らないように保険
}
```

### 5) 設定・チューニング（旧デバッグ）
- OLED設定（`STG_TOG`）のページで直感的に監視・調整できます。
  - Mouse / AML / Scroll Param / Scroll Snap / Scroll Raw / Swipe Config / Swipe Monitor
- コンソール（`qmk console`）の利用：
  - `SWIPE FIRE tag=.. dir=..` で発火状況
  - 任意の `dprintf()` を追加して分岐確認

### 6) よくある注意点
- 下位レイヤに Mod-Tap がある位置にスワイプキーを置く場合は `_Lock` レイヤ方式を使うと安全です。
- AML の timeout が短いと押下/解放のレイヤ解決がズレやすくなります。現在の調整範囲は 300–3000ms（50ms刻み）です。

### 7) 具体例: canceller（macのSpotlight/App View対応）

意図:
- macOS で、トラックパッドのジェスチャ方向（上/下）に合わせて Spotlight（上）や App View（下）を呼び出し、もう一度同じ操作方向でキャンセル（ESC）させる体験をキーボードでも再現することを目的に実装。

実装のポイント（例: `keyball/lib_user/features/swipe_user.c` の `KBS_TAG_APP`）:
- UP/DOWN の発火で Spotlight/App View 相当のショートカットを `tap_code16_os()` で送出。
- 直前に同機能を発火した“キャンセル状態”を `canceller` で保持し、同方向の次回発火で `ESC` を送って閉じる。

補足:
- `canceller` はユーザ管理のフラグなので、実装側の任意のタイミング（発火直後など）で `true/false` を切り替えます。

### 8) 具体例: Windows の Winスワイプ（ウィンドウスナップ）

意図:
- Windows では `Win + 矢印` により、ウィンドウのスナップ/最大化/最小化等を行えます。任意の状態へ移動するまで Win を押しっぱなしにして矢印を複数回送る必要があるため、スワイプ押下中は Win を維持し、離した時に解除する実装にしています。

実装のポイント（例: `keyball/lib_user/features/swipe_user.c` の `KBS_TAG_WIN`）:
- 発火時は `register_code(KC_LGUI); tap_code(KC_↑/↓/←/→);` を送る。
- 解放時（`keyball_on_swipe_end()` など）に `unregister_code(KC_LGUI)` でWinを確実に解除。
- 押下中のみ `_mMou/_wMou` や `_SW_Block` を併用して、下位レイヤからの影響を抑制するのが安全です。

### 9) 補足: tap_code16_os について

`tap_code16_os(win, mac, ios, linux, unsure)` は OS 検出結果（`host_os = detected_host_os()`）に応じて、OSごとのキーストロークを選択して送出するユーティリティです（`keyball/lib_user/features/util.c`）。
- 例: `tap_code16_os(C(KC_L), G(KC_L), G(KC_L), KC_NO, KC_NO)` は
  - Windows: `Ctrl + L`、macOS/iOS: `Cmd + L`、Linux: 無効（KC_NO）
- OSごとにショートカットが異なる場面で、コード分岐を簡潔にできます。


### Scroll API / スクロールAPI
- `keyball_set_scroll_mode`, `keyball_get_scroll_mode`
- `keyball_set_scroll_div`, `keyball_get_scroll_div`
- Variables `g_scroll_deadzone`, `g_scroll_hysteresis`

### Pointer API / ポインタAPI
- Variables `g_move_gain_lo_fp`, `g_move_th1`
- `keyball_on_apply_motion_to_mouse_move()` hook

### OLED API / OLED API
- `keyball_oled_mode_toggle`, `keyball_oled_next_page`, `keyball_oled_prev_page`
- `keyball_oled_render_setting`, `keyball_oled_render_ballinfo` など

## Scroll snap mode

When scrolling with the trackball, the scroll direction is restricted.
This restriction is called "scroll snap".

The direction of restriction is configured in the OLED setting pages.
It is called as "scroll snap mode"
The current mode is displayed on the OLED.

There are 3 modes for scroll snap.

1. Vertical (default): key code is `SSNP_VRT`, indicated as `VT`.
2. Horizontal: key code is `SSNP_HOR`, indicated as `HO`.
3. Free: key code is `SSNP_FRE`, indicated as `SCR`.

The scroll snap mode at startup is vertical,
but you can change it by saving the current mode with `KBC_SAVE`

## MEMO

This section contains notes regarding the specifications of this library.
Since the purpose is to keep a record in whatever form it takes,
a lot of Japanese is included.
If you would like to read it in English, please request a translation via issue or discussion.
Of course you can translate it for us. If you translate it,
please make pull requests to share it us.

### Scroll Snap Spec

この機能は config.h に `#define KEYBALL_SCROLLSNAP_ENABLE 0` を書き加えることで無効化できる。

トラックボールによるスクロールの方向を制限するのがスクロールスナップ。
現在のスクロールスナップには3つのモードがある。

* 垂直方向にスナップ (デフォルト)
* 水平方向にスナップ
* スナップしない自由スクロール

以上を `SSNP_VRT`, `SSNP_HOR`, `SSNP_FRE` の独自キーコードを用いて手動で切り替える。

#### up to 1.3.2

初期状態でトラックボールによるスクロールを垂直方向に制限(スナップ)している。
この振る舞いは config.h に `#define KEYBALL_SCROLLSNAP_ENABLE 1` を書き加えることで有効化できる。

この機能はスナップモードとフリーモードから構成される。
初期状態はスナップモードで、このモードではスクロール方向は垂直に制限される。
スナップモードで水平の一定方向に一定カウント(デフォルトは12)以上スクロールするとフリーモードに遷移する。
なおこのカウントはスクロール除数適用後のカウントである。
フリーモードでは制限が取り払われ、水平と垂直どちらにも自由にスクロールできる。
フリーモードで一定時間(デフォルトは 100 ミリ秒)、スクロール操作を行わないとスナップモードに遷移する。

フリーモードに遷移するためのカウント数を変更するには `KEYBALL_SCROLLSNAP_TENSION_THRESHOLD` を、
スナップモードに遷移するためのインターバル(ミリ秒)を変更するには `KEYBALL_SCROLLSNAP_RESET_TIMER` を、それぞれ config.h で設定できる。

以下はカウント数を 5 に、インターバルを 200 ミリ秒に変更する例:

```c
#define KEYBALL_SCROLLSNAP_TENSION_THRESHOLD 5
#define KEYBALL_SCROLLSNAP_RESET_TIMER 200
```

#### History of Scroll Snap

もともとは自由にスクロールできるようにしていた。
しかし思ったよりもボールの感度が高く一定方向だけに動かすのが難しく、
誤操作を誘発していた。
そのためなんらかのスナップ機能が必要だと判断した。

最初のスナップ機能は垂直・水平のどちらかの軸から一定角度以内で収まってるうちはそちらへスナップするとした。
しかし回転開始初期にはその移動量が極小かつセンサーの感度が高いので、
垂直に動かしたい時に水平にも極小量の移動が発生しておりスナップ方向が定まらない、
という問題が発生した。
人間は自分が思うほどには指を正確に動かせていなかった。

そこで一定方向に一定以上のカウントを検出するまでは一切スクロールしないようにした。
これは回転開始初期のスクロール量を読み捨てるに等しい。

しかしWebブラウザを思い浮かべてもらえればすぐにわかるように、
一般のユースケースでは垂直方向のスクロールを頻繁に利用する。
先の読み捨てにより、垂直方向のスクロールがワンテンポ遅れ、体験を大幅に損なうことが明らかになった。
この解決のためモード: 初期は垂直のみ、後に自由スクロールする、を導入した。

### Scroll Step (ST) / スクロールステップ

Keyballのセンサーは感度がとても高い。
そのため生の値をスクロール量としてしまうとスクロール操作がとても難しくなった。
そこで生の値をある数で割ってスクロールに適用する方式を採用した。
この時の割る数をスクロール除数と言っている。

スクロール除数は、体感として小さく制御する意味がなかったので、
1, 2, 4, 8, 16, 32, 64 というように2の乗数とした。
2の乗数であるのならば値の表現として $2 ^ n$ の $n$ で表せる。
またEEPROMに設定値を保存できるようにするために、
ビット数を節約する目的で $n$ が取りうる値は 1~7 の範囲とした。
結果実際の割る数は以下の式で計算できる。

$$ 2 ^ {(n - 1)} $$

$n$ の初期値は 4 で 1/8 になることを意味する。
この値は config.h で `KEYBALL_SCROLL_STEP_DEFAULT` マクロを定義することで変更できる。
これを0にすることは考慮していないので設定しないこと。

### Scroll Inhivitor

トラックボールの移動量をポインタに適用するかスクロールに適用するか、
Keyballは内部にスクロールモードという名のモードで管理している。
スクロールモードはキーコードやAPI呼び出しの任意のタイミングで切り替えられる。
デフォルトのキーマップでは特定のレイヤーの状態を
スクロールモードのオンオフに適用している。

当初はスクロールモードの切り替え直後に、
トラックボールの移動が意図しない適用先に適用されることが頻発した。
ポインタをブラウザまで動かした後にスクロールモードに変更すると、
意図していない方向にスクロールするといった体験になる。

そこでスクロールモード切替直後の一定時間は
一切のトラックボール操作を読み捨てることにした。
この読み捨てる時間のことを Scroll Inhivitor と名付けた。
この Scroll Inhivitor のデフォルト値は 50 ミリ秒である。
短い時間ではあるが結構効いている。

Scroll Inhivitor は config.h で `KEYBALL_SCROLLBALL_INHIVITOR` マクロを定義することで変更できる。
無効化したい場合は値として `0` を設定する。
興味があれば無効にしてみるのも面白いかもしれない。
