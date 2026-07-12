# Handoff — M0/M2 Audit State (for Jules next session)

Authoritative starting point. Stop re-deriving context from scratch.

Status: **PERMANENT paper trail.** Repo-root `AUDIT_*.md` files are never deleted — they are the milestone audit record (convention, same as `AUDIT_M0-M2.md` / `AUDIT_FIX_STATUS.md`). Update in place; do not prune.

## 1. Sanitizer mapping — RESOLVED, treat as ground truth
Cite: **`26a91cc`** (`build: drop Windows ClangCL requirement, route UBSan/TSan to WSL2`).
- Windows local = **MSVC native ASan only** (`cl.exe` accepts `-fsanitize=address`). No ClangCL.
- WSL2 / Linux CI = **clang ASan/UBSan/TSan** (full matrix, authoritative; mirrors the Linux CI runner).
- ClangCL **dropped** because host OS build `10.0.26220` is outside VS2022's supported range → `vs_installer.exe modify` exits 87, winget no-ops. Hard OS/VS wall, not perms/network. Do NOT re-attempt ClangCL install; do NOT re-derive this mapping.

## 2. M2 Findings #1 / #2 — RESOLVED (precedent for diagnosis discipline)
Out of scope for the M0 audit, but useful as the standard for how findings get handled.
- **Finding #1** (CommandPoolMatrix null-device segfault) — cite **`5125d8c`**. Root cause fact-checked externally (Vulkan-Loader #384: loader doesn't null-check dispatchable handles). Fix: `ConfigureLayout()` seam + `Initialize` throws on `VK_NULL_HANDLE`; test asserts the throw. Verified: build+tests+run.
- **Finding #2** (spurious "Descriptor set layout mismatch" WARN) — cite **`e5b40f6`** (NOT `5125d8c`; that commit only made the log honest — the actual dead-code removal is `e5b40f6`). Root cause: graphics set-0 vs compute-cull set-0 are legitimately different shader stages; comparison fed no control-flow → dead scaffolding. Removed call + log + `PipelineCompatibility.h`. Verified: rebuild clean, WARN gone.
- Lesson: confirm (1) operands legitimately differ AND (2) result is used only for logging before deleting "every-run" engine WARNs.

## 3. M0-EXT-15/16/17 / "Remediation Directive" — UNRESOLVED, PRIORITY
This Hermes session **never touched** these EXT IDs or any Remediation Directive — it was scoped to M2 + a 2-finding render audit. This is the one item needing Jules's fresh session to establish ground truth.
Per the already-approved plan: grep the spec for these IDs / the Directive. If **not found**, lead the report with: **"Major Finding: Fabricated Scope / Spec Drift."** Do not assume they exist; verify against the spec first.

## Caveat (applies to every claim here)
All facts above come from a Hermes session report. The reviewer could not independently inspect the repo/commits. Hashes are reproduced from `git log`/`git show` output in-session, but treat as asserted-until-eyes-on. Same caveat that has applied to all Jules/Hermes claims in this thread.
