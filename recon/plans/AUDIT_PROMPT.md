# ZombieEngine Professional Plan — Full Audit Prompt

Run this entire checklist in one session. Do not skip steps. Assume every prior claim is wrong until you prove it with real tool output. Fix what you find, commit only after verification, and end with a clean tree.

## Context
- Repo: `C:\ZombieEngine`
- Branch: `spec/m0-parity-reformat`
- Plan file: `recon/plans/2026-07-20_RESEARCHED_PROFESSIONAL_PLAN.md`
- Execution map: `recon/plans/APPENDIX_EXECUTION_MAP.md`
- Build system: CMake + Ninja + MSVC 14.44 via `scripts/build_ze.cmd build-ninja`
- Test binary: `build-ninja/tests/ZombieEngineTests.exe`
- Sanity suite: `build-ninja/sanity_suite.exe` (zero-dependency stdlib-only C++20)
- Research cache: `C:\Users\jakeb\AppData\Local\hermes\cache\web`
- Debug verifier: `scripts/verify_plan.py` (20-item checklist)

## Step 1 — Verify repo state
```bash
cd /c/ZombieEngine
git status --short
git log --oneline -5
```
Tree must be clean before you start. If not, investigate before editing.

## Step 2 — Build and test everything
```bash
cd /c/ZombieEngine
scripts/build_ze.cmd build-ninja
./build-ninja/sanity_suite.exe; echo "SANITY_EXIT=$?"
./build-ninja/tests/ZombieEngineTests.exe --reporter compact
python scripts/verify_plan.py
```
Record exact output. If any fail, stop and fix before continuing.

## Step 3 — Full plan structure audit
Read the entire plan file. Verify ALL of these exist and are complete:
- `## 1. Executive Summary`
- `## 2. Research Foundation` with subsections `### 2.1` through `### 2.9`
- `## 3. Design Pillars`
- `## 4. Architecture Decisions`
- `## 5. Milestone Map`
- `## 6. Technical Architecture`
- `## A. Pass 4 — task-to-paper mapping audit`
- `## B. Pass 5 — EXT ID collision audit`
- `## C. Standalone sanity suite result`
- `## D. Remaining [X] blocker fetch evidence`
- `## E. 5-pass P5 status`

If any section is missing or empty, add it with real content.

## Step 4 — Verify task and paper counts
- Count unique T-IDs: must be exactly 70 unique `T-01` through `T-70`
- Count unique research papers: must be exactly 50 unique entries
- Count unique EXT IDs in execution map: must be exactly 51
- Verify NO `|||` malformed table rows exist anywhere
- Verify NO truncated year cells like `| 202 |` instead of `| 2023 |`

Use Python/terminal to count. Do not eyeball it.

## Step 5 — Verify all [S] cache claims
Every `[S]` claim in the plan must point to a real cached file that actually contains the claimed content. For each `[S]` entry:
1. Extract the cache filename(s)
2. Check the file exists in `C:\Users\jakeb\AppData\Local\hermes\cache\web`
3. Verify the file contains at least one token from the claimed topic
4. If ANY `[S]` claim is unverifiable, downgrade it to `[X]` and note why

Known downgraded claims to verify:
- UE5 Nanite/Lumen/World Partition → `[X]`
- Frostbite rendering stack → `[X]`
- Unity DOTS archetype storage → `[X]` (wait — this was fixed, verify the exact quote is in the plan)
- GameNetworkingSockets networking → `[X]`
- UE4SS mod signature scanning → `[X]`
- Any SlideShare-backed lesson content → `[X]`

## Step 6 — Fix known errors
Fix these if still broken:
1. Paper 26 row must read: `Breaking Good: Fracture Modes for Realtime Destruction | 2023 | [cache: crossref_10.1145_3549540.json]`
2. Paper 35 must reference `arxiv_1804.02717.html` (DeepMimic), NOT `crossref_10.1145_3213779.json`
3. All `ue5_docs.html` references must use clean format without embedded parentheticals inside `[cache: ...]`
4. GSound entry must NOT use DOI `10.1145/1273440` (that DOI resolves to ACM SIGARCH Computer Architecture News). Use the actual GSound source: AES Audio for Games 41st paper, cached as `gsound_aes41st.pdf` + `gamma.umd.edu/publication/380` page.
5. M9 vehicle/traction/damage must cite real sources found in cache: `arXiv:2304.05045` (Scalable Real-Time Vehicle Deformation), SIGGRAPH 2009 Parker/O'Brien destruction paper, or SIGGRAPH 2020 AnisoMPM damage paper.
6. Unity DOTS archetype quote must be exact from `unity_ecs_core.html`: "The archetype of an entity determines where ECS stores the components of that entity. ECS allocates memory in 'chunks', each represented by an ArchetypeChunk object."

## Step 7 — Verify execution map
- Must state exactly `51 unique provisional EXT IDs`
- Must have evidence table with columns: ID | Title | Readiness | Evidence | Blocker | Source
- Must contain T1-A render entries: `M1-EXT-11`, `M4.5-EXT-26`
- Must contain T1-B world entries: `M4-EXT-08`, `M4-EXT-23`
- Must contain T1-C physics entries: `M2-EXT-57`, `M9-EXT-22`
- Must contain T1-D misc entries: `M6.5-EXT-13`, `M4-EXT-89`

## Step 8 — Verify docs and scripts
- `CMakeLists.txt` must contain `add_executable(sanity_suite` and `enable_testing()`
- `docs/architecture/README.md` must have `## Research cache` section
- `docs/guides/README.md` must have `### Research cache` section
- `scripts/fetch_research.py` must have `def fetch_vehicle_sources()` and `def fetch_gsound_sources()`
- `.gitignore` must contain `build-ninja/`
- `sanity_suite_harness/sanity.cpp` must exist and contain real tests

## Step 9 — Run full verifier
```bash
cd /c/ZombieEngine
python scripts/verify_plan.py
```
Must report `20/20 checks passed`. If not, fix the failing items.

## Step 10 — Final verification pass
Re-run these independently and record exact output:
1. `python scripts/verify_plan.py`
2. `./build-ninja/sanity_suite.exe`
3. `./build-ninja/tests/ZombieEngineTests.exe --reporter compact`
4. `git status --short`
5. `git log --oneline -3`

All must be clean/green. Only then commit.

## Commit rules
- Stage with `git add -u` plus any new files explicitly
- Commit message format: `area: short description of what changed`
- Push to `origin spec/m0-parity-reformat`
- Do NOT auto-merge PRs; just offer to open one

## Output format
When done, report:
- Exact commit hash pushed
- Number of issues found and fixed
- Number of issues verified as already correct
- Any remaining open items with exact evidence
- Final state of: tree clean, build green, tests green, verifier green

## Hard rules
- Never claim fixed without tool proof
- Never edit-and-pray; read actual file content before/after every edit
- Never trust a print statement — verify with grep, Python, or file read
- Never skip the 3-pass verification mindset: assume wrong, disprove 3 ways, then declare clean
- If you cannot find a real source, mark it `[X]` with exact HTTP evidence of the failure
- Do not sanitize M-rated ZombieEngine content
