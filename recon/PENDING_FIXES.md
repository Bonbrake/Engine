# Pending fixes (Phase 8 handoff — not blocking the format base)

Format base is committed + verified (0 structural issues, 627 blocks, all 49
reconciled systems merged). These are the remaining items, split by effort.

## HARD (authoring / content — needs your call, not a quick fix)
- **113 content-gap blocks** missing >=1 of the 5 subsections (Systems Touched /
  Math / How It Works / Reference Implementation / Player-Facing Impact).
  Full list: `recon/content_gaps.json`. M1 alone = 42 blocks missing most
  subsections (it was a skeleton). These are real authoring, deferred by design.
- **M1 duplicate/misplaced anchors**: 35 inline `<a id>` anchors embedded mid-prose
  inside the prior block's body (baseline had 77 total for 42 blocks). Verifier
  passes (standalone anchor before each header is correct) but the file carries
  35 junk duplicate IDs. Clean during the M1 content-gap fill — remove inline,
  keep one per block.

## SIMPLE (none found this pass)
- Structural sweep (sidecars present, ext_blocks matches headers, anchors correct)
  across all M*.md: ALL CLEAN. No quick wins to take.

## PHASES REMAINING (from plan)
- Phase 9 — Consolidation (APPROVAL-GATE): M4.6->M4, M5.x->M5, M2.x->M2,
  M8.x->M8, APPENDIX_*->1. Ripples into llms.txt, sidecars, AGENTS.md, cross-refs.
- Phase 10 — Final llms.txt hardening (reference only).
