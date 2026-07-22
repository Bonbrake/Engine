# Recon — Research & Planning

This directory holds ZombieEngine's research, planning, and analysis artifacts. Everything here is informational — no source code, no build system files.

## Naming Scheme

All files follow `YYYY-MM-DD_descriptive_name.ext` for dated artifacts. Categories:

| Directory | Contents | Naming |
|-----------|----------|--------|
| `plans/` | Active canonical plans for current work | `YYYY-MM-DD_category_description.md` |
| `plans/archive/` | Superseded plan snapshots (preserved for reference) | Same as above |
| `research/` | External research, fact-checks, feature analysis, session state | `CATEGORY_description.md` or `YYYY-MM-DD_description.md` |
| `scripts/` | Python utility scripts for spec analysis, audits, translation | `snake_case.py` |
| `data/` | Structured data: JSON gap analyses, inventories, scan results | `snake_case.json` |
| `data/archive/` | Superseded data snapshots | `descriptive_name.json` |
| `talks/` | Design discussion notes, retrospectives, process reviews | `YYYY-MM-DD_descriptive-name.md` |
| `transcripts/` | YouTube video transcripts used for research | `yt_ID.en.json3` |

## Conventions

- **Active plans** live in `plans/` — at most 3-4 active files
- **Superseded plans** are moved to `plans/archive/` — never deleted
- **Research notes** cite their sources (URL, video ID, paper DOI)
- **Scripts** are standalone Python — no external dependencies beyond stdlib
- **Data files** are versioned alongside the spec they analyze
- **Dates** are `YYYY-MM-DD` for machine sortability
