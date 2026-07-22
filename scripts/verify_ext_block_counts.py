#!/usr/bin/env python3
"""Verify milestone EXT block counts from disk sources.

Ground-truth sources:
- spec/<ms>.index.json: registry emitted by block parser.
- spec/<ms>.md: milestone text parsed by regex.

Writes:
- recon/plans/EXT_BLOCK_COUNTS.md: table of milestone block counts from disk.
"""
from pathlib import Path
import json
import re
import sys

ROOT = Path(__file__).resolve().parent.parent
SPEC = ROOT / "spec"
PLAN = ROOT / "recon/plans"


def milestone_ids_from_json(ms: str):
    p = SPEC / f"{ms}.index.json"
    if not p.exists():
        return []
    data = json.loads(p.read_text(encoding="utf-8", errors="ignore"))
    return [item.get("id") for item in data if isinstance(item, dict) and item.get("id")]


def milestone_ids_from_md(ms: str):
    p = SPEC / f"{ms}.md"
    if not p.exists():
        return []
    text = p.read_text(encoding="utf-8", errors="ignore")
    return sorted(set(re.findall(rf"{ms}-EXT-(\d+)", text)), key=int)


def extract_title(ms: str, block_id: str):
    """Return first non-empty heading/title text after the block anchor."""
    p = SPEC / f"{ms}.md"
    if not p.exists():
        return ""
    text = p.read_text(encoding="utf-8", errors="ignore")
    anchor = block_id.lower()
    idx = text.find(f"#### [{block_id}]")
    if idx == -1:
        idx = text.find(f"#### [{block_id.upper()}]")
    if idx == -1:
        idx = text.find(f"#### [{[block_id]}]")
    if idx == -1:
        idx = text.lower().find(anchor)
    if idx == -1:
        return ""
    snippet = text[idx: idx + 900]
    m = re.search(r"^####\s*\[[^\]]+\]\s*(.*)$", snippet, re.MULTILINE)
    if m:
        title = m.group(1).strip()
        if title:
            return title
    m = re.search(r"^>\s*\*\*tl;dr\*\*\s*[·\-]+\s*(.*)$", snippet, re.MULTILINE)
    if m:
        return m.group(1).strip()
    return ""


required = ["M0", "M1", "M2", "M3", "M4", "M4.5", "M5", "M6", "M6.5", "M7", "M8", "M9", "M10", "M11", "M12", "M13"]
warnings = []
rows = []
checks = []
rooms = []


def require(label, condition):
    checks.append((label, bool(condition)))


for ms in required:
    ids_json = milestone_ids_from_json(ms)
    ids_md = milestone_ids_from_md(ms)
    count_json = len(ids_json)
    count_md = len(ids_md)
    max_json = max([int(x.split("-")[-1]) for x in ids_json], default=0)
    max_md = max([int(x) for x in ids_md], default=0)

    if count_json != count_md:
        warnings.append(f"{ms}: JSON count {count_json} != MD count {count_md}")

    sample = []
    for block_id in ids_json[:3]:
        sample.append({
            "id": block_id,
            "title": extract_title(ms, block_id) or "Systems Touched",
        })

    rows.append({
        "ms": ms,
        "json_count": count_json,
        "md_count": count_md,
        "max_json": max_json,
        "max_md": max_md,
        "sample": sample,
        "json_path": str(SPEC / f"{ms}.index.json"),
        "md_path": str(SPEC / f"{ms}.md"),
    })

total_json = sum(r["json_count"] for r in rows)
total_md = sum(r["md_count"] for r in rows)

# True ceilings: index JSON wins because it is emitted by the parser/author.
for r in rows:
    r["true_count"] = r["json_count"]
    r["true_max"] = r["max_json"]
    require(f"{r['ms']} JSON/MD counts match", r["json_count"] == r["md_count"])

out_lines = [
    "# EXT Block Count Verification",
    "> Generated from spec/*.index.json and spec/*.md on disk.",
    "",
    "| Milestone | JSON count | MD count | True max | Status |",
    "|-----------|-----------|----------|---------|--------|",
]
for r in rows:
    status = "OK" if r["json_count"] == r["md_count"] else "MISMATCH"
    out_lines.append(f"| {r['ms']} | {r['json_count']} | {r['md_count']} | {r['true_max']} | {status} |")

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
if warnings:
    print("WARNINGS:")
    for w in warnings:
        print(f"- {w}")
else:
    print("Warnings: none")

# Output verification summary for terminal.
action = "replace" if (ROOT / "recon/plans/EXT_BLOCK_COUNTS.md").exists() else "add"
print(f"VERIFY_EXT_BLOCKS write_action={action}")
