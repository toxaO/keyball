# keyball/lib_user — ユーザーレベル拡張の作法（最新実装対応）

本ディレクトリはユーザー固有の機能（レイヤ、マクロ、OLED表示、スワイプ等）をまとめる領域です。ここでは「ユーザーレベルのカスタムキーコード（QK_USER_*)」の作法と、Vial 連携時の注意点を最新の実装に合わせて整理します。

## 1. カスタムキーコードの定義場所
- 定義: `keyball/lib_user/keycode_user.h`
- 実装: `keyball/lib_user/features/macro_user.c` の `process_record_user()` 内に `case ...:` を追加

例: 新しいユーザーキー `MY_ACT` を追加する場合

1) キーコード定義（QK_USER ベースを使用）
```
// keyball/lib_user/keycode_user.h
// 注意: スワイプ実行キー（SW_APP/SW_VOL/SW_BRO/SW_TAB/SW_WIN）は
// いまは「キーボードレベル(QK_KB_*)」に移管されています。ここでは定義しません。
enum custom_keycodes {
    MULTI_A = QK_USER, // 先頭に割当（例）
    MULTI_B,
    // ここに追記
    MY_ACT,
};
```

2) 動作実装
```
// keyball/lib_user/features/macro_user.c
bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
    case MY_ACT:
        if (record->event.pressed) {
            // ここに押下時の動作
        } else {
            // ここに解放時の動作（必要なら）
        }
        return false; // ここで処理完結
    }
    return true; // その他はデフォルトへ
}
```

3) 名称表示（任意）
```
// keyball/lib_user/keycode_string_user.c
// OLED やログ等の表示名テーブルに追記
{ MY_ACT, "MY_ACT" },
```

## 2. Vial 連携時の考え方（重要・最新）

### 2.1 Vial の customKeycodes は QK_KB_* で送出される
- Vial の `customKeycodes` は「キーボードレベル（QK_KB_*）」のキーとして送出されます。
- 一方、本プロジェクトのユーザーキーは `QK_USER_*` で定義します。両者はレベルが異なるため、そのまま 1:1 掲載はしません。

### 2.2 掲載方針（customKeycodes）
- Vial の `customKeycodes` には「キーボードレベル（QK_KB_*）の機能」のみを掲載します。
- 現状の掲載対象（例）: `KBC_RST`/`KBC_SAVE`、`STG_TOG`、`SCRL_TO`/`SCRL_MO`、`SSNP_*`（有効時）、スワイプ実行 `SW_APP`/`SW_VOL`/`SW_BRO`/`SW_TAB`/`SW_WIN` など。
- ユーザーレベル（QK_USER_*）のキーは Vial のリストに掲載しません（配布初期状態の安定性重視）。
  - 必要に応じ、Vial の「Any」で 16 進値（`QK_USER_*` 範囲）を直接指定して割当可能です（表示は 16 進のまま）。

### 2.3 並び順と制限
- `customKeycodes` の配列順は `QK_KB_0..` の順に対応します。
- 合計32個（`QK_KB_0..31`）の範囲に収める必要があります。ユーザー系は掲載しない方針のため、この制約は主にKB機能側で満たします。

### 2.4 32 個の制限と拡張の指針
- `QK_KB_*` は 0..31 の32個まで。KB機能の掲載数が上限に達する場合は、優先度の低いものを Vial 掲載から外すことを検討してください（ファームの実装自体は維持可能）。

### 2.5 Vial UI の表示に関する制約（既知の仕様）
- Vial 上で配置したキーの「割当表示」が `KC_00xx` や `0x7eXX` といった16進表記になることがあります。これは Vial 側の表示仕様・制約によるものです。
- 本ファームでは `keyball/lib_user/keycode_string_user.c` によってデバイス側の表示（OLEDやログ）で分かりやすい名前を出すようにしています。Vial UI 側の表示は完全には制御できません。

## Key Override（Vial対応）
- Vial の Key Overrides タブから、修飾条件に応じた置換（例: Shift+数字→記号 など）を定義できます。
- 本リポジトリの Vial ビルドでは `KEY_OVERRIDE_ENABLE = yes` を有効化済（`vial-qmk/.../keymaps/mymap/rules.mk`）。
- 既存のキーマップ側に静的な `key_overrides[]` を用意する必要はありません（Vial がEEPROMへ保存・適用します）。
- 配布用のキーマップでは、初期レイアウトにユーザーキー（QK_USER_*）を含めない方針を推奨します（Vial初期表示で混乱を避けるため）。

## 3. 追加チェックリスト（Vial対応を行う場合）
- [ ] `keycode_user.h` に `QK_USER_*` を追加
- [ ] `macro_user.c` の `process_record_user()` に処理を追加
- [ ] `keycode_string_user.c` に表示名（任意）
- [ ] `vial.json` の `customKeycodes` の末尾に（順序を守って）追加・または差替
- [ ] ただし合計 32 個（0..31）に収める
- [ ] Vial ビルドで動作確認

## 4. ビルド
```
make -C vial-qmk SKIP_GIT=yes VIAL_ENABLE=yes keyball/keyball44:mymap
```

以上。Vial 側表示の細かな表記（16進表示など）は現状の仕様として受け入れ、
ファーム側ではキーの入力・動作が期待通りであることを重視しています。

## 5. スワイプ設定と実行の分担（重要な変更点）
- スワイプ“設定”（しきい値/デッドゾーン/リセット/フリーズ）は、専用キーではなく「OLED の設定ページ」で調整します。保存は `KBC_SAVE`。
- スワイプ“実行”（押下で開始・解放で終了）は「キーボードレベルのキー（`SW_APP`/`SW_VOL`/`SW_BRO`/`SW_TAB`/`SW_WIN`）」で行います。
- 方向別の送出は `keyball_on_swipe_fire()`（user実装）で記述します。タップ時のフォールバックは `keyball_on_swipe_tap()` で上書き可能です。

補足: Mod-Tap等の干渉が気になる場合、`process_record_user()` 側で `SW_APP` などの押下をフックして `_SW_Block` を ON にし、解放は `keyball_on_swipe_end()` で OFF にする、という併用も可能です（キーボードレベルの処理は `true` を返して通す）。
