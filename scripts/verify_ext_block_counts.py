#!/usr/bin/env python3
"""Verify milestone EXT block counts from disk sources.

Ground-truth source: spec/<ms>.md regex scan.
Derived artifact: spec/<ms>.index.json must mirror MD exactly.
Writes:
- recon/plans/EXT_BLOCK_COUNTS.md: table of milestone block counts from disk.
- Auto-regenerates any .index.json that is missing or disagrees with MD.
"""
from pathlib import Path
import json
import re
import sys

ROOT = Path(__file__).resolve().parent.parent
SPEC = ROOT / "spec"
PLAN = ROOT / "recon/plans"

required = [
    "M0", "M1", "M2", "M3", "M4", "M4.5", "M5",
    "M6", "M6.5", "M7", "M8", "M9", "M10", "M11", "M12", "M13",
]
sub_milestones = [
    "M2.6", "M2.7", "M2.8", "M2.9",
    "M4.6", "M5.1", "M5.2", "M5.3", "M5.4", "M6.5",
    "M8.5", "M8.6", "M8.7",
]

def milestone_ids_from_md(ms: str):
    p = SPEC / f"{ms}.md"
    if not p.exists():
        return []
    text = p.read_text(encoding="utf-8", errors="ignore")
    esc = re.escape(ms)
    return sorted(set(int(x) for x in re.findall(rf"{esc}-EXT-(\d+)", text)))

def milestone_ids_from_json(ms: str):
    p = SPEC / f"{ms}.index.json"
    if not p.exists():
        return []
    try:
        data = json.loads(p.read_text(encoding="utf-8", errors="ignore"))
        return sorted(
            int(x["id"].split("-")[-1])
            for x in data
            if isinstance(x, dict) and x.get("id", "").startswith(f"{ms}-EXT-")
        )
    except Exception:
        return []

def extract_title(ms: str, block_id: str):
    p = SPEC / f"{ms}.md"
    if not p.exists():
        return ""
    text = p.read_text(encoding="utf-8", errors="ignore")
    snippet_start = text.find(f"#### [{block_id}]")
    if snippet_start == -1:
        snippet_start = text.lower().find(block_id.lower())
    if snippet_start == -1:
        return ""
    snippet = text[snippet_start: snippet_start + 900]
    m = re.search(r"^####\s*\[[^\]]+\]\s*(.*)$", snippet, re.MULTILINE)
    if m:
        title = m.group(1).strip()
        if title:
            return title
    m = re.search(r"^>\s*\*\*tl;dr\*\*\s*[·\-]+\s*(.*)$", snippet, re.MULTILINE)
    if m:
        return m.group(1).strip()
    return ""

def regenerate_json_from_md(ms: str, ids):
    idx_path = SPEC / f"{ms}.index.json"
    entries = []
    for num in ids:
        bid = f"{ms}-EXT-{num}"
        entries.append({
            "id": bid,
            "title": extract_title(ms, bid) or "Systems Touched",
            "cluster": "A",
            "depends_on": [],
            "depended_by": [],
            "line": 0,
            "anchor": bid.lower(),
        })
    idx_path.write_text(json.dumps(entries, indent=2) + "\n", encoding="utf-8")
    return idx_path

def audit_milestone(ms: str):
    ids_md = milestone_ids_from_md(ms)
    ids_json = milestone_ids_from_json(ms)
    count_md = len(ids_md)
    count_json = len(ids_json)
    max_md = max(ids_md) if ids_md else 0
    max_json = max(ids_json) if ids_json else 0

    mismatched = (count_json != count_md) or (max_json != max_md) or (set(ids_json) != set(ids_md))
    status = "REGEN" if mismatched else "OK"

    if mismatched:
        regenerate_json_from_md(ms, ids_md)

    sample = []
    for num in ids_md[:3]:
        bid = f"{ms}-EXT-{num}"
        sample.append({"id": bid, "title": extract_title(ms, bid) or "Systems Touched"})

    return {
        "ms": ms,
        "json_count": count_json,
        "md_count": count_md,
        "max_json": max_json,
        "max_md": max_md,
        "status": status,
        "sample": sample,
        "json_path": str(SPEC / f"{ms}.index.json"),
        "md_path": str(SPEC / f"{ms}.md"),
    }

rows = []
warnings = []
regenerated = []

for ms in required + sub_milestones:
    result = audit_milestone(ms)
    rows.append(result)
    if result["status"] == "REGEN":
        warnings.append(f"{ms}: JSON count {result['json_count']} != MD count {result['md_count']}; regenerating .index.json")
        regenerated.append(ms)

total_json = sum(r["json_count"] for r in rows)
total_md = sum(r["md_count"] for r in rows)

out_lines = [
    "# EXT Block Count Verification",
    "> Generated from spec/*.md as source of truth; .index.json mirrors MD.",
    "",
    "| Milestone | JSON count | MD count | True max | Status |",
    "|-----------|-----------|----------|---------|--------|",
]
for r in rows:
    out_lines.append(
        f"| {r['ms']} | {r['json_count']} | {r['md_count']} | {r['max_md']} | {r['status']} |"
    )

out_lines += [
    "",
    f"**Total JSON**: {total_json}",
    f"**Total MD**: {total_md}",
    "",
    "## Warnings",
]
if warnings:
    out_lines += [f"- {w}" for w in warnings]
else:
    out_lines.append("- None")

out_lines += [
    "",
    "## Regenerated JSON sidecars",
    "",
]
if regenerated:
    out_lines += [f"- {ms}" for ms in regenerated]
else:
    out_lines.append("- None")

out_lines += [
    "",
    "## Sample titles",
    "",
    "| Milestone | Block | Title |",
    "|-----------|-------|-------|",
]
for r in rows:
    for item in r["sample"]:
        out_lines.append(f"| {r['ms']} | {item['id']} | {item['title']} |")

out_path = PLAN / "EXT_BLOCK_COUNTS.md"
out_path.write_text("\n".join(out_lines) + "\n", encoding="utf-8")

print(f"Wrote: {out_path}")
print(f"Total JSON: {total_json}")
print(f"Total MD: {total_md}")
if regenerated:
    print(f"Regenerated .index.json: {regenerated}")
if warnings:
    print("WARNINGS:")
    for w in warnings:
        print(f"- {w}")
else:
    print("Warnings: none")

action = "replace" if (ROOT / "recon/plans/EXT_BLOCK_COUNTS.md").exists() else "add"
print(f"VERIFY_EXT_BLOCKS write_action={action}")
