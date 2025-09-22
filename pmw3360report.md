# PMW3360/Pointing Device 調査レポート（暫定）

本レポートは、QMK 0.27 以降の変更点を踏まえ、keyball における PMW3360 ドライバ／分割ポイントデバイス周りの現状と問題要因、そして修正方針の当たりを整理したものです。

## 背景
- 以前は `keyball/drivers/pmw3360`（自前ドライバ）を使用していたが、QMK 0.27 の Breaking Change により正常動作しなくなり、現在は QMK 公式の PMW33xx 共通ドライバに移行。
- 過去は左右どちらに USB を挿してもトラックボールが動作したが、移行後は「ボール側（USB を挿した側）でしか動かない」事象が発生。
- holykeebs/hires-scroll ブランチ（QMK 0.29.4）では公式ドライバのまま、両側 USB で理想的挙動を確認。

## 調査対象と構成
- 自前ドライバ関連
  - `keyball/drivers/pmw3360/pmw3360_pointing_device_driver.c`
  - `keyball/drivers/pmw3360/pmw3360.c|.h`, `srom_0x04.c`, `srom_0x81.c`
- キーボード共通ロジック
  - `keyball/lib/keyball/keyball.c`（分割 RPC、combined 合成、初期化ステータス判定）
- QMK 公式側（現行採用）
  - `qmk_firmware/drivers/sensors/pmw33xx_common.c|.h`（PMW3360/3389 共通）
  - `qmk_firmware/drivers/sensors/pmw3360.c|.h`（型定義など）
  - `qmk_firmware/quantum/pointing_device/*.c|.h`（ドライバ API、combined、分割共有）
  - `qmk_firmware/quantum/split_common/transactions.c`（pointing 共有の実装）

## QMK 0.27+ の主な変更点（PMW33xx/pointing/split）
- PMW33xx ドライバの共通化：`pmw33xx_common.*` に集約、`pmw3360`/`pmw3389` は別名付与。
- センサ SROM のバイナリ削除：SROM はユーザー側が `pmw33xx_srom_get_length()` と `pmw33xx_srom_get_byte()` の weak 関数で供給（未供給ならアップロード省略）。
- ポイントデバイス API の整理：`pointing_device_driver_t` の `init()` は `bool` を返す。ステータスは `pointing_device_get_status()` で取得。
- 分割共有の公式実装：`SPLIT_POINTING_ENABLE` と `POINTING_DEVICE_{LEFT,RIGHT,COMBINED}` の定義に応じ、`transactions.c` が slave 側でセンサーをポーリングし、master に CPI/レポートを共有。
- combined 用の左右別回転・反転マクロ：`POINTING_DEVICE_ROTATION_*` と `*_RIGHT` を個別に適用してから合成。

## 現状リポジトリの要点
- 現在は QMK 公式ドライバ（`POINTING_DEVICE_DRIVER = pmw3360` → 実体は `pmw33xx_common`）でビルド。
- `keyball/lib/keyball/keyball.c` は 0.27+ のステータス API に対応（`pointing_device_get_status()` を使用）。
- 分割前提の combined 合成フック `pointing_device_task_combined_kb()` を実装。
- グローバル `keyball/config.h` に以下を定義：
  - `SPLIT_POINTING_ENABLE`, `POINTING_DEVICE_COMBINED`
  - `PMW33XX_CS_PIN`、左右別回転／反転（`*_RIGHT` を含む）
  - ただし、この親 `config.h` がビルド経路によっては必ずしも取り込まれない懸念（特に vial-qmk など）。
- SROM 供給（`pmw33xx_srom_get_*` の実装）は未検出 → 公式ドライバは SROM アップロードをスキップ。

## 症状と原因仮説
### 1) USB を挿した側でしか動かない
- 仮説（強）：`SPLIT_POINTING_ENABLE` / `POINTING_DEVICE_COMBINED` の定義がビルド時に無効になっている（親 `keyball/config.h` が読まれていない）。
  - この場合、`transactions.c` の pointing ハンドラが無効化→slave 側でセンサーがポーリングされず、共有もされない→ボール側が master のときのみ動作。
- holykeebs 側は keyboards/keyball 配下の `config.h` に定義があり、確実に有効化されている可能性が高い。

