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
| `MVGL`     | `Kb 4`          | `0x7e04` | Adjust low-speed pointer gain (Shift: decrease)                   |
| `MVTH1`    | `Kb 5`          | `0x7e05` | Adjust pointer threshold1 (Shift: decrease)                       |

### Scroll control
| Keycode    | Value on Remap  | Hex      | Description                                                       |
|:-----------|:----------------|:---------|:------------------------------------------------------------------|
| `SCRL_PST` | `Kb 6`          | `0x7e06` | Switch scroll preset: mac={120,120}, others={120,1}<->{1,1}       |
| `SCRL_TO`  | `Kb 7`          | `0x7e07` | Toggle scroll mode                                                |
| `SCRL_MO`  | `Kb 8`          | `0x7e08` | Enable scroll mode while pressed                                  |
| `SCRL_STI` | `Kb 9`          | `0x7e09` | Increase ST (scroll step), higher is faster (1..7, default 4)     |
| `SCRL_STD` | `Kb 10`         | `0x7e0a` | Decrease ST (scroll step), lower is slower (1..7, default 4)      |
| `SCRL_INV` | `Kb 11`         | `0x7e0b` | Invert scroll direction                                           |

### Scroll snap
| Keycode    | Value on Remap  | Hex      | Description                                                       |
|:-----------|:----------------|:---------|:------------------------------------------------------------------|
| `SSNP_VRT` | `Kb 12`         | `0x7e0c` | Set scroll snap mode to vertical                                  |
| `SSNP_HOR` | `Kb 13`         | `0x7e0d` | Set scroll snap mode to horizontal                                |
| `SSNP_FRE` | `Kb 14`         | `0x7e0e` | Disable scroll snap mode (free scroll)                            |

### Automatic mouse layer
| Keycode    | Value on Remap  | Hex      | Description                                                       |
|:-----------|:----------------|:---------|:------------------------------------------------------------------|
| `AML_TO`   | `Kb 15`         | `0x7e0f` | Toggle automatic mouse layer                                      |
| `AML_I50`  | `Kb 16`         | `0x7e10` | Increase 50ms automatic mouse layer timeout (max 3000ms)         |
| `AML_D50`  | `Kb 17`         | `0x7e11` | Decrease 50ms automatic mouse layer timeout (min 300ms)          |

### Swipe control
| Keycode    | Value on Remap  | Hex      | Description                                                       |
|:-----------|:----------------|:---------|:------------------------------------------------------------------|
| `SW_RT`    | `Kb 18`         | `0x7e12` | Adjust swipe reset delay (Shift: decrease)                        |
| `SW_ST`    | `Kb 19`         | `0x7e13` | Adjust swipe trigger threshold (Shift: decrease)                  |
| `SW_DZ`    | `Kb 20`         | `0x7e14` | Adjust swipe deadzone (Shift: decrease)                           |
| `SW_FRZ`   | `Kb 21`         | `0x7e15` | Toggle swipe pointer freeze                                       |

### Debug
| Keycode    | Value on Remap  | Hex      | Description                                                       |
|:-----------|:----------------|:---------|:------------------------------------------------------------------|
| `DBG_TOG`  | `Kb 22`         | `0x7e16` | Toggle OLED debug mode                                            |
| `DBG_NP`   | `Kb 23`         | `0x7e17` | OLED debug page next                                              |
| `DBG_PP`   | `Kb 24`         | `0x7e18` | OLED debug page previous                                          |

[^1]: CPI, scroll step (ST), automatic mouse layer's enable/disable, and automatic mouse layer's timeout.

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
| `SCRL_STI` | `Kb 8`          | `0x7e08` | ST(スクロールステップ)を1つ上げます。値が大きいほど速い(1..7/既定4) |
| `SCRL_STD` | `Kb 9`          | `0x7e09` | ST(スクロールステップ)を1つ下げます。値が小さいほど遅い(1..7/既定4) |
| `SCRL_INV` | `Kb 16`         | `0x7e10` | スクロール方向を反転します                                       |
| `SCRL_PST` | `Kb 5`          | `0x7e05` | プリセット切替: mac={120,120}, それ以外={120,1}<->{1,1}           |

## スクロールの調整（ST とプリセット）

- ST は「スクロール速度の段階」です。中心は ST=4（基準速度）で、1..7 の範囲です。
- 数値を上げると速く、下げると遅くなります。変化は大きめに設定しています。
- プリセットは OS ごとに保存され、mac は {120,120} 固定、それ以外は {120,1} ↔ {1,1} を切替できます。
- `KBC_SAVE` で現在の OS スロットの設定を EEPROM に保存します。

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
| `AML_I50`  | `Kb 11`         | `0x7e0b` | 自動マウスレイヤーのタイムアウトを50msec増やします (最大3000ms)  |
| `AML_D50`  | `Kb 12`         | `0x7e0c` | 自動マウスレイヤーのタイムアウトを50msec減らします (最小300ms)   |

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
