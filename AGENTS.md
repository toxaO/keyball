# Repository Guidelines

## Project Structure & Module Organization
- `keyball/` — Keyboard sources. Boards: `keyball39/`, `keyball44/`, `keyball61/`. Shared code in `lib/` (project libs) and `mylib/` (user features).
- `qmk_firmware/` — Pinned QMK tree used for builds (includes `.build/` outputs).
- `scripts/` — Helper scripts (e.g., `scripts/setup_and_build.sh`).
- `.github/workflows/` — CI that compiles firmware via the QMK CLI.

Keymaps live under `keyball/<board>/keymaps/<name>/` with `keymap.c` (optionally `rules.mk`, `config.h`).

## Build, Test, and Development Commands
- One‑shot setup + build (macOS/Ubuntu):
  - `bash scripts/setup_and_build.sh` (sets up venv, links keyboards, compiles default targets). Artifacts: `qmk_firmware/.build/`.
- Manual local build (example):
  - `python3 -m venv .venv && . .venv/bin/activate && pip install -U qmk`
  - `qmk setup -H ./qmk_firmware -y`
  - `ln -s ../../keyball qmk_firmware/keyboards/keyball`
  - `qmk compile -kb keyball/keyball44 -km mymap`
- Toolchain: GNU Arm Embedded (`arm-none-eabi-gcc`). On macOS: `brew install arm-none-eabi-gcc`; Ubuntu: `sudo apt-get install -y gcc-arm-none-eabi binutils-arm-none-eabi`.

## Coding Style & Naming Conventions
- Language: C for QMK; follow QMK’s `.clang-format` (4‑space indent, no tabs).
- Naming: snake_case for functions/vars; headers use `#pragma once`. Filenames lowercase with underscores (e.g., `keyball_move.c`).
- Place shared features in `keyball/lib/` or `keyball/mylib/`; board‑specific code stays in the board directory.

## Testing Guidelines
- Compile cleanly and test on hardware. Use `keymaps/test` or `keymaps/via` for quick verification.
- Clean between builds: `qmk clean -q -n -y`.
- Keep changes isolated so CI (`.github/workflows/build.yml`) can compile your target board/keymap.

## Commit & Pull Request Guidelines
- Commits: concise, imperative, scoped. Prefer Conventional Commits:
  - `feat(keyball44): add thumb layer toggles`
  - `fix(lib): correct wheel delta scaling`
  - `refactor(mylib): tidy combo handling`
- PRs: include summary, affected boards/keymaps, rationale, and build proof (artifact path or CI green). Link issues. Add screenshots/layout JSON for VIA when relevant. Keep PRs focused; avoid mixing refactors with feature changes.

## Notes & Tips
- Artifacts land in `qmk_firmware/.build/` and sometimes repo root `.build/` (see CI). Set `QMK_FLOAT=1` with the script to temporarily advance QMK for testing.
