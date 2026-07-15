# Archive: ZombieEngine Repo Cleanup Session (2026-07-15)

**Branch:** `beta` (pushed to `origin/beta`, SHA `688451c`)
**Outcome:** repo tidied from ~75 root files → 9 legit files; 12 atomic commits; all verified 3 independent ways; code fixes runtime-verified via test suite + headless engine smoke.

## Standing rules established this session (now in memory HARD RULES)
1. **3-DEBUG** — before any permanent action (commit / memory write / delete / push), verify THREE independent ways. Never fewer than 3.
2. **LOOK ONLINE FIRST** — when uncertain, `web_search` before acting; decide from best practice, never guess.
3. **No silent self-edits** — show literal diff + revert pending review.
4. **Be autonomous** — fix as you go, don't stop to ask.

## Commits (chronological, all on `beta`)
| SHA | Message |
|---|---|
| `c181ea2` | chore: remove tracked runtime artifacts, relocate one-off scripts to scripts/ |
| `227fb9a` | docs: move design docs to docs/, add README |
| `9132c49` | build: add standalone focus_probe target [M0-EXT-15] |
| `b591acf` | docs: relabel occlusion-query comments [M1-EXT-03] -> [M1-EXT-28] |
| `df54f0c` | feat: scaffold engine subsystem stubs (ai/audio/modding/net/save/slm/ui/world) |
| `afd58db` | docs: track the remaining M0-M2 audit trail |
| `c3a6462` | chore: gitignore timestamped milestone backup snapshots |
| `907747e` | fix: avoid signed-shift UB in SpatialHashKey |
| `4590a0b` | fix: reap in-flight async collision bakers, stop enkiTS leak [M2-EXT-01] |
| `6baab5a` | fix: poll continuous keyboard state, add test hook, drop unused hasScript() [M0-EXT-15] |
| `7271904` | docs: expand M0-EXT spec blocks (add EXT-15..29, resolve reconstructed stubs) |
| `688451c` | docs: track Hermes spec-navigation bridge + M1 reconstructed-ID record |

## Verification highlights
- **Code fixes build-verified:** `cmake --build build-asan --config Release` → `BUILD_EXIT=0`, 0 errors.
- **Test suite runtime-verified:** rebuilt `ZombieEngineTests.exe` (fresh post-fix), ran → **17 passed, 1 skipped (SDL_VIDEO headless — environmental), 0 failed, 69 assertions, exit 0**.
- **Engine headless smoke:** `--headless --quit-frame 60` → `HEADLESS CI SMOKE TEST: SUCCESS`, `ENGINE_EXIT=0`. Only GOG/OBS/RTSS overlay-layer warnings (environmental).
- **Push verified 3 ways:** local HEAD `688451c…` == `git ls-remote origin beta` tip (exact match); `origin/beta` tracking set.

## Key decisions
- MASTER_PLAN `64/89` diff was a **strict superset** (added M0-EXT-15..29, resolved 3 RECONSTRUCTED stubs; 350→365 unique block IDs, zero lost, zero dupes, fences balanced) → committed, not held.
- `.hermes.md` = live spec-nav bridge → tracked (like AGENTS.md), not gitignored.
- `spec/_m1_reconstructed_ids.json` = audit record → tracked.
- Backups (`milestones_M0-M13_antigravity_bak-*`) → gitignored (kept on disk as safety net; milestones/ is tracked in git).

## Final state
- Root: 9 legit files (`.clang-tidy`, `.gitignore`, `.hermes.md`, `AGENTS.md`, `CMakeLists.txt`, `README.md`, `STATUS.md`, `vcpkg.json`, `vcpkg-configuration.json`).
- `git status` clean. `beta` up to date on `origin`.
- Full session also persisted in Hermes local session DB.
