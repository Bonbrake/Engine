# PLAN: ti_debug.py — AI-First Diagnostics (specialized for Hermes + hy3)

> Goal: make `recon/ti_debug.py` primarily an agent-consumable verification tool, specialized
> for the TWO models that actually work on this repo: **Hermes** (this agent runtime) and
> **hy3** (tencent/hy3:free, the session model). The human-readable view is a SECONDARY thin
> layer. The PRIMARY output is a stable, versioned, schema-validated JSON "contract" an LLM
> agent can parse deterministically and act on with zero surrounding context.
> Author: Hermes (refine pass, 2026-07-16). Status: PLAN — awaiting GO to implement.

## Design principle (per user)
1. **Machine contract is the source of truth.** Default output when piped / no TTY = canonical
   JSON. Color/human text is opt-in (`--human`, auto-on TTY only).
2. **Specialized for Hermes + hy3.** The contract's `consumer` field names the target agent;
   finding objects are shaped for LLM ingest: stable `id`/`kind`, explicit `evidence` (the
   exact spec block text snippet + missing keywords), explicit `fix_hint` (what the agent
   should do next), and `locs` with file:line for both spec + plan. An LLM needs NOTHING
   outside the object to act.
3. **Stable schema, versioned.** `"schema": "ti-gate/v1"`. Validated with `jsonschema`
   (INSTALLED) before emit — the agent can trust the shape.
4. **Zero ambiguous prose in the contract.** No "body lacks X" hand-waving — exact missing
   keywords + exact block snippet + exact file:line.
5. **Streamable.** `--jsonl` = one JSON object per finding (NDJSON), stdlib only.
6. **CI-native secondary.** `--gha` / `--sarif` for the rare case a human wires it into CI.

## Authoritative sources fetched live (2026-07-16)
- GitHub Actions annotation format (docs.github.com):
  `::error file={name},line={line},endLine={endLine},title={title}::{message}`
  `::warning file={name},line={line}::{message}`
- SARIF 2.1.0 schema (json.schemastore.org): live; result = {ruleId, level, message, locations}.
- `rich` 15.0.0 (PyPI): deps markdown-it-py + pygments only. INSTALLED.
- `icontract` 2.7.3 (PyPI): `@require`/`@ensure`/`@invariant`. MISSING locally (pip install, trivial).
- `jsonschema` INSTALLED -> validates emitted contract against its own SCHEMA.
- `jsonlines` MISSING -> not needed; JSONL via stdlib.

## Primary consumers (baked into contract)
- `consumer: "hermes"`  — Hermes Agent (Nous Research) runtime; this file's own agent.
- `consumer: "hy3"`     — tencent/hy3:free (session model); reviews/acts on findings.
- The gate emits BOTH as the target set; an agent wrapper filters on `consumer` if needed.
- Because the contract is self-describing, ANY agent can consume it; Hermes+hy3 are the
  tuned primary path (their `fix_hint` wording is written for an LLM operator).

## Canonical JSON contract (schema ti-gate/v1)
```
{
  "schema": "ti-gate/v1",
  "tool": "ti_debug.py",
  "consumer": ["hermes", "hy3"],
  "exit": 0 | 1,
  "summary": {
    "claims_parsed": 22,
    "covered_partial_backed": 7,
    "gaps": 9,
    "violations": 0,
    "collisions": 0,
    "warnings": 0
  },
  "findings": [            // self-contained; an LLM acts on ONE object alone
    {
      "id": "UNBACKED",
      "kind": "unbacked",               // unbacked|false_cover|collision|unclassified|wrong_gap
      "severity": "error",              // error|warning
      "claim": "R7",
      "status": "COVERED",
      "ext": "M4.5-EXT-20",
      "detail": "no named block satisfies [['silhouette','edge','depth'],['hysteresis','variance']]",
      "missing_keywords": ["hysteresis","variance"],
      "evidence": {
        "spec_block": "#### [M4.5-EXT-20] ...Burley...",   // first ~200 chars of block
        "plan_line_text": "R7  MSAA viable... SPEC: COVERED — M4.5-EXT-11..."
      },
      "locs": {
        "spec": {"file": "spec/M4.5.md", "line": 282},
        "plan": {"file": "recon/PLAN_...gospel.md", "line": 46}
      },
      "fix_hint": "Add hysteresis+variance to M4.5-EXT-20 OR move R7 to GAP targeting a new EXT. Do NOT keep COVERED while unbacked."
    }
  ],
  "claims": [              // full parsed matrix for agent re-reasoning
    {"claim":"R7","status":"COVERED","ext":"M4.5-EXT-11","backed":true,"spec_line":282},
    ...
  ],
  "proposed_new": ["M4.5-EXT-22", ...],
  "schema_valid": true
}
```
- `--jsonl` = one line per `findings[]` entry + final summary line. Same fields.

