# SESSION RESUME SNAPSHOT — TI-Gospel Plan Work (2026-07-16)

> STOP POINT. Next session reads this + the referenced files and continues. No context
> re-derivation needed. Branch: `spec/m0-parity-reformat`.

## WHAT IS DONE (committed)
1. **11 Threat Interactive transcripts** extracted to `recon/transcripts/yt_*.txt` (primary
   source). 4 still missing (see OPEN).
2. **TI-gospel plan**: `recon/PLAN_threat_interactive_gospel_2026-07-16.md`
   - 13 RULES (R1–R13) with REAL spec EXT IDs + accurate COVERED/PARTIAL/GAP status.
   - 15 TECHNIQUE RULES (T1–T15) mined from transcripts, each mapped to EXT or marked GAP.
   - Material Maker (MIT, free, procedural) chosen as the material tool.
3. **Gap-fill EXT proposals**: `recon/PLAN_gap_fill_ext_proposals_2026-07-16.md`
   - Ready-to-paste EXT drafts for every GAP (R2/R3/R4/R5/R6/R9/R11 + T1/T3/T4/T5/T8/T12).
   - These are PROPOSALS — milestone spec files are READ-ONLY to the agent; need user GO
     before pasting into spec/M4 / M4.5 / M4.6.
4. **Mechanical bug-catchers (committed)**:
   - `recon/verify_ti_plan.py` — reads real spec block bodies, fails unbacked claims.
   - `recon/ti_contract.py` — DESIGN-BY-CONTRACT gate: parses plan claim→EXT pairs, fails
     if a COVERED claim's spec block lacks the capability keywords. Prevention, not detection.
   - `recon/analyze_transcripts.py` — 3-pass transcript extractor (tech/fault/number).
   - `recon/transcript_analysis.json` — extracted data (committed this session-end).

## KEY CORRECTIONS MADE (so next session doesn't re-litigate)
- **R1 (Burley) + R10 (translucent) are COVERED** by M4.5-EXT-20 (Burley/Kalisto diffuse +
  Beer-Lambert). Not gaps. Spec author used CANONICAL Disney/Filament Burley form — verified.
- **5 false "covered" mappings were caught by the verifier** and re-marked GAP:
  T1 (mip BRDF), T3 (MSAA stencil), T4 (RGB10A2 normals), T5 (shadow stencil), T8 (SSS LUT).
  These are GENUINE GAPS — their EXT drafts are in the gap-fill file.
- **M4.5-EXT-11 is a depth-Laplacian UPSCALER-edge aid, NOT MSAA/stencil** (T3 gap confirmed).
- **M4.5-EXT-13 has a known BUG-FLAG** in spec (inverted/empty VSM overlap test) — noted, not fixed.
- **M4.5-EXT-19 is Heiligenschein wet-surface only** — no SSS LUT, no mip-filter (T1/T8 gaps).

## OPEN ITEMS (resume here)
- [~] **Pull 4 missing transcripts** — IDs `5lDkHQ1bxG0 w1OzfuqCS10 aB5qxp6SPPQ oD1cvng8SJE`.
  Live probe 2026-07-16 (resume) returned `IpBlocked` (datacenter egress still in cooldown;
  VPN makes it worse — "sign in to confirm you're not a bot"). Started
  `python recon/pull_ti_transcripts.py` in BACKGROUND with 6x backoff — it self-retries and
  grabs the 4 once YouTube lifts the block. Skip-list already resolves to exactly those 4 IDs.
  After it lands: re-run `analyze_transcripts.py` + re-verify plan/contract gate still accurate.
- [x] **PIPELINE SEAM FIX (done, uncommitted at resume)** — `pull_ti_transcripts.py` used to
  write to `recon/transcripts_api/<VID>.txt`, which `analyze_transcripts.py` NEVER reads (it
  globs `transcripts/yt_*.txt`). Silent seam: every successful pull fed nothing. Patched
  puller to write `transcripts/yt_<VID>.txt` (correct prefix + dir). Compile-checked; skip-list
  verified to resolve to the 4 missing IDs. Commit as its own batch.
- [ ] **Web independent-check of TI claims** — online egress still throttled/bot-blocked
  (DDG/Bing/selfshadow). Burley verified via Filament. Lazarov specular-AA formula still NOT
  online-verified — flag for re-check when egress recovers.
- [ ] **User GO to paste gap-fill EXTs into spec** (read-only gate). When approved, paste
  from `PLAN_gap_fill_ext_proposals_2026-07-16.md` into correct Mx.md following 5-subsection
  skeleton + `<a id>` anchor. Verify ascending IDs, no dup defs, balanced fences.
- [ ] **Web independent-check of TI claims still BLOCKED** (DDG/Bing/selfshadow bot-blocked,
  egress throttled). Burley math verified via Filament. Lazarov specular-AA formula NOT yet
  online-verified — flag for re-check when egress recovers.
- [ ] **Run `ti_contract.py` after ANY plan edit** — exit 0 = safe to call done.

## RESUME COMMANDS (copy-paste)
```bat
cd C:\ZombieEngine
git log --oneline -3
python recon/ti_contract.py          # gate: exit 0 = plan claims backed by spec
python recon/verify_ti_plan.py        # secondary check
python recon/analyze_transcripts.py   # re-extract transcripts if 4 new ones added
ls recon/transcripts/yt_*.txt | wc -l # should be 11; goal 15
```
To pull missing transcripts (after IP cooldown, VPN OFF):
```bat
cd C:\ZombieEngine\recon
python pull_ti_transcripts.py
```

## FILE MAP (what to read)
- Plan: `recon/PLAN_threat_interactive_gospel_2026-07-16.md`
- Gap drafts: `recon/PLAN_gap_fill_ext_proposals_2026-07-16.md`
- Gate tool: `recon/ti_contract.py`  |  Verifier: `recon/verify_ti_plan.py`
- Transcripts: `recon/transcripts/yt_*.txt` (11)  |  Extractor: `recon/analyze_transcripts.py`
- User directive reminder: "Threat Interactive is gospel — it's the rule." TI claims are
  HARD CONSTRAINTS on the renderer, not suggestions.
