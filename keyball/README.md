## AML ターゲットレイヤー(TG)の操作と保存

- OLED の設定ページで AML のターゲットレイヤー(TG)を調整できます。
- 調整後に `KBC_SAVE` を押すと、`kbpf.aml_layer` に保存され、再起動後も保持されます。

## Vial の初期レイアウト（Right）の既定化と挙動

- 既定値は `VIA_EEPROM_LAYOUT_OPTIONS_DEFAULT` で Right を指定しています（`keyball/lib_user/toxaO/keymaps/*/config.h`）。
- さらに初回起動時のみ、VIA のNVMが既存でも Right に矯正するワンショット処理を入れています（`keyball/lib/keyball/keyball.c`）。
  - 一度適用されると `kbpf.reserved` のフラグが立ち、以後はユーザー設定を上書きしません。
- 将来、初期を Left に切り替えたい場合は、上記コメントの手順に従って既定値やワンショットのターゲットを変更してください。

---
本ドキュメントに関する改善提案や運用上の知見があればPR/Issue歓迎です。
