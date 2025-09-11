# Keyball: Special Keycodes

* [English/英語](#english)
* [日本語/Japanese](#japanese)

<a id="english"></a>
## Special Keycodes

| Keycode    | Value on Remap  | Hex      | Description                                                       |
|:-----------|:----------------|:---------|:------------------------------------------------------------------|
| `KBC_RST`  | `Kb 0`          | `0x7e00` | Reset Keyball configuration[^1]                                   |
| `KBC_SAVE` | `Kb 1`          | `0x7e01` | Persist Keyball configuration[^1] to EEPROM                       |
| `CPI_I100` | `Kb 2`          | `0x7e02` | Increase 100 CPI (max 12000)                                      |
| `CPI_D100` | `Kb 3`          | `0x7e03` | Decrease 100 CPI (min 100)                                        |
| `SW_RT`    | `Kb 4`          | `0x7e04` | Adjust swipe reset delay (Shift: decrease)                        |
| `SCRL_TO`  | `Kb 6`          | `0x7e06` | Toggle scroll mode                                                |
| `SCRL_MO`  | `Kb 7`          | `0x7e07` | Enable scroll mode while pressed                                  |
| `SCRL_DVI` | `Kb 8`          | `0x7e08` | Increase scroll divider (max D7 = 1/128)                          |
| `SCRL_DVD` | `Kb 9`          | `0x7e09` | Decrease scroll divider (min D0 = 1/1)                            |
| `AML_TO`   | `Kb 10`         | `0x7e0a` | Toggle automatic mouse layer                                      |
| `AML_I50`  | `Kb 11`         | `0x7e0b` | Increase 50ms automatic mouse layer timeout (max 1000ms)          |
| `AML_D50`  | `Kb 12`         | `0x7e0c` | Decrease 50ms automatic mouse layer timeout (min 100ms)           |
| `SSNP_VRT` | `Kb 13`         | `0x7e0d` | Set scroll snap mode to vertical                                  |
| `SSNP_HOR` | `Kb 14`         | `0x7e0e` | Set scroll snap mode to horizontal                                |
| `SSNP_FRE` | `Kb 15`         | `0x7e0f` | Disable scroll snap mode (free scroll)                            |
| `SCRL_INV` | `Kb 16`         | `0x7e10` | Invert scroll direction                                           |
| `MVGL`     | `Kb 17`         | `0x7e11` | Adjust low-speed pointer gain (Shift: decrease)                   |
| `MVTH1`    | `Kb 19`         | `0x7e13` | Adjust pointer threshold1 (Shift: decrease)                       |
| `SW_ST`    | `Kb 21`         | `0x7e15` | Adjust swipe trigger threshold (Shift: decrease)                  |
| `SW_DZ`    | `Kb 23`         | `0x7e17` | Adjust swipe deadzone (Shift: decrease)                           |
| `SW_FRZ`   | `Kb 25`         | `0x7e19` | Toggle swipe pointer freeze                                       |
| `DBG_TOG`  | `Kb 26`         | `0x7e1a` | Toggle OLED debug mode                                            |
| `DBG_NP`   | `Kb 27`         | `0x7e1b` | OLED debug page next                                              |
| `DBG_PP`   | `Kb 28`         | `0x7e1c` | OLED debug page previous                                          |
| `SCRL_DZ`  | `Kb 29`         | `0x7e1d` | Adjust scroll deadzone (Shift: decrease)                          |
| `SCRL_HY`  | `Kb 31`         | `0x7e1f` | Adjust scroll hysteresis (Shift: decrease)                        |

[^1]: CPI, scroll divider, automatic mouse layer's enable/disable, and automatic mouse layer's timeout.

<a id="japanese"></a>
## 特殊キーコード

| キーコード | Remap上での表記 | 値       | 説明                                                              |
|:-----------|:----------------|:---------|:------------------------------------------------------------------|
| `KBC_RST`  | `Kb 0`          | `0x7e00` | Keyball設定[^2]のリセット                                         |
| `KBC_SAVE` | `Kb 1`          | `0x7e01` | 現在のKeyball設定[^2]をEEPROMに保存します                         |
| `CPI_I100` | `Kb 2`          | `0x7e02` | CPIを100増加させます(最大:12000)                                  |
| `CPI_D100` | `Kb 3`          | `0x7e03` | CPIを100減少させます(最小:100)                                    |
| `SW_RT`    | `Kb 4`          | `0x7e04` | スワイプリセット遅延を調整します(Shiftで短縮)                     |
| `SCRL_TO`  | `Kb 6`          | `0x7e06` | タップごとにスクロールモードのON/OFFを切り替えます                |
| `SCRL_MO`  | `Kb 7`          | `0x7e07` | キーを押している間、スクロールモードになります                    |
| `SCRL_DVI` | `Kb 8`          | `0x7e08` | スクロール除数を１つ上げます(max D7 = 1/128)←最もスクロール遅い   |
| `SCRL_DVD` | `Kb 9`          | `0x7e09` | スクロール除数を１つ下げます(min D0 = 1/1)←最もスクロール速い     |
| `AML_TO`   | `Kb 10`         | `0x7e0a` | 自動マウスレイヤーをトグルします                                  |
| `AML_I50`  | `Kb 11`         | `0x7e0b` | 自動マウスレイヤーのタイムアウトを50msec増やします (最大1000ms)   |
| `AML_D50`  | `Kb 12`         | `0x7e0c` | 自動マウスレイヤーのタイムアウトを50msec減らします (最小100ms)    |
| `SSNP_VRT` | `Kb 13`         | `0x7e0d` | スクロールスナップモードを垂直にする                              |
| `SSNP_HOR` | `Kb 14`         | `0x7e0e` | スクロールスナップモードを水平にする                              |
| `SSNP_FRE` | `Kb 15`         | `0x7e0f` | スクロールスナップモードを無効にする(自由スクロール)              |
| `SCRL_INV` | `Kb 16`         | `0x7e10` | スクロール方向を反転します                                       |
| `MVGL`     | `Kb 17`         | `0x7e11` | 低速ゲインを調整します(Shiftで減少)                               |
| `MVTH1`    | `Kb 19`         | `0x7e13` | ポインタしきい値1を調整します(Shiftで減少)                        |
| `SW_ST`    | `Kb 21`         | `0x7e15` | スワイプ閾値を調整します(Shiftで減少)                             |
| `SW_DZ`    | `Kb 23`         | `0x7e17` | スワイプゆらぎ抑制を調整します(Shiftで減少)                       |
| `SW_FRZ`   | `Kb 25`         | `0x7e19` | スワイプ時のポインタフリーズを切り替えます                        |
| `DBG_TOG`  | `Kb 26`         | `0x7e1a` | OLEDデバッグモードを切り替えます                                  |
| `DBG_NP`   | `Kb 27`         | `0x7e1b` | OLEDデバッグページを次に送ります                                  |
| `DBG_PP`   | `Kb 28`         | `0x7e1c` | OLEDデバッグページを前に戻します                                  |
| `SCRL_DZ`  | `Kb 29`         | `0x7e1d` | スクロールのデッドゾーンを調整します(Shiftで減少)                 |
| `SCRL_HY`  | `Kb 31`         | `0x7e1f` | スクロール反転ヒステリシスを調整します(Shiftで減少)               |

[^2]: CPI、スクロール除数、自動マウスレイヤーのON/OFF状態、及び自動マウスレイヤのタイムアウト