### 2) 動作が固い／左右で方向が変わる
- SROM 未供給により追従や安定性が低下（固さ）。
- combined の回転・反転の適用順と、ドライバ層（自前）での座標変換が干渉（左右で方向不一致）。公式の combined フローに合わせ、座標変換は QMK 側マクロに委譲するのが安全。
- RPC で無理やり動かした場合は、公式の分割共有ロジック（スロットル、CRC一致スキップ、左右別回転の順序など）を経由せず、更新周期が粗くなるため「低頻度」に見えやすい。

## holykeebs/hires-scroll（QMK 0.29.4）との相違点の目星
- keyboards/keyball 配下の `config.h`/各 board `config.h` に、以下が確実に定義されている：
  - `SPLIT_POINTING_ENABLE` / `POINTING_DEVICE_COMBINED`（または LEFT/RIGHT）
  - `PMW33XX_CS_PIN` と必要なら `PMW33XX_CS_PIN_RIGHT`
  - 左右別回転・反転 `POINTING_DEVICE_ROTATION_*` / `*_RIGHT`
- SROM 供給（`pmw33xx_srom_get_*`）が実装されている可能性（配布可否に留意）。
- 以上により、どちら側を master にしても（USB 差替えでも）slave 側が正しくポーリング・共有され、かつ向きが一貫して正しい。

## 自前ドライバ（keyball/drivers）を 0.27+ で再活用するなら
- `pointing_device_driver_init` の戻り値を `bool` に変更し、`pmw3360_init()` 成否を返す（現状は `void`）。
- rules.mk を `POINTING_DEVICE_DRIVER = custom` に変更し、weak のフックを自前実装で上書き（公式 pmw33xx と衝突しないように）。
- 座標変換はドライバから撤去し、QMK 側の回転／反転マクロに委譲（combined での左右整合性を担保）。
- SPI/CS 周りは `PMW3360_NCS_PIN` のままでもよいが、左右別 CS が必要なら公式流儀（`PMW33XX_CS_PIN(_RIGHT)`）へ寄せると保守が容易。
- SROMは自前同梱が可能だが、配布物（vial）ではライセンス上 NG。mymap のみ同梱、配布用は未供給という切り分けが現実的。

## 公式ドライバのまま改善するなら（推奨）
1) 定義の適用位置をキーボード配下へ移設
- 各 board の `keyball/<board>/config.h` に以下を明示：
  - `#define SPLIT_POINTING_ENABLE`
  - `#define POINTING_DEVICE_COMBINED`（構成に応じて LEFT/RIGHT）
  - `#define PMW33XX_CS_PIN ...` と必要に応じて `#define PMW33XX_CS_PIN_RIGHT ...`
  - 左右別回転・反転 `POINTING_DEVICE_ROTATION_*` / `*_RIGHT`
- これで vial-qmk 等、あらゆるビルド経路で確実に有効化。

2) SROM 供給の検討（mymap 限定）
- `pmw33xx_srom_get_length()` / `pmw33xx_srom_get_byte()` を実装し、既存の `srom_0x81` などから供給（配布ビルドでは無効化）。
- 追従性・安定性（「固さ」）の改善が期待できる。

3) 右手側 CS の明示
- 実配線が左右で異なる場合は `_RIGHT` を正しく設定。holykeebs の定義と照合。

## 次アクション（提案）
- holykeebs/hires-scroll（QMK 0.29.4）版の `keyboards/keyball` 配下 `config.h`/rules を参照し、上記マクロ定義の相違を洗い出し。
- 当リポジトリでも各 board の `config.h` に split/combined/CS/回転反転を明示→vial-qmk ビルドで両側 USB 動作を再確認。
- 必要に応じて mymap にのみ SROM フックを実装（配布ビルドでは無効）し、挙動を holykeebs 相当に近づける。

## 参考：関連ファイル
- 自前ドライバ側
  - `keyball/drivers/pmw3360/*`
- キーボード共通
  - `keyball/lib/keyball/keyball.c`
  - `keyball/config.h`（現状ここに split/combined/回転反転/CS が集約）
- 公式ドライバ／分割共有
  - `qmk_firmware/drivers/sensors/pmw33xx_common.c|.h`
  - `qmk_firmware/quantum/pointing_device/pointing_device.c|.h`
  - `qmk_firmware/quantum/split_common/transactions.c`

---
最重要ポイントは「定義の適用位置（キーボード配下に明示）」「SROM供給の有無」「左右別CS/回転反転の明示」です。holykeebs 側はこれらが揃っており、結果としてどちら側 USB でも理想的に動いていると推測されます。
