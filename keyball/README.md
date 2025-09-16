# Keyball user-level tips: Mod-Tap と AML の注意点／回避策／スワイプ実装指針

このドキュメントは、ユーザーレベルのキーマップ実装（`keyball/lib_user` 配下）で遭遇しやすい事象と、今回導入した回避策・指針をまとめたものです。

## 背景と問題例

- 下位レイヤに Mod-Tap（例: `GUI_T(KC_COMM)` など）がある位置へスワイプ専用キーを重ねると、押下直後〜TAPPING_TERMの判定で GUI などの修飾が混入する場合がある。
- AML（Auto Mouse Layer）有効時は自動でレイヤが遷移するため、押下/解放のレイヤ解決がずれてスワイプ終了が取りこぼされる、あるいは意図しない修飾がまざることがある。
- AML の timeout が短いと、レイヤが素早く戻ってしまい、上記の取りこぼしが起きやすい。
- Windows向け Winスワイプ（Win押しっぱなし＋矢印）のように register_code を使う送出では、レイヤ遷移等が絡むと修飾が残留する危険がある。

## 回避策（今回の実装）

1) Lock レイヤで下位レイヤを遮断
- すべて `XXX` の `_SW_Block` レイヤを用意し、マウスレイヤ（`_mMou/_wMou`）の“下”に置く。
- スワイプキー押下時に `layer_on(_SW_Block)`、解放時に `layer_off(_SW_Block)` する。
- これにより押下中は下位レイヤの Mod-Tap 判定が無効化され、修飾混入を防げる。

2) スワイプ終了の確実化
- 押下開始の物理位置（row/col）を記録し、その物理キーの解放を検知したら必ず `keyball_swipe_end()` を呼ぶ（レイヤ差異の影響を受けにくい）。
- 分割構成では発火処理をマスター側に限定して送出の取りこぼしを防止。
- セッション終了フック `keyball_on_swipe_end(tag)` を実装し、保険として `_SW_Block` の解除を行う。

3) Winスワイプの保護（一般論）
- 押下中のみ必要な修飾＋送出を行い、解放時に通常状態へ戻す実装にする。
- 本件では Lock レイヤ方式により下位レイヤからの修飾混入を防ぐことが要点。

4) AML のタイムアウト調整
- AML の timeout 調整範囲は 300〜3000ms（100ms刻み）。
- 調整は OLED 設定ページ（AML セクション）で行い、`KBC_SAVE` で永続化できます。

## スワイプ独自実装時の注意点

- `process_record_user()` 内で begin/end を必ずペアにする（押下で `keyball_swipe_begin(tag)`、解放で `keyball_swipe_end()`）。
- 送出は基本 `tap_code`/`tap_code16` を使う。`register_code` を使う場合は、解放パスですべて確実に `unregister_code` する（`keyball_on_swipe_end()` での後始末も推奨）。
- Mod-Tap を重ねる位置にスワイプキーを置かないか、置く場合は `_SW_Block` レイヤ方式で下位を遮断する。
- AML 有効時のレイヤ遷移に注意：スワイプ押下中は `_mMou/_wMou` 等のみで完結するよう設計する（今回実装のように押下中だけ補助レイヤON/解放でOFFなど）。
- 分割構成では発火処理をマスター側で行う（実装済・変更不要）。
- OLED/コンソールで状態を確認しながら調整する：
- OLED設定: ページ8構成（Mouse/AML/Scroll/ScrollSnap/Raw/SwipeCfg/SwipeMon/RGB）
  - コンソール: `SWIPE FIRE tag=.. dir=..`、`TAB_SW ... (OS=..)`、`AML: ...`、`SSNP: ...` などのログで分岐確認可能。

## 既知の落とし穴と対策まとめ

- 短すぎる AML timeout → 300ms 以上を推奨。現行は 300〜3000ms に拡張済み。
- 下位レイヤに Mod-Tap → `_SW_Block` レイヤで押下中だけ遮断する。
- スワイプ中にレイヤ/AMLが切り替わる → 物理位置での終了補足＋終了フックで必ず終わるようにする。

## 追加: AML ターゲットレイヤー(TG)の操作と保存

- OLED の設定ページで AML のターゲットレイヤー(TG)を調整できます。
- 調整後に `KBC_SAVE` を押すと、`kbpf.aml_layer` に保存され、再起動後も保持されます。

## 追加: Vial の初期レイアウト（Right）の既定化と挙動

- 既定値は `VIA_EEPROM_LAYOUT_OPTIONS_DEFAULT` で Right を指定しています（`keyball/lib_user/toxaO/keymaps/*/config.h`）。
- さらに初回起動時のみ、VIA のNVMが既存でも Right に矯正するワンショット処理を入れています（`keyball/lib/keyball/keyball.c`）。
  - 一度適用されると `kbpf.reserved` のフラグが立ち、以後はユーザー設定を上書きしません。
- 将来、初期を Left に切り替えたい場合は、上記コメントの手順に従って既定値やワンショットのターゲットを変更してください。

## 最小サンプル（概念）

```
// 抜粋: macro_key.c
case TAB_SW:
  if (record->event.pressed) {
    layer_on(_SW_Block);
    keyball_swipe_begin(KBS_TAG_TAB);
  } else {
    keyball_swipe_end();
    layer_off(_SW_Block);
  }
  return false;

// 抜粋: swipe_user.c
void keyball_on_swipe_fire(kb_swipe_tag_t tag, kb_swipe_dir_t dir) {
  // 方向に応じて tap_code16() 等で送出
}
void keyball_on_swipe_end(kb_swipe_tag_t tag) {
  layer_off(_SW_Block);
}
```

---
本ドキュメントに関する改善提案や運用上の知見があればPR/Issue歓迎です。
