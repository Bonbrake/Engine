# Hermes task: system + build + runtime diagnostic bundle, TEQ engine

## Context
Repo: `C:\ZombieEngine`. Currently debugging a focus-loss/restore camera-drift
suspect in `Input.cpp`, tested via `build-asan\Release\focus_probe.exe`.
Two scripts are provided in the repo root (copy them there if not already):
- `Collect-DebugBundle.ps1` — machine/build/git/crash-evidence snapshot
- `Test-FocusProbe.ps1` — automated minimize/restore/Alt+Tab regression driver

## Run this exact sequence

1. `cd C:\ZombieEngine`
2. `.\Collect-DebugBundle.ps1 -RepoRoot C:\ZombieEngine -IncludeFocusProbeRun -ZipOutput`
3. Do not proceed to step 4 if step 2 throws before writing `manifest.json`.
   If it throws partway through, that's fine — every section is independent;
   report which sections have `Ok: false` in the manifest and move on.
4. Open `manifest.json` in the newly created `debug_bundle_<timestamp>` folder.
   Report its raw contents. Do not summarize which sections "look fine" —
   list the file for every section explicitly.
5. Cat these files verbatim into your reply, in full, no truncation, no
   paraphrase:
   - `09_git_evidence.txt`
   - `08_asan_dll_fingerprint.txt`
   - `focus_probe_run\summary.json` (if step used `-IncludeFocusProbeRun`)
6. For every other section file, report only the first 5 lines and the byte
   count (`(Get-Item <file>).Length`), not a description of what's in it.

## Evidence rules for this task (non-negotiable)

- **Raw output only.** `git diff`, `git status --porcelain`, file byte
  counts, exit codes, and verbatim log lines are evidence. A sentence like
  "the build looks clean" or "input handling is now fixed" is not evidence
  and must not appear in your report.
- **One quote of a claim per fact.** If you assert something changed, name
  the exact file and line range and show the diff line, not a paraphrase.
- **Do not mark anything "verified" or "complete"** based on this bundle
  alone. This is data collection, not a milestone check. If step 2's
  `-IncludeFocusProbeRun` produced a `FAIL` or `INCONCLUSIVE` drift verdict
  in `focus_probe_run\summary.json`, report it as-is — do not editorialize
  about likely causes unless asked.
- **If `08_asan_dll_fingerprint.txt` shows any ASan DLL in `build-asan\Release`
  with a `FileVersion`/`ProductVersion` that does not match the MSVC-bundled
  DLL found under `VC\Tools\MSVC\...`** — flag this explicitly and first,
  above everything else in your reply. This exact mismatch previously caused
  a silent `0xC0000139` crash with zero-byte logs on this machine.
- **If `11_overlay_interference_check.txt` shows Discord, RTSS, GeForce
  overlay, Steam overlay, or GameBar running** — note it, don't kill the
  process yourself.
- Do not touch `STATUS.md`, `CMakeLists.txt`, or `focus_probe.cpp`. This task
  is read-only data collection. No commits, no pushes.

## Output location
Leave the `debug_bundle_<timestamp>` folder and its `.zip` in
`C:\ZombieEngine\phase1_verify\` for consistency with prior ad-hoc
verification runs in this repo. Do not delete previous `debug_bundle_*`
folders — each run is a dated snapshot, not a replacement.
