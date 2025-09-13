# Repository Guidelines

## Project Structure & Module Organization
- `keyball/` — Keyboard sources. Boards: `keyball39/`, `keyball44/`, `keyball61/`. Shared code in `lib/` (project libs) and `lib_user/` (user features).
- `qmk_firmware/` — Pinned QMK tree used for builds (includes `.build/` outputs).
- `scripts/` — Helper scripts (e.g., `scripts/setup_and_build.sh`).
- `.github/workflows/` — CI that compiles firmware via the QMK CLI.

Keymaps live under `keyball/<board>/keymaps/<name>/` with `keymap.c` (optionally `rules.mk`, `config.h`). User-level shared code is under `keyball/lib_user/`.

## Coding Style & Naming Conventions
- Language: C for QMK; follow QMK’s `.clang-format` (4‑space indent, no tabs).
- Naming: snake_case for functions/vars; headers use `#pragma once`. Filenames lowercase with underscores (e.g., `keyball_move.c`).
- Place shared features in `keyball/lib/` or `keyball/lib_user/`; board‑specific code stays in the board directory.

## Commit & Pull Request Guidelines
- Commits: concise, imperative, scoped. Prefer Conventional Commits:
  - `feat(keyball44): add thumb layer toggles`
  - `fix(lib): correct wheel delta scaling`
  - `refactor(lib_user): tidy combo handling`
- PRs: include summary, affected boards/keymaps, rationale, and build proof (artifact path or CI green). Link issues. Add screenshots/layout JSON for VIA when relevant. Keep PRs focused; avoid mixing refactors with feature changes.

## Notes & Tips
- Artifacts land in `qmk_firmware/.build/` and sometimes repo root `.build/` (see CI). Set `QMK_FLOAT=1` with the script to temporarily advance QMK for testing.

- vialへの対応を進めること

- keyball_rp2040_vialはあくまで参考であり、keyball/lib_user/keymap/44/mymapをvialで正常に動作させることを目標とすること

- 返信に関してはutf-8日本語で行うこと
- コメントの付記に関しても同様
