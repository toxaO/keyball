# keyball/lib_user — ユーザーレベル拡張の作法

本ディレクトリはユーザー固有の機能（レイヤ、マクロ、OLED表示、スワイプ等）をまとめる領域です。ここでは「ユーザーレベルのカスタムキーコード」を追加・運用する際の手順と、Vial 連携時の注意点をまとめます。

## 1. カスタムキーコードの定義場所
- 定義: `keyball/lib_user/keycode_user.h`
- 実装: `keyball/lib_user/features/macro_user.c` の `process_record_user()` 内に `case ...:` を追加

例: 新しいユーザーキー `MY_ACT` を追加する場合

1) キーコード定義（QK_USER ベースを使用）
```
// keyball/lib_user/keycode_user.h
enum custom_keycodes {
    APP_SW = QK_USER,
    VOL_SW,
    BRO_SW,
    TAB_SW,
    WIN_SW,
    MULTI_A,
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

## 2. Vial 連携時の考え方（重要）

### 2.1 Vial の customKeycodes は QK_KB_* で送出される
- Vial の `vial.json` に `customKeycodes` を並べると、ファームへは「キーボードレベルのカスタムキーコード（QK_KB_*）として」インデックス順に送出されます。
- 一方、本プロジェクトのユーザーキーは `QK_USER_*` で定義します。両者を橋渡しするため、Vial ビルド時のみ以下の対応を行います。

### 2.2 掲載方針（customKeycodes）
- Vial の `customKeycodes` には「キーボードレベル（QK_KB_*) の機能」だけを掲載します。
  - 例: CPI 調整、スクロール関連、スクロールスナップ、AML、スワイプ“設定”系（SW_RT/ST/DZ/FRZ）、デバッグ等
- ユーザーレベル（QK_USER_*）のキーは Vial のリストに掲載しません（配布の初期表示を安定させるため）。
  - 例: 実行系のスワイプキー（APP_SW/BRO_SW/TAB_SW/VOL_SW/WIN_SW/MULTI_A/MULTI_B 等）
  - これらが必要な場合は、Vial の「Any」に16進で直接入力して割当てます（表示は16進のまま）。

### 2.3 並び順と制限
- `customKeycodes` の配列順は `QK_KB_0..` の順に対応します。
- 合計32個（`QK_KB_0..31`）の範囲に収める必要があります。ユーザー系は掲載しない方針のため、この制約は主にKB機能側で満たします。

### 2.4 32 個の制限と拡張の指針
- `QK_KB_*` は 0..31 の32個まで。KB機能の掲載数が上限に達する場合は、優先度の低い機能をVialから外すことを検討してください（ファーム側機能は維持できます）。

### 2.5 Vial UI の表示に関する制約（既知の仕様）
- Vial 上で配置したキーの「割当表示」が `KC_00xx` や `0x7eXX` といった16進表記になることがあります。これは Vial 側の表示仕様・制約によるものです。
- 本ファームでは `keyball/lib_user/keycode_string_user.c` によってデバイス側の表示（OLEDやログ）で分かりやすい名前を出すようにしています。Vial UI 側の表示は完全には制御できません。
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

## 5. スワイプ“設定”キーの16進数対応表（Vialの「Any」でも指定可）
スワイプ設定に関するキー（SW_RT/ST/DZ/FRZ）は Vial のカスタムキーにも掲載しています。任意で「Any」に16進入力して指定することも可能です。

- SW_RT  0x7e12 — スワイプリセット遅延の調整（Shift押下で減少）
- SW_ST  0x7e13 — スワイプ閾値の調整（Shift押下で減少）
- SW_DZ  0x7e14 — スワイプのデッドゾーン調整（Shift押下で減少）
- SW_FRZ 0x7e15 — スワイプ時のポインタフリーズのトグル

使い方（VialのAnyを使う場合）
- レイアウト上の割当先キーを選択 → Keycodes から「Any」を選ぶ → `0x7e12` など上記の値を入力して確定します。
- 表示は16進のままですが、動作はファーム側で処理されます。

注意
- 実行系のスワイプキー（APP_SW/BRO_SW/TAB_SW/VOL_SW/WIN_SW/MULTI_A/MULTI_B 等）は Vial のカスタムキーには掲載していません。必要なら「Any」で16進指定してください（表示は16進になります）。
- スワイプキーとMod-Tapの重なりは意図しない修飾が混ざることがあります。必要に応じて `_SW_Block` レイヤ等で押下中だけ下位を遮断してください（詳細はリポジトリのREADME参照）。

## 6. スワイプ“実行”キーの16進数対応表（現状分・Vialの「Any」で指定）
以下はユーザーレベルで定義しているスワイプ実行系キー（TAB swipe など）の16進数対応です。VialのKeycodesで「Any」を選び、値を直接入力して割り当ててください（表示は16進のまま）。

- APP_SW  0x7e40 — アプリ切替スワイプ（タップは OS 別アプリスイッチ）
- VOL_SW  0x7e41 — 音量スワイプ（タップは再生/一時停止）
- BRO_SW  0x7e42 — ブラウザ履歴スワイプ（タップはアドレスバー）
- TAB_SW  0x7e43 — タブ切替スワイプ（タップは新規タブ）
- WIN_SW  0x7e44 — ウィンドウスワイプ（押下中のみマウスレイヤ強制ON、OS依存の動作）
- MULTI_A 0x7e45 — スワイプ中の補助操作A（例: 前タブ/前デスクトップ/戻る等）、非スワイプ時は Undo
- MULTI_B 0x7e46 — スワイプ中の補助操作B（例: 次タブ/次デスクトップ/進む等）、非スワイプ時は Redo

備考
- これらは `keyball/lib_user/features/macro_user.c` で実装されています。OS別の送出やタップ時の挙動は実装に従います。
- 上記以外のユーザーキー（EISU_S_N / KANA_C_N / ESC_LNG2 など）はスワイプではありませんが、必要に応じ同様に `QK_USER_*` の連番（0x7e40〜）で割当可能です。
