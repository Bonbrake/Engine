#!/usr/bin/env python3
"""Verify Zombie Engine Master Refactor Plan v8.0 DAG and Document Parity."""

from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parent.parent
LOCAL_PLAN = ROOT / "docs" / "plans" / "ZOMBIE_ENGINE_MASTER_REFACTOR_PLAN_v8.0.md"
DOCS_PLAN = Path("C:/Users/jakeb/Documents/Plans/ZOMBIE_ENGINE_MASTER_REFACTOR_PLAN_v8.0.md")

def main():
    print("==================================================")
    print(" ZOMBIE ENGINE v8.0 MASTER PLAN DAG & PARITY VERIFIER")
    print("==================================================")

    if not LOCAL_PLAN.exists():
        print(f"FAIL: Local plan file missing: {LOCAL_PLAN}")
        return 1

    text = LOCAL_PLAN.read_text(encoding="utf-8")
    lines = text.splitlines()

    stage_pat = re.compile(r"^###\s+(STAGE\s+\d+[^:\n]*)")
    row_pat = re.compile(r"^\|\s*(?:`)?(T\d+-\d+)(?:`)?\s*\|([^|]+)\|([^|]+)(?:\|([^|]+)\|)?")

    current_stage = "UNKNOWN"
    features = {}

    for line in lines:
        sm = stage_pat.match(line.strip())
        if sm:
            current_stage = sm.group(1).strip()
            continue
        rm = row_pat.match(line.strip())
        if rm:
            fid = rm.group(1).strip()
            col1 = rm.group(2).strip()
            col2 = rm.group(3).strip()
            col3 = rm.group(4).strip() if rm.group(4) else ""
            deps = re.findall(r"T\d+(?:-\d+)?", col3) if col3 else []
            features[fid] = {"stage": current_stage, "name": col1, "desc": col2, "deps": deps}

    stage_counts = {}
    for fid, data in features.items():
        s = data["stage"]
        stage_counts[s] = stage_counts.get(s, 0) + 1

    print(f"\n[1] Feature Inventory: {len(features)} total features parsed across Stages 0-5")
    for s in sorted(stage_counts.keys()):
        print(f"    - {s}: {stage_counts[s]} features")

    # 2. Check dangling dependencies
    dangling = []
    for fid, data in features.items():
        for d in data["deps"]:
            if d in ("T0", "T1", "T2", "T3", "T4", "T5"):
                continue
            if d not in features:
                dangling.append((fid, d))

    print(f"\n[2] Dangling Dependency Check: {len(dangling)} dangling references")
    if dangling:
        for d in dangling:
            print(f"    FAIL: Feature {d[0]} depends on nonexistent {d[1]}")
        return 1
    else:
        print("    PASS: 0 dangling dependencies.")

    # 3. Check cycles
    visited = {}
    cycles = []

    def check_cycle(node, path):
        visited[node] = 1
        for dep in features[node]["deps"]:
            if dep in features:
                if visited.get(dep, 0) == 1:
                    cycles.append(path + [dep])
                elif visited.get(dep, 0) == 0:
                    check_cycle(dep, path + [dep])
        visited[node] = 2

    for fid in features:
        if visited.get(fid, 0) == 0:
            check_cycle(fid, [fid])

    print(f"\n[3] Cycle Detection: {len(cycles)} circular loops found")
    if cycles:
        for c in cycles:
            print(f"    FAIL: Cycle detected: {' -> '.join(c)}")
        return 1
    else:
        print("    PASS: 0 dependency cycles (Strict DAG).")

    # 4. Check stage ordering violations
    stage_order = {s: i for i, s in enumerate(sorted(stage_counts.keys()))}
    stage_violations = []

    for fid, data in features.items():
        curr_s_idx = stage_order[data["stage"]]
        for d in data["deps"]:
            if d in features:
                dep_s_idx = stage_order[features[d]["stage"]]
                if dep_s_idx > curr_s_idx:
                    stage_violations.append((fid, data["stage"], d, features[d]["stage"]))

    print(f"\n[4] Stage Ordering Check: {len(stage_violations)} violations")
    if stage_violations:
        for v in stage_violations:
            print(f"    FAIL: Feature {v[0]} ({v[1]}) depends on later feature {v[2]} ({v[3]})")
        return 1
    else:
        print("    PASS: 0 stage-ordering violations (forward-only progression).")

    # 5. Check binary equivalence with Documents/Plans
    print(f"\n[5] Mirror Parity Check:")
    if not DOCS_PLAN.exists():
        print(f"    WARN: Documents plan not found at {DOCS_PLAN}")
    else:
        local_bytes = LOCAL_PLAN.read_bytes()
        docs_bytes = DOCS_PLAN.read_bytes()
        if local_bytes == docs_bytes:
            print(f"    PASS: docs/plans and Documents/Plans are 100% byte-for-byte identical ({len(local_bytes)} bytes).")
        else:
            print(f"    FAIL: Binary mismatch between repo and Documents/Plans!")
            return 1

    print("\n==================================================")
    print(" VERIFICATION SUCCESS: v8.0 Plan is 100% Mathematically Sound!")
    print("==================================================")
    return 0

if __name__ == "__main__":
    sys.exit(main())