## Upgrade modules (each independently shippable, AI-first, Hermes+hy3 tuned)

### M1. Canonical JSON by default (no TTY) — CORE
- TTY detection: terminal + no flag -> rich human. Else -> JSON contract.
- Build `findings`/`claims`/`summary` from computed data. Include `evidence` (block snippet +
  plan line text) and `fix_hint` per finding — written for an LLM operator.
- Validate vs embedded SCHEMA (jsonschema) before print; malformed -> `{"schema_valid":false,
  "error":...}` + exit 2. Never emit a broken contract.

### M2. Exact file:line + evidence (data we have)
- `load_block()` -> `(body, block_start_line)`; surface `locs.spec` + `evidence.spec_block`
  (first ~200 chars). `parse_claims()` already has `line_no` -> `locs.plan` + `evidence.plan_line_text`.
- Makes findings self-contained (principle #2).

### M3. Stable machine codes
- `id` + `kind` per finding: UNBACKED / FALSE_COVER / COLLISION / UNCLASSIFIED / WRONG_GAP.
- Agents branch on these, not prose.

### M4. `--jsonl` stream (NDJSON, stdlib)
- One JSON object per finding; final line = summary.

### M5. CI annotations (secondary)
- `--gha`: `::error file=spec/M4.5.md,line=282,title=UNBACKED R7::...` per error; `::warning::` per warning.
- `--sarif=PATH`: SARIF 2.1.0, one `result` per finding; `ruleId`=id, `level`=severity.

### M6. Self-verifying gate (icontract) — helps Hermes debug the gate itself
- `pip install icontract` (pure-python); guard import, skip if absent.
- `@require` on `load_block`: well-formed EXT id.
- `@ensure` on `parse_claims`: list of 4-tuples, ids non-empty.
- `@ensure` on `main`: `(violations or collisions) == (exit == 1)`.

### M7. Built-in `--selftest` (retires temp verify scripts)
- Positive/negative/collision/unclassified + JSON-schema-valid. tempfile, self-cleaning.
- `ti_debug.py --selftest` => exit 0 = tool healthy. Also proves the JSON contract is valid.

### M8. Human view (rich) — SECONDARY, opt-in
- `rich` (INSTALLED): color, Panel per section, Table for matrix, pretty Traceback.
- Only `--human` or TTY. Never blocks the JSON contract.

### M9. CLI ergonomics
- `--plan PATH`, `--spec DIR`, `--strict` (warnings->fail), `--quiet` (contract only),
  `--schema` (print SCHEMA + exit), `--selftest`, `--consumer hermes|hy3|both` (tags output).

## Implementation order
1. M2 (locs + evidence) — data-only.
2. M1 (canonical JSON + jsonschema validation) — core AI artifact.
3. M3 (stable codes).
4. M4 (JSONL).
5. M7 (--selftest, validates JSON).
6. M5 (--gha / --sarif).
7. M6 (icontract).
8. M8 (rich) + M9 (CLI).

## Verification (3 ways, per project rule)
- A) `ti_debug.py --selftest` exits 0 (positive+negative+collision+unclassified+JSON-valid).
- B) `ti_debug.py --json | python -c "import sys,json,jsonschema; ..."` parses + validates vs
     SCHEMA; broken temp plan -> `findings` non-empty + `exit==1` in JSON.
- C) `ti_debug.py --gha` prints `::error file=..::`; `--sarif=o.sarif` valid SARIF 2.1.0.
- D) Pipe test: `ti_debug.py | python -c "import json,sys;print(json.load(sys.stdin)['exit'])"`
     proves default-when-piped = JSON (Hermes/hy3 get the contract, not prose).

## Open dependencies (flagged)
- 4 missing transcripts (5lDkHQ1bxG0 w1OzfuqCS10 aB5qxp6SPPQ oD1cvng8SJE) STILL pulling
  (background, YouTube IP-block backoff; 11/15 at plan-write time). On landing: re-run
  analyze_transcripts.py, harvest new TI rules into gaps + extend CAP_REQUIREMENTS/claims matrix.

## Follow-up (after transcripts land)
- Re-run `analyze_transcripts.py`; diff new transcript text vs existing 11.
- Extract any TI rule/technique not yet in plan's 13-rules / 15-techniques; add GAP draft +
  gate check; extend the JSON `claims` matrix so Hermes/hy3 see it immediately.
