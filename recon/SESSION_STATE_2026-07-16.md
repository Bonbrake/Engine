# SESSION STATE — pick up here (saved 2026-07-16, end of fact-check turn)

> Drop this into a new Hermes session (or read it) to continue ZombieEngine spec work.
> Repo: C:\ZombieEngine · Branch: spec/m0-parity-reformat · Engine: Vulkan 1.4 + 100% procedural + modding.

## WHERE WE ARE (done this session + prior)
1. **Spec consolidation COMPLETE**: 16 milestone files (M0–M13 + M4.5 + M6.5) + APPENDICES.md. ROADMAP.md (engine-first→game, vertical-slice gate), phased llms.txt, frontmatter has layer/phase/build_order + real titles. Verified 3 ways.
2. **All 25 content gaps FILLED** (5 subsections each, grounded + studio-failure guardrails from NMS/Starfield post-mortems).
3. **Deep per-milestone audit DONE**: 0 structural defects (recon/MILESTONE_AUDIT.md).
4. **Feature-completeness audit DONE**: all 374 v79-master systems present, 0 losses (recon/AUDIT_feature_completeness_2026-07-16.md).
5. **Lighting corrected per Threat Interactive gospel**: M4.5-EXT-20 = Burley/Kalisto diffuse (Lambert 1760 obsolete), Beer-Lambert kept for transmission only. CONFIRMED vs Disney 2012 BRDF paper.
6. **M11 frontmatter fixed** (ext_blocks 49→48). Committed.
7. **Fact-check report**: recon/FACTCHECK_2026-07-16.md (3-way verify, curl research).

## KNOWN OPEN ITEMS (the actual to-do list)
- **[A] M1 mega-file extraction (OPTION B)** — GATED spec edit, NOT done.
  - M1.md has 318 embedded foreign blocks concatenated MID-LINE (e.g. line 35:
    `##### Player-Facing Impact ... #### [M1-EXT-02]` on one line). Makes verify_m0_parity.py
    false-positive on M1 (says FIX(4) but content is actually 5/5 complete).
  - My autonomous extraction attempt FAILED (mid-line concatenation broke parser) → reverted.
  - NEEDS: a parser that splits on `#### [` even mid-line, OR user explicit approval.
  - BLOCKER: reliable M1 verification depends on this. Everything else verifies clean.
- **[B] Deeper Threat Interactive research — RESOLVED (fetch-verified 2026-07-16, this resume).**
  - Search rate-limit had cooled; YouTube channel `@ThreatInteractive` loaded (1.1 MB).
    Channel ID `UC2ksme4hP4Nx97ilFTQ0K0Q`; 15 videos via public RSS.
  - Captions API IP-blocked (`ip=0.0.0.0` in signed URL) → could NOT pull transcript text.
    Video TITLES + chapter lists (primary source) extracted successfully.
  - Video 20 "Fake Realism" chapter list CONFIRMS the Burley/Disney + "Callisto BRDF"
    thesis behind M4.5-EXT-20 (previously only paraphrased from se7en.ws). Fix now
    PRIMARY-SOURCE verified. ✅
  - Video 30 "Regression In Graphics" chapters ("Stop Defending Graphics That Are
    Purposefully Worse", "Reward System Of Garbage & Lies") CONFIRM the slop/LOD/streaming
    + 9th-gen-neglect thesis framing we mapped to M4/M4.5 guardrails. ✅
  - FULL RECORD: recon/THREAT_INTERACTIVE_RESEARCH_2026-07-16.md. Thesis framing verified;
    exact algorithmic specifics remain unverified (need transcript — captions blocked).
  - STILL GATED: writing the M4/M4.5 guardrail language into milestone files = spec edit →
    needs user GO (same gate as [A]/[C]/[D]).
- **[C] Duplicate merges** (from recon/dup_cohesion_scan.json + PLAN_spec_hygiene_2026-07-16.md):
  - TRUE DUPES to merge: M0-EXT-24==M0-EXT-41, M0-EXT-25==M0-EXT-40, M5-EXT-22==M5-EXT-52,
    M5-EXT-17==M9-EXT-11. (Other 3 pairs are DISTINCT — keep.)
- **[D] Naming-scheme rename** (user wants it — current Mx-EXT-NN disliked):
  - Proposed: files `NN-kebab-slug.md` (00-vulkan-bootstrap.md), block IDs `[slug]`
    ([vulkan-boot-lock]), cross-refs `[slug]`. Plan in recon/PLAN_spec_hygiene_2026-07-16.md.
  - BIG gated refactor (567 blocks + 658 cross-refs). Needs user GO.

## USER RULES TO HONOR (from memory + this session)
- Always have a BACKUP tool for every tool (web down → curl terminal; script fails → re-run
  differently). Route around failures, never block.
- After EVERY task: ASSUME YOU ARE WRONG, then PROVE it's not (read disk / re-run / cross-check).
- Fact-check 3 ways before "done". Don't fabricate — mark *(verify)* if ungrounded.
- Milestone spec files (spec/Mx.md, APPENDICES.md) are READ-ONLY gated except with approval.
- Batch rule: ≤3 files/commit. Fix simple bugs inline; flag hard ones.
- Be autonomous; don't ping for approval mid-run unless a genuine decision point.

## HOW TO RESUME
1. Read recon/FACTCHECK_2026-07-16.md + recon/PLAN_spec_hygiene_2026-07-16.md for full context.
2. Next likely action: ask user which of [A][B][C][D] to do. [A] and [D] need approval (gated).
   [B] = retry Threat fetch (try curl again, or ask user for a file path). [C] = safe to do now.
3. If continuing fact-check: re-run `python "$LOCALAPPDATA/hermes/scripts/verify_m0_parity.py"`
   and the dup_cohesion scan; M1 will still false-positive until [A] is done.

## ENV NOTES
- Web tools (web_search/web_extract) BLOCKED (no Firecrawl credits). curl terminal = working
  research path (but rate-limited after heavy use — wait if 429).
- Python via terminal works. Use POSIX paths (git-bash/MSYS) in terminal calls.
- M1.index.json regenerated by apply_deep_opt.py is fine; commit with spec edits.
