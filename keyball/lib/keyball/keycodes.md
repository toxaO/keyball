# Keyball: Special Keycodes

* [English/英語](#english)
* [日本語/Japanese](#japanese)

<a id="english"></a>
## Special Keycodes

### Configuration
| Keycode    | Value on Remap  | Hex      | Description                                                       |
|:-----------|:----------------|:---------|:------------------------------------------------------------------|
| `KBC_RST`  | `Kb 0`          | `0x7e00` | Reset Keyball configuration[^1]                                   |
| `KBC_SAVE` | `Kb 1`          | `0x7e01` | Persist Keyball configuration[^1] to EEPROM                       |

### Pointer settings
| Keycode    | Value on Remap  | Hex      | Description                                                       |
|:-----------|:----------------|:---------|:------------------------------------------------------------------|
| `CPI_I100` | `Kb 2`          | `0x7e02` | Increase 100 CPI (max 12000)                                      |
| `CPI_D100` | `Kb 3`          | `0x7e03` | Decrease 100 CPI (min 100)                                        |
| `MVGL`     | `Kb 17`         | `0x7e11` | Adjust low-speed pointer gain (Shift: decrease)                   |
| `MVTH1`    | `Kb 19`         | `0x7e13` | Adjust pointer threshold1 (Shift: decrease)                       |

### Scroll control
| Keycode    | Value on Remap  | Hex      | Description                                                       |
|:-----------|:----------------|:---------|:------------------------------------------------------------------|
| `SCRL_TO`  | `Kb 6`          | `0x7e06` | Toggle scroll mode                                                |
| `SCRL_MO`  | `Kb 7`          | `0x7e07` | Enable scroll mode while pressed                                  |
| `SCRL_DVI` | `Kb 8`          | `0x7e08` | Increase scroll divider (max D7 = 1/128)                          |
| `SCRL_DVD` | `Kb 9`          | `0x7e09` | Decrease scroll divider (min D0 = 1/1)                            |
| `SCRL_INV` | `Kb 16`         | `0x7e10` | Invert scroll direction                                           |
| `SCRL_IVI` | `Kb 18`         | `0x7e12` | Adjust scroll interval (+1 / Shift: -1)                           |
| `SCRL_IVD` | `Kb 20`         | `0x7e14` | Adjust scroll interval coarse (+10 / Shift: -10)                  |
| `SCRL_VLI` | `Kb 22`         | `0x7e16` | Adjust scroll value (+1 / Shift: -1)                              |
| `SCRL_VLD` | `Kb 24`         | `0x7e18` | Adjust scroll value coarse (+10 / Shift: -10)                     |
| `SCRL_DZ`  | `Kb 29`         | `0x7e1d` | Adjust scroll deadzone (Shift: decrease)                          |
| `SCRL_HY`  | `Kb 31`         | `0x7e1f` | Adjust scroll hysteresis (Shift: decrease)                        |

### Scroll snap
| Keycode    | Value on Remap  | Hex      | Description                                                       |
|:-----------|:----------------|:---------|:------------------------------------------------------------------|
| `SSNP_VRT` | `Kb 13`         | `0x7e0d` | Set scroll snap mode to vertical                                  |
| `SSNP_HOR` | `Kb 14`         | `0x7e0e` | Set scroll snap mode to horizontal                                |
| `SSNP_FRE` | `Kb 15`         | `0x7e0f` | Disable scroll snap mode (free scroll)                            |

### Automatic mouse layer
| Keycode    | Value on Remap  | Hex      | Description                                                       |
|:-----------|:----------------|:---------|:------------------------------------------------------------------|
| `AML_TO`   | `Kb 10`         | `0x7e0a` | Toggle automatic mouse layer                                      |
| `AML_I50`  | `Kb 11`         | `0x7e0b` | Increase 50ms automatic mouse layer timeout (max 1000ms)          |
| `AML_D50`  | `Kb 12`         | `0x7e0c` | Decrease 50ms automatic mouse layer timeout (min 100ms)           |

### Swipe control
| Keycode    | Value on Remap  | Hex      | Description                                                       |
|:-----------|:----------------|:---------|:------------------------------------------------------------------|
| `SW_RT`    | `Kb 4`          | `0x7e04` | Adjust swipe reset delay (Shift: decrease)                        |
| `SW_ST`    | `Kb 21`         | `0x7e15` | Adjust swipe trigger threshold (Shift: decrease)                  |
| `SW_DZ`    | `Kb 23`         | `0x7e17` | Adjust swipe deadzone (Shift: decrease)                           |
| `SW_FRZ`   | `Kb 25`         | `0x7e19` | Toggle swipe pointer freeze                                       |

### Debug
| Keycode    | Value on Remap  | Hex      | Description                                                       |
|:-----------|:----------------|:---------|:------------------------------------------------------------------|
| `DBG_TOG`  | `Kb 26`         | `0x7e1a` | Toggle OLED debug mode                                            |
| `DBG_NP`   | `Kb 27`         | `0x7e1b` | OLED debug page next                                              |
| `DBG_PP`   | `Kb 28`         | `0x7e1c` | OLED debug page previous                                          |

[^1]: CPI, scroll divider, automatic mouse layer's enable/disable, and automatic mouse layer's timeout.

<a id="japanese"></a>
## 特殊キーコード

### 設定
| キーコード | Remap上での表記 | 値       | 説明                                                              |
|:-----------|:----------------|:---------|:------------------------------------------------------------------|
| `KBC_RST`  | `Kb 0`          | `0x7e00` | Keyball設定[^2]のリセット                                         |
| `KBC_SAVE` | `Kb 1`          | `0x7e01` | 現在のKeyball設定[^2]をEEPROMに保存します                         |

### ポインタ設定
| キーコード | Remap上での表記 | 値       | 説明                                                              |
|:-----------|:----------------|:---------|:------------------------------------------------------------------|
| `CPI_I100` | `Kb 2`          | `0x7e02` | CPIを100増加させます(最大:12000)                                  |
| `CPI_D100` | `Kb 3`          | `0x7e03` | CPIを100減少させます(最小:100)                                    |
| `MVGL`     | `Kb 17`         | `0x7e11` | 低速ゲインを調整します(Shiftで減少)                               |
| `MVTH1`    | `Kb 19`         | `0x7e13` | ポインタしきい値1を調整します(Shiftで減少)                        |

### スクロール制御
| キーコード | Remap上での表記 | 値       | 説明                                                              |
|:-----------|:----------------|:---------|:------------------------------------------------------------------|
| `SCRL_TO`  | `Kb 6`          | `0x7e06` | タップごとにスクロールモードのON/OFFを切り替えます                |
| `SCRL_MO`  | `Kb 7`          | `0x7e07` | キーを押している間、スクロールモードになります                    |
| `SCRL_DVI` | `Kb 8`          | `0x7e08` | スクロール除数を１つ上げます(max D7 = 1/128)←最もスクロール遅い   |
| `SCRL_DVD` | `Kb 9`          | `0x7e09` | スクロール除数を１つ下げます(min D0 = 1/1)←最もスクロール速い     |
| `SCRL_INV` | `Kb 16`         | `0x7e10` | スクロール方向を反転します                                       |
| `SCRL_IVI` | `Kb 18`         | `0x7e12` | スクロール interval を調整(+1 / Shift:-1)                         |
| `SCRL_IVD` | `Kb 20`         | `0x7e14` | スクロール interval を粗く調整(+10 / Shift:-10)                   |
| `SCRL_VLI` | `Kb 22`         | `0x7e16` | スクロール value を調整(+1 / Shift:-1)                            |
| `SCRL_VLD` | `Kb 24`         | `0x7e18` | スクロール value を粗く調整(+10 / Shift:-10)                      |
| `SCRL_DZ`  | `Kb 29`         | `0x7e1d` | スクロールのデッドゾーンを調整します(Shiftで減少)                 |
| `SCRL_HY`  | `Kb 31`         | `0x7e1f` | スクロール反転ヒステリシスを調整します(Shiftで減少)               |

## Scroll interval/value (新実装の補足)

- sdiv は除数ではなく「スクロール感度レベル (SL)」として扱います。
- 実際の挙動は interval（蓄積のしきい値）と value（出力の分母）で決まります。
- Windows の例: interval=1, value=1 で高精度、interval=120, value=1 で通常風。
- macOS の例: interval=120, value=120 で通常風。
- OS ごとに `KBC_SAVE` で保存すれば、以降は OS 検出によるスロット切替だけで同じロジックが動きます（OS 分岐コードはありません）。

### スクロールスナップ
| キーコード | Remap上での表記 | 値       | 説明                                                              |
|:-----------|:----------------|:---------|:------------------------------------------------------------------|
| `SSNP_VRT` | `Kb 13`         | `0x7e0d` | スクロールスナップモードを垂直にする                              |
| `SSNP_HOR` | `Kb 14`         | `0x7e0e` | スクロールスナップモードを水平にする                              |
| `SSNP_FRE` | `Kb 15`         | `0x7e0f` | スクロールスナップモードを無効にする(自由スクロール)              |

### 自動マウスレイヤー
| キーコード | Remap上での表記 | 値       | 説明                                                              |
|:-----------|:----------------|:---------|:------------------------------------------------------------------|
| `AML_TO`   | `Kb 10`         | `0x7e0a` | 自動マウスレイヤーをトグルします                                  |
| `AML_I50`  | `Kb 11`         | `0x7e0b` | 自動マウスレイヤーのタイムアウトを50msec増やします (最大1000ms)   |
| `AML_D50`  | `Kb 12`         | `0x7e0c` | 自動マウスレイヤーのタイムアウトを50msec減らします (最小100ms)    |

### スワイプ制御
| キーコード | Remap上での表記 | 値       | 説明                                                              |
|:-----------|:----------------|:---------|:------------------------------------------------------------------|
| `SW_RT`    | `Kb 4`          | `0x7e04` | スワイプリセット遅延を調整します(Shiftで短縮)                     |
| `SW_ST`    | `Kb 21`         | `0x7e15` | スワイプ閾値を調整します(Shiftで減少)                             |
| `SW_DZ`    | `Kb 23`         | `0x7e17` | スワイプゆらぎ抑制を調整します(Shiftで減少)                       |
| `SW_FRZ`   | `Kb 25`         | `0x7e19` | スワイプ時のポインタフリーズを切り替えます                        |

### デバッグ
| キーコード | Remap上での表記 | 値       | 説明                                                              |
|:-----------|:----------------|:---------|:------------------------------------------------------------------|
| `DBG_TOG`  | `Kb 26`         | `0x7e1a` | OLEDデバッグモードを切り替えます                                  |
| `DBG_NP`   | `Kb 27`         | `0x7e1b` | OLEDデバッグページを次に送ります                                  |
| `DBG_PP`   | `Kb 28`         | `0x7e1c` | OLEDデバッグページを前に戻します                                  |

[^2]: CPI、スクロール除数、自動マウスレイヤーのON/OFF状態、及び自動マウスレイヤのタイムアウト
