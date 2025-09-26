# Keyball: Special Keycodes

* [English/英語](#english)
* [日本語/Japanese](#japanese)

<a id="english"></a>
## Special Keycodes

Only a minimal set of keyboard-level keycodes remain. All detailed parameter adjustments (Mouse Speed/CPI, Scroll Speed, AML, swipe, scroll snap, RGB) are done in the OLED setting view. For quick tweaks, dedicated speed keys are provided.

### Setting / Scroll
| Keycode    | Value on Remap  | Hex      | Description                               |
|:-----------|:----------------|:---------|:------------------------------------------|
| `KBC_RST`  | `Kb 0`          | `0x7e00` | Reset Keyball configuration               |
| `KBC_SAVE` | `Kb 1`          | `0x7e01` | Persist configuration to EEPROM           |
| `STG_TOG`  | `Kb 2`          | `0x7e02` | Toggle OLED setting view                  |
| `SCRL_TO`  | `Kb 3`          | `0x7e03` | Toggle scroll mode                        |
| `SCRL_MO`  | `Kb 4`          | `0x7e04` | Enable scroll mode while pressed          |
| `SSNP_VRT` | `Kb 5`          | `0x7e05` | Set scroll snap mode to vertical          |
| `SSNP_HOR` | `Kb 6`          | `0x7e06` | Set scroll snap mode to horizontal        |
| `SSNP_FRE` | `Kb 7`          | `0x7e07` | Disable scroll snap mode (free)           |
| `SCSP_DEC` | `Kb 20`         | `0x7e14` | Decrease Scroll Speed (ScSp −)            |
| `SCSP_INC` | `Kb 21`         | `0x7e15` | Increase Scroll Speed (ScSp +)            |
| `MOSP_DEC` | `Kb 22`         | `0x7e16` | Decrease Mouse Speed (CPI −)              |
| `MOSP_INC` | `Kb 23`         | `0x7e17` | Increase Mouse Speed (CPI +)              |

### Swipe Actions
The following keys start a swipe session on press and end it on release. Actual
actions per direction are implemented in user code via hooks
(`keyball_on_swipe_fire/end/tap`).

| Keycode | Value on Remap | Hex     | Description                  |
|:--------|:----------------|:--------|:-----------------------------|
| `APP_SW` | `Kb 8`         | `0x7e08` | App switch swipe             |
| `VOL_SW` | `Kb 9`         | `0x7e09` | Volume/media swipe           |
| `BRO_SW` | `Kb 10`        | `0x7e0a` | Browser/history swipe        |
| `TAB_SW` | `Kb 11`        | `0x7e0b` | Tab switch swipe             |
| `WIN_SW` | `Kb 12`        | `0x7e0c` | Window/desktop swipe         |

<a id="japanese"></a>
## 特殊キーコード

キーボードレベルのキーコードは最小限のみです。Mouse Speed（CPI）/ Scroll Speed / AML / スワイプ / スクロールスナップ / RGB などの詳細調整は、OLED の設定ページで行います。素早い調整用に速度増減キーを用意しました。

### 設定 / スクロール
| キーコード | Remap上での表記 | 値       | 説明                                      |
|:-----------|:----------------|:---------|:------------------------------------------|
| `KBC_RST`  | `Kb 0`          | `0x7e00` | Keyball の設定をリセット                  |
| `KBC_SAVE` | `Kb 1`          | `0x7e01` | Keyball の設定を EEPROM に保存            |
| `STG_TOG`  | `Kb 2`          | `0x7e02` | OLEDの設定表示を切り替え                  |
| `SCRL_TO`  | `Kb 3`          | `0x7e03` | スクロールモードのトグル                  |
| `SCRL_MO`  | `Kb 4`          | `0x7e04` | 押下中のみスクロールモード有効            |
| `SSNP_VRT` | `Kb 5`          | `0x7e05` | スクロールスナップを垂直に設定            |
| `SSNP_HOR` | `Kb 6`          | `0x7e06` | スクロールスナップを水平に設定            |
| `SSNP_FRE` | `Kb 7`          | `0x7e07` | スクロールスナップを解除（自由）          |
| `SCSP_DEC` | `Kb 20`         | `0x7e14` | スクロールスピードを下げる（ScSp−）       |
| `SCSP_INC` | `Kb 21`         | `0x7e15` | スクロールスピードを上げる（ScSp＋）      |
| `MOSP_DEC` | `Kb 22`         | `0x7e16` | マウススピードを下げる（CPI−）            |
| `MOSP_INC` | `Kb 23`         | `0x7e17` | マウススピードを上げる（CPI＋）           |

### スワイプ実行
以下のキーは押下でスワイプ開始、解放で終了します。実際の方向別アクションは
ユーザー側のフック（`keyball_on_swipe_fire/end/tap`）で実装します。

| キーコード | Remap上での表記 | 値       | 説明                       |
|:-----------|:----------------|:---------|:---------------------------|
| `APP_SW`  | `Kb 8`          | `0x7e08` | アプリ切替スワイプ         |
| `VOL_SW`  | `Kb 9`          | `0x7e09` | 音量/メディア操作スワイプ  |
| `BRO_SW`  | `Kb 10`         | `0x7e0a` | ブラウザ/履歴スワイプ      |
| `TAB_SW`  | `Kb 11`         | `0x7e0b` | タブ切替スワイプ           |
| `WIN_SW`  | `Kb 12`         | `0x7e0c` | ウィンドウ/デスクトップ系  |

補足: OLED設定ページの操作はキーコードから削除されました。操作方法は以下の通りです。

- ページ送り: `KC_LEFT`（前ページ）/ `KC_RIGHT`（次ページ）
- 値の調整: `Shift+KC_LEFT`（減少）/ `Shift+KC_RIGHT`（増加）
