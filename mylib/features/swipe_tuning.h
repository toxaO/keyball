// swipe_tuning.h
#pragma once

// ==============================
// 基本フィルタ & しきい値（X/Y共通）
// ==============================

// 微小ゆらぎのカット（counts）
#ifndef DEADZONE
#  define DEADZONE               1
#endif

// 一次ローパス強度（大きいほどマイルド）
#ifndef LP_SHIFT
#  define LP_SHIFT               2
#endif

// スワイプ起動の最初のゲート（軽くしたいなら小さく）
#ifndef START_COUNT
#  define START_COUNT            1
#endif

// 繰り返し発火の基本距離（大きいほど単発寄り）
#ifndef STEP_COUNT
#  define STEP_COUNT            280
#endif

// 1スキャンあたりの発火上限（単発寄りにしたいので 1 推奨）
#ifndef MAX_FIRES_PER_SCAN
#  define MAX_FIRES_PER_SCAN     1
#endif

// STEP を越えて更に必要な距離（単発寄りマージン）
#ifndef FIRE_MARGIN_COUNT
#  define FIRE_MARGIN_COUNT      120   // 60〜120 目安
#endif


// ==============================
// 軸ヒステリシス & ロック（縦混入防止）
// ==============================

// 優勢判定（整数比較: ay/ax <= 0.75 を水平、>= 1.33 を垂直）
#ifndef AXIS_H_RATIO_X100
#  define AXIS_H_RATIO_X100     75    // 0.75
#endif
#ifndef AXIS_V_RATIO_X100
#  define AXIS_V_RATIO_X100    133    // 1.33
#endif

// 軸切替に必要な「連続優勢」フレーム数
#ifndef AXIS_SWITCH_HYST
#  define AXIS_SWITCH_HYST       3
#endif

// 発火後に軸を固定するフレーム数（直交軸の誤検出を減らす）
#ifndef POST_FIRE_LOCK_FRAMES
#  define POST_FIRE_LOCK_FRAMES  4
#endif

// ロック中の直交軸の減衰強度（>>1 で半減）
#ifndef ORTHO_DECAY_SHIFT
#  define ORTHO_DECAY_SHIFT      1
#endif


// ==============================
// （互換）厳格分類用の比率しきい値
// classify_swipe_dir_strict() を使う場合だけ参照されます。
// コメントと値を一致させています。
// ==============================
#ifndef CLASSIFY_MIN
#  define CLASSIFY_MIN           3    // 分類する最小大きさ（counts）
#endif

#ifndef AXIS_H_LO_X100
#  define AXIS_H_LO_X100        80    // 0.80: ay/ax <= 0.80 を水平優勢
#endif
#ifndef AXIS_V_HI_X100
#  define AXIS_V_HI_X100       125    // 1.25: ay/ax >= 1.25 を垂直優勢
#endif


// ==============================
// ビルド時チェック（C11 以降）
// ==============================
#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L)
_Static_assert(START_COUNT > 0, "START_COUNT must be > 0");
_Static_assert(STEP_COUNT > 0, "STEP_COUNT must be > 0");
_Static_assert(FIRE_MARGIN_COUNT >= 0, "FIRE_MARGIN_COUNT must be >= 0");
_Static_assert((STEP_COUNT + FIRE_MARGIN_COUNT) > START_COUNT,
               "STEP_COUNT + FIRE_MARGIN_COUNT should be larger than START_COUNT");

_Static_assert(MAX_FIRES_PER_SCAN >= 1, "MAX_FIRES_PER_SCAN must be >= 1");

// 軸しきい値の整合性
_Static_assert(AXIS_H_RATIO_X100 > 0 && AXIS_V_RATIO_X100 > 0,
               "AXIS_*_RATIO_X100 must be positive");
_Static_assert(AXIS_H_RATIO_X100 < AXIS_V_RATIO_X100,
               "AXIS_H_RATIO_X100 should be < AXIS_V_RATIO_X100");
#endif
