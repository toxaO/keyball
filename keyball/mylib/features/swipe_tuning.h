// swipe_tuning.h
#pragma once


// ==== 単一方向・厳密分類の距離ベース ====
// 初回発火のしきい値（微小ゆらぎで出さないために十分な値）
#ifndef FIRST_STEP_COUNT
#  define FIRST_STEP_COUNT 20   // 200〜260 目安
#endif

// 2回目以降の等間隔しきい値（超えるたびに発火、余剰は持ち越し）
#ifndef STEP_COUNT
#  define STEP_COUNT       100   // 140〜200 目安（初回より小さめ推奨）
#endif
