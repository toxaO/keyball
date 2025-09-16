# Keyball: Special Keycodes

* [English/英語](#english)
* [日本語/Japanese](#japanese)

<a id="english"></a>
## Special Keycodes

Only a minimal set of keyboard-level keycodes remain. All detailed parameter adjustments (CPI, scroll step, AML, swipe, scroll snap, RGB) are done in the OLED setting view.

### Setting / Scroll
| Keycode    | Value on Remap  | Hex      | Description                               |
|:-----------|:----------------|:---------|:------------------------------------------|
| `KBC_RST`  | `Kb 0`          | `0x7e00` | Reset Keyball configuration               |
| `KBC_SAVE` | `Kb 1`          | `0x7e01` | Persist configuration to EEPROM           |
| `STG_TOG`  | `Kb 2`          | `0x7e02` | Toggle OLED setting view                  |
| `STG_NP`   | `Kb 3`          | `0x7e03` | OLED setting page next                    |
| `STG_PP`   | `Kb 4`          | `0x7e04` | OLED setting page previous                |
| `SCRL_TO`  | `Kb 5`          | `0x7e05` | Toggle scroll mode                        |
| `SCRL_MO`  | `Kb 6`          | `0x7e06` | Enable scroll mode while pressed          |
| `SSNP_VRT` | `Kb 7`          | `0x7e07` | Set scroll snap mode to vertical          |
| `SSNP_HOR` | `Kb 8`          | `0x7e08` | Set scroll snap mode to horizontal        |
| `SSNP_FRE` | `Kb 9`          | `0x7e09` | Disable scroll snap mode (free)           |

### Swipe Actions
The following keys start a swipe session on press and end it on release. Actual
actions per direction are implemented in user code via hooks
(`keyball_on_swipe_fire/end/tap`).

| Keycode | Value on Remap | Hex     | Description                  |
|:--------|:----------------|:--------|:-----------------------------|
| `APP_SW` | `Kb 10`        | `0x7e0a` | App switch swipe             |
| `VOL_SW` | `Kb 11`        | `0x7e0b` | Volume/media swipe           |
| `BRO_SW` | `Kb 12`        | `0x7e0c` | Browser/history swipe        |
| `TAB_SW` | `Kb 13`        | `0x7e0d` | Tab switch swipe             |
| `WIN_SW` | `Kb 14`        | `0x7e0e` | Window/desktop swipe         |

<a id="japanese"></a>
## 特殊キーコード

キーボードレベルのキーコードは最小限のみです。CPI/スクロール段階/AML/スワイプ/スクロールスナップ/RGB などの詳細調整は、OLED の設定ページで行います。

### 設定 / スクロール
| キーコード | Remap上での表記 | 値       | 説明                                      |
|:-----------|:----------------|:---------|:------------------------------------------|
| `KBC_RST`  | `Kb 0`          | `0x7e00` | Keyball の設定をリセット                  |
| `KBC_SAVE` | `Kb 1`          | `0x7e01` | Keyball の設定を EEPROM に保存            |
| `STG_TOG`  | `Kb 2`          | `0x7e02` | OLEDの設定表示を切り替え                  |
| `STG_NP`   | `Kb 3`          | `0x7e03` | OLED設定ページを次に送る                  |
| `STG_PP`   | `Kb 4`          | `0x7e04` | OLED設定ページを前に戻す                  |
| `SCRL_TO`  | `Kb 5`          | `0x7e05` | スクロールモードのトグル                  |
| `SCRL_MO`  | `Kb 6`          | `0x7e06` | 押下中のみスクロールモード有効            |
| `SSNP_VRT` | `Kb 7`          | `0x7e07` | スクロールスナップを垂直に設定            |
| `SSNP_HOR` | `Kb 8`          | `0x7e08` | スクロールスナップを水平に設定            |
| `SSNP_FRE` | `Kb 9`          | `0x7e09` | スクロールスナップを解除（自由）          |

### スワイプ実行
以下のキーは押下でスワイプ開始、解放で終了します。実際の方向別アクションは
ユーザー側のフック（`keyball_on_swipe_fire/end/tap`）で実装します。

| キーコード | Remap上での表記 | 値       | 説明                       |
|:-----------|:----------------|:---------|:---------------------------|
| `APP_SW`  | `Kb 10`         | `0x7e0a` | アプリ切替スワイプ         |
| `VOL_SW`  | `Kb 11`         | `0x7e0b` | 音量/メディア操作スワイプ  |
| `BRO_SW`  | `Kb 12`         | `0x7e0c` | ブラウザ/履歴スワイプ      |
| `TAB_SW`  | `Kb 13`         | `0x7e0d` | タブ切替スワイプ           |
| `WIN_SW`  | `Kb 14`         | `0x7e0e` | ウィンドウ/デスクトップ系  |
