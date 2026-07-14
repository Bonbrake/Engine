# Agent Rules — The Endless Quarantine (Jules)

The versioned spec document (v73+) is the single source of truth for every
milestone. The rules below were originally written for Antigravity but apply
identically to Jules — same repo, same standards, no exceptions.

---
[Antigravity Rules — The Endless Quarantine
This file is a persistent Rule, not a milestone doc. Antigravity agents do not carry context between sessions/Tasks — only Rules do (per Antigravity's own docs). Load this file as a workspace Rule so every session, on every milestone, starts from the same ground truth instead of re-deriving it from scratch or guessing.

Model policy: REASONING_MODEL = Gemini 3.1 Pro (thinking_level: high) for planning, feature implementation, and anything touching cross-file/cross-system reasoning. FAST_MODEL = Gemini 3.5 Flash (thinking_level: low) for mechanical/boilerplate steps (renames, formatting, repetitive scaffolding). Don't use Flash for anything that requires holding milestone-wide context — that's what causes scope drift.

Specificity protocol — non-negotiable, applies to every milestone
Every number in the milestone docs is authoritative, not a suggestion. Don't round, don't "improve," don't substitute a library default instead.
"Propose, confirm" means stop, not proceed-with-a-default. Every "Decisions to flag" line in a milestone file is a hard checkpoint — surface it and wait.
API surfaces get verified, not recalled. Check the actual signature against installed package headers (vcpkg) in the sandbox before calling anything — the sandbox can do this even when it can't run the graphical build.
Missing context is a stop condition, not a fill-in-the-blank. If a milestone file doesn't specify something needed, stop and ask rather than inventing an interface.
When in doubt about scope, under-scope and flag it. Do not silently expand a task to "while I'm in here" adjacent files.
Repo hygiene — read before touching git
These exist because they've already gone wrong on this repo (Ltmonkeysmash/Engine):

Never stage or commit build/ or vcpkg_installed/. Confirm .gitignore covers both before the first commit of a session. If a commit ever includes either path, stop and unstage — do not "fix it in a follow-up commit."
Confirm the active branch before writing anything. State the branch name back at the start of a session. Never assume main/the last-used branch is still checked out.
Touch only files the current milestone's Implementation Steps or EXT items name. If a fix genuinely requires touching an out-of-scope file (e.g. Application.cpp, Renderer.h from an unrelated milestone), stop and flag it as a cross-cutting change instead of folding it into the current commit.
Never reconstruct file content from another branch's history to "fill in" a gap. If a file's current content is unclear or seems wrong, say so and ask — don't fabricate plausible-looking source.
STATUS.md / CHANGELOG.md are the durable, session-independent source of truth for where the project actually stands. On a fresh session (new Task, session TTL expiry, or a Rules update), read STATUS.md first. Update it before ending a session that changed milestone state.
Dependency versions are locked in vcpkg.json. Don't bump a version to "latest" without it being an explicit milestone item (e.g. the Jolt 5.5.0 / EnTT 3.16.0 bumps that are already spec'd).
Tooling & developer iteration loop
Debug-draw is a first-class system — the immediate-mode debug-line/shape renderer built in M1 stays available and gets used, not bypassed with printf-debugging.
Render graph / pass dependency management is owned by M1; don't hand-roll a second barrier-insertion path in a later milestone.
Two job systems, not four. Jolt's JobSystemThreadPool (M2) is dedicated to physics only. Everything else uses the one general-purpose task scheduler (enkiTS) built at M0. Don't introduce a third scheduler for convenience.
Shader toolchain: HLSL + DXC, targeting SPIR-V. No GLSL-via-glslang side path.
Pipeline cache persists to disk (M0) — don't rebuild it from scratch every boot to "keep things simple."
Profiler of record: Tracy.
Confirm RenderDoc-capturable at M0 and don't regress it later.
Unit tests for pure-logic systems: Catch2 or doctest, vendored via vcpkg.
Minimum viable failure-mode handling: VK_ERROR_DEVICE_LOST and out-of-memory conditions log the failure and exit cleanly — never a silent hang or an unguarded crash with no artifact.
Target hardware — every budget below is against this, not against the dev machine
Recommended (dev reference machine): RTX 2070 Super, Ryzen 5700G, 16GB RAM — Tier-1/ Tier-2 quality (RT shadows, ReSTIR GI) is built and profiled against this, at 60 FPS.

Floor (Tier-0 minimum spec): the whole same-generation GPU class, not one card — RTX 2000-series (binding case: RTX 2060, 6GB VRAM), RX 6000-series (binding case: RX 6600, 8GB VRAM), or Intel Arc equivalent (binding case: Arc A580, 8GB VRAM). 6GB VRAM (from the RTX 2060) is the tightest common constraint and what every Tier-0 memory budget is built against — not the higher 8GB figures from the other two families. CPU floor: a 6-core/12-thread chip contemporaneous with that GPU generation (Ryzen 5 3600 / Intel i5-10400). Floor target: 30 FPS (33.3ms/frame), not 60.

Features not universally supported gate behind a capability check with a Tier-0 fallback — never a hard dependency on an optional extension.

Design pillars, in priority order:

Procedural variety over authored content, everywhere it's viable.
One system, fed from multiple places — not one system per feature.
Cost scales with what's near/visible, not with total world/population size.
Best-looking-for-the-cost, not maximal.
Reuse proven, battle-tested defaults for infrastructure that isn't the game's creative focus (networking transport, physics solver, etc).
Tier your ambition, don't assume it.
Performance & resource budgets (Tier-0 floor, 33.3ms/frame)
Render: ~14.0 ms
Physics: ~5.0 ms
AI: ~5.0 ms
Animation: ~3.0 ms
Audio: ~1.0 ms
Headroom: ~5.3 ms
Memory: ~4.0GB VRAM engine baseline (single-viewport) / ~3.3GB (split-screen) on the 6GB Tier-0 floor. 12.0GB system RAM. These are design-time estimates — verify against actual Tier-0-class hardware once the render path exists; don't treat as profiled fact until then.
Numerical method budget (don't upgrade these to per-tick/per-entity without a stated reason — the coarse update rate is intentional, not a placeholder):

Scent Dispersion (M5.3) / Fear Diffusion (M5.4): coarse 2D grid, updated every 3–6 ticks.
Regional SEIR Curve (M8): coarse per-region, updated once per in-game hour.
Faction Population ODEs (M8.5): per-faction scalars, updated per in-game hour.
Settlement Truss Solve (M8.6): small node count, event-driven.
Groundwater PDE (M10): shared 2D flooding heightmap, updated every 6 ticks.
Day 0 — architecture risk spikes (run before/alongside M0 if not already done)
Spike A — Determinism: standalone console program, fixed literal seed, XOR particle states into a running 64-bit hash. Run twice, alternate compilation flags. If divergence occurs, M2.8 uses snapshot-sync as the primary co-op strategy instead of deterministic lockstep.
Spike B — World precision: minimal SDL3 window + Vulkan swapchain. Authoritative position as glm::dvec3. GPU processes renderPos = (glm::vec3)(entityPos - cameraPos). Track worldOriginOffset for rebasing at >500.0 units.
Spike C — Tier fallback switch: query ray-tracing-pipeline via vkb::PhysicalDeviceSelector. Add a runtime toggle forcing supportsRtPipeline = false to verify the pure rasterized fallback actually works, not just detects.
Spike D — Destruction at scale: Jolt boxes held by FixedConstraint. Sever constraints, run BFS graph search. Measure time at the target entity count.
ID scheme
Every extended system is [Mx-EXT-nn] (e.g. [M1-EXT-06]) — stable, load-bearing IDs. Reference them by ID, not by description, when talking about a system across files or sessions. If a milestone file's Extended Systems Library has a gap in the numbering (e.g. M0 has no EXT-13), that's an intentional retired ID from a past renumber, not a missing system to invent — the milestone file's own "completeness audit" note will say so.

Milestone file sequencing
One milestone file per Antigravity session/Task. Don't load two milestone files into one session. Don't proceed past a milestone's Exit Criteria until every item passes by actually running the build — reading the code and reasoning that it should work is not verification. When Exit Criteria pass, start a fresh session and load the next file in sequence; update STATUS.md first.

## Verify-Then-Push Gate

(Permanent standing rule for ALL agents on this repo — Hermes, Jules, Antigravity.
Every session, every milestone, must load and enforce this. Durable version;
do not drop the persistence step in any redraft.)

Baseline content hash: <fill in after first hash-object below>

═══════════════ STEP 0: BOOTSTRAP (sole exception to the gate) ═══════════════
- The AGENTS.md edits in this bootstrap (BOTH the initial add AND the
  baseline-hash follow-up commit) are the ONLY commits exempt from the gate below.
- Add this "Verify-Then-Push Gate" section to AGENTS.md verbatim, INCLUDING the
  literal line:
      Baseline content hash: <fill in after first hash-object below>
  This baseline line is what makes the session-start check enforceable in a
  fresh session — it must live IN the file, not just in chat.
- Commit + push the initial add explicitly:
    git add AGENTS.md
    git commit -m "docs: add Verify-Then-Push Gate to AGENTS.md"
    git push origin <current-branch>
  Paste literal output of all three.
- Capture the canonical content hash (everything EXCEPT the baseline line, so
  the file can legitimately describe its own content without the quine problem):
    grep -n "Verify-Then-Push Gate" AGENTS.md
    git show HEAD:AGENTS.md | grep -v "^Baseline content hash:" | git hash-object --stdin
  Paste both. Then fill the "Baseline content hash" line with that hash-object
  output, commit as a follow-up ("docs: record gate baseline content hash"),
  and re-run the grep + hash-object command to confirm it CONVERGES (the value
  is identical before and after filling the line, because the line is excluded
  from the hash). Paste the final, matching state.

═══════════════ THE GATE ═══════════════

STEP 1 — TARGET & PROOF FORM
  - `git branch --show-current` — paste it.
  - Target = origin/<current-branch> (feature/milestone branch), NOT main.
  - Proof of landing (step 8) = `git log origin/<branch> -1`. If the task
    involves opening a PR, also run `gh pr create` (or equivalent) and paste
    the returned URL — do not claim a PR URL as proof unless a PR-open step
    actually ran in this session.
  - If a task ever legitimately targets main directly, that must be stated
    and confirmed explicitly before step 1 is considered satisfied — never
    assume main.

STEP 2 — BUILD
  - First: `git status --porcelain` and `git diff --name-only` — paste both.
    This is the exact file set "changed files" refers to for the rest of
    this step and Step 5's scope check. Resolve any pre-existing stash
    (`git stash list` must be empty) before proceeding — if one exists,
    stop and report it rather than stacking a new stash on top.
  - Real verify build = build-asan/Release (MSVC, NOT LLVM; per repo setup the
    ASan DLL lives in build-asan/Release/).
  - Full rebuild of the Catch2/test target:
      cmake --build build-asan --config Release --target ZombieEngineTests
    Paste exit code. (ZombieEngineTests is the real Catch2 target linked against
    ZombieEngineCore, established in-thread. If CMakeLists does NOT define that
    target, fall back to ALL_BUILD and STATE you did so.)
  - Attempt Debug too, classified as COMPILE-ONLY-OR-SKIP: the known
    __imp__CrtDbgReport/__imp__calloc_dbg/__imp__free_dbg linker signature is
    an ALLOWED, LOGGED exception — not a gate failure. Any OTHER Debug failure
    signature IS a real gate failure.
  - Re-confirm pre-existing status via scoped stash, using the ACTUAL changed
    source files captured above (not milestone doc paths — the Debug link
    failure lives in engine code, so stashing docs proves nothing):
      git diff > /tmp/pre-stash.diff
      git stash list                      # must be empty (resolved above)
      git stash push -- <exact paths from `git diff --name-only` above, no globs>
      cmake --build build-asan --config Debug --target ZombieEngineTests   # paste result
      git stash pop
      git diff > /tmp/post-stash.diff
      diff /tmp/pre-stash.diff /tmp/post-stash.diff   # must be empty
    Paste all output. Non-empty diff-of-diffs = STOP, round-trip corrupted
    something, report exactly what changed before doing anything else.
  - EMPTY-CHANGED-SET GUARD: if `git diff --name-only` (tracked) is empty — i.e.
    only untracked/reserved files changed — the scoped stash re-confirm is N/A:
    note "stash re-confirm: N/A (no tracked edits)" and SKIP the stash push/pop.
    Do NOT run `git stash push --` with an empty path list (it errors: "No pathspec").

STEP 3 — FULL TEST SUITE
  - Run on build-asan/Release:
      cd build-asan && ctest -C Release --output-on-failure
    (or invoke the built ZombieEngineTests Catch2 executable directly from
    build-asan/Release/).
  - ALL GREEN: paste command + exit code + summary line only (no firehose).
  - ANY failure/unexpected skip: paste FULL verbose output for just those
    cases, with enough context to diagnose.

STEP 4 — RUNTIME CHECK
  - If gameplay-visible: launch interactively, exercise the actual changed
    behavior, report what was observed. Skip with explicit reason if the
    change is non-visual (refactor, config, test-only).

STEP 5 — STATIC CHECKS (falsifiable)
  - Grep SPECIFIC known telltales, not "no dead code" generally:
      grep -rn "TODO\|FIXME\|XXX" <changed files>
      grep -n "<known duplicate-block signature from this milestone's history>" <changed files>
    Paste output, state pass/fail per pattern.
  - Paste compiler warning count from the build log.
  - ID/spec-collision check (standing rule): grep this change's milestone
    ID/EXT tag against milestones_M0-M13_antigravity/ (confirm this path
    exists before relying on it — if the directory has moved, find the
    real one first; a grep against a nonexistent path is not a passed check).
    Paste output, confirm uniqueness.
  - `git status --porcelain` — paste it. Every listed file must be within the
    milestone's named scope (repo-hygiene rule). Flag and justify anything
    outside scope — don't silently include it. Reserved/scratch filenames
    (nul, con, prn, aux, or similar) must never be staged — if one appears
    in the status output, exclude it and flag it, don't commit it.

STEP 6 — DIFF REVIEW + SECRET SCRUB + STATUS UPDATE
  - `git diff` for every file being committed — paste full diff. For any
    single file over ~500 lines of diff, paste the first/last ~100 lines
    plus a summary of what's in between rather than the full firehose,
    and say you're doing so.
  - Scan for keys/tokens/passwords before pasting; redact and flag if found.
  - Update STATUS.md/CHANGELOG.md for the milestone state change — paste that diff too.

STEP 7 — APPROVAL (split: commit, then push, separately)
  - Present commit message + exact `git add`/`git commit` command + target branch.
  - WAIT for explicit "commit approved."
  - After commit lands, paste hash. Present exact `git push` command separately,
    WAIT for explicit "push approved" before running it.
  - While waiting on either approval, do NOT idle — continue other in-scope
    work within the SAME milestone (standalone spikes, docs, test-writing).
    Do not load a second milestone file into this session (standing rule:
    one milestone file per session). Report back on the parked task's
    approval status whenever you resume it.

STEP 8 — POST-PUSH VERIFICATION + REJECTION HANDLING
  - On success: `git fetch origin && git log origin/<branch> -1` — paste,
    confirm hash matches. If a PR was opened in Step 1, confirm its URL here.
  - On REJECT (non-fast-forward): STOP. No force-push, no silent rebase/merge.
    Report the rejection, show what changed upstream, re-run the ENTIRE gate
    from Step 2 against the rebased/merged state before pushing again.

═══════════════ FAILURE HANDLING ═══════════════
Any Step 2–6 failure (excluding the logged pre-existing Debug-link exception)
= STOP, report exactly what failed with literal output, no commit/push. Fix,
re-run the full gate from Step 2 — don't patch around one failing check or
resume mid-gate.

═══════════════ SESSION-START PROOF (every future session, every agent) ═══════════════
Before any push-related work:
    grep -n "Verify-Then-Push Gate" AGENTS.md
    git show HEAD:AGENTS.md | grep -v "^Baseline content hash:" | git hash-object --stdin
Paste both. Confirm the hash-object output matches the "Baseline content hash"
line recorded inside AGENTS.md itself. If it does NOT match, AGENTS.md changed
since bootstrap — explain the delta, and if the change was legitimate, update
the Baseline content hash line as part of that change (and re-run this proof).
A mismatch is a REAL signal, not noise — do not normalize it.
