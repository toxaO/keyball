# Keyball Vial での row0/col1 ずれと左右ポインタ動作の不整合について

## 背景
- `keyball` ディレクトリ配下の Vial 対応キーマップでは、Vial 接続時にレイヤー 0 の `row0/col1` が `KC_Q` から `KC_R` に変わる症状が発生していた。
- USB をトラックボールと反対側（例: 左手側）に接続すると、ポインタが動かない／キーが反応しないといった不具合も併発していた。
- 一方、`holykeebs` ディレクトリの最小構成 (`vial_min`) では問題が再現せず、両手側でポインタ操作も可能だった。

## 根本原因の比較
| 項目 | keyball 旧構成 | holykeebs 最小構成 | 影響 |
| ---- | --------------- | ------------------- | ----- |
| `VIA_EEPROM_LAYOUT_OPTIONS_DEFAULT` | `0x00000001` (Right 固定) | `0` | Vial 初期同期時にレイアウトオプションが「右ボール」と矯正され、`LAYOUT_universal` の物理位置とズレて row0/col1 が `KC_R` へシフトする。 |
| 動的キーマップ層数 (`DYNAMIC_KEYMAP_LAYER_COUNT`) | 8〜12 層に拡張 | 4 層 (最小) | ゼロ初期化された領域が大きく、Vial 側が `KC_NO(0x0001)` を読み取る確率が増加。ズレが顕在化しやすい。 |
| キーボード独自プロファイル (`keyball_kbpf`) | 有効。レイアウトオプションを 1 度だけ Right へ書き換える処理あり | 未使用 | EEPROM 書き換え後もボール位置が Right 固定となり、row0/col1 のズレを助長。 |
| トラックボール検出 | `pointing_device_get_status()` 未定義環境で `keyball.this_have_ball = true;` と見なす | `pmw33xx_check_signature(0)` で物理センサを確認 | 左手側 USB 接続時に、左側でもボール有りと誤認しスクロール処理に吸い込むためポインタが動かなくなる。 |
| OLED 回転 | 右手マスターのみ `OLED_ROTATION_270` | マスター側は左右問わず回転 | 左手をマスターにした際、OLED 表示の初期化不整合からキースキャンがハングする恐れ。 |

## 対応内容
1. `keyball/lib/keyball/keyball.c`
   - `pmw33xx_check_signature(0)` を用いてセンサの有無を判定し、左右どちらをマスターにしてもポインタを動作させるよう変更。
   - `via_set_layout_options()` で Right 固定に矯正する処理を削除。
2. `keyball/keyball44/keymaps/via/config.h` ほか (39/61 含む)
   - `VIA_EEPROM_LAYOUT_OPTIONS_DEFAULT` を `0` に戻し、Vial 初期同期でレイアウトオプションを固定しないよう統一。
3. Vial 用キーマップ分離 (44)
   - none/right/left/dual を別キーマップとして分け、`vial.json` から `layout` オプションを削除して純粋な物理配列と一致させることで row0/col1 のズレを解消。
4. `keyball/lib/keyball/keyball_oled.c`
   - `oled_init_kb()` で、マスター側 OLED を左右問わず 270 度回転させ、左手マスター時のフリーズを防止。
5. EEPROM 初期化
   - 変更後は `EEPROM Reset` を実施し、旧設定が残っている場合は手動でリセットすることで正常化。

## 今後の注意点
- Vial 用に多機能な `keyball_kbpf` と独自設定を保持する場合、初期レイアウト書き換えや大容量動的キーマップと Vial の整合性に留意する。
- レイアウト切り替えはレイアウトオプションではなく、キーマップ自体を none/right/left/dual に分割して配布する方が安全。
- 左手マスター運用を想定する場合、OLED 初期化や `keyball.this_have_ball` 判定が左右で一貫していることを確認する。

