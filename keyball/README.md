# Keyball の使い方（OLED設定・デフォルト機能・キーコード）

本書は「OLED上で調整できる項目」「デフォルトで搭載しているスワイプ/マルチキーの動作」「カスタムキーコードの概要」をまとめたものです。API等の詳細は `keyball/lib/keyball/README.md` を参照してください。

## OLED 設定ページと調整項目

設定モードの入/切は `STG_TOG`。ページ移動は `STG_NP`/`STG_PP`（または設定モード中の Shift+←/→）です。選択行は ↑/↓、値の増減は ←/→ で行います。`KBC_SAVE` で保存、`KBC_RST` で初期化します。

- Mouse（ポインタ）
  - MoSp: マウススピード（CPIベース）。
  - Glo: 低速域ゲイン（%）。OLED操作は5%刻み（内部は近似）。
  - Th1/Th2: 低速/高速のしきい値。
- AML（Auto Mouse Layer）
  - en: 有効(1)/無効(0)
  - TO: タイムアウト（100..9500ms, HOLD=無制限）
  - TH: 閾値（既定100）
  - TG_L: 追従先レイヤ（0..31）
- Scroll（スクロール）
  - ScSp: スクロールスピード（体感速度段）
  - Dz: デッドゾーン（小入力の無視）
  - Iv: 反転（0/1）
  - Md: プリセット（nor/fin/mac 相当）
  - H_Ga: 最終水平ゲイン（垂直基準に対する%）
- Scroll Snap（スクロールスナップ）
  - Mode: VRT/HOR/FRE（垂直/水平/自由）
  - Thr: テンション閾値（FREEへ一時遷移するしきい）
  - Rst: FREE 継続時間（ms）
- Swipe conf（スワイプ設定）
  - St: 発火しきい値
  - Dz: デッドゾーン
  - Rt: リセット（ms）
  - Frz: フリーズ（押下中ポインタを止める等）
- RGB（有効時）
  - on/off, H/S/V, Mode
- layer conf
  - def: 既定レイヤー（保存して保持）

## デフォルト搭載のスワイプ・マルチキー

スワイプ実行キー（押下で開始・解放で終了）
- `APP_SW` / `VOL_SW` / `BRO_SW` / `TAB_SW` / `WIN_SW`
  - 方向（Up/Down/Left/Right）に応じてユーザー実装のフックにより動作します。
  - 例: App切り替え、音量、ブラウザ履歴、タブ、Windowsのウィンドウスナップ 等。
- `ARR_SW`（矢印疑似）：押下中に矢印方向を1回発火させる用途など。

マルチキー（ユーザーフックで任意動作）
- `MULTI_A` / `MULTI_B` / `MULTI_C` / `MULTI_D`
  - スワイプの「モード」や方向に応じた多段アクションの割当て例が `lib_user` にあります。

ヒント（拡張の例）
- 疑似フリック入力: `keyball_on_swipe_fire(tag, dir)` で、方向に応じた文字や役割を送出することで実現可能です。

## カスタムキーコード（キーボードレベル）

Vial の customKeycodes にも掲載される主要キー：
- KBC_*: `KBC_RST`（初期化）, `KBC_SAVE`（保存）
- STG_*: `STG_TOG`（設定ON/OFF）, `STG_NP` / `STG_PP`（設定ページ移動）
- スクロール: `SCRL_TO`（トグル）, `SCRL_MO`（押下中）
  - スピード調整: `SCSP_DEC` / `SCSP_INC`（ScSp−/＋）
- マウス速度: `MOSP_DEC` / `MOSP_INC`（MoSp−/＋）
- スクロールスナップ: `SSNP_VRT` / `SSNP_HOR` / `SSNP_FRE`
- スワイプ実行: `APP_SW` / `VOL_SW` / `BRO_SW` / `TAB_SW` / `WIN_SW` / `ARR_SW` / `SW_EX1` / `SW_EX2`

Vial で編集する場合
- Vial/QMKのVial対応ビルドを使えば、キーマップとこれらのキーコードをGUIから配置可能です。

## 参考

- 詳細なAPIや内部仕様は `keyball/lib/keyball/README.md` を参照してください。
- ユーザーカスタムやサンプルは `keyball/lib_user/` を参照してください。
