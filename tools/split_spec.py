#!/usr/bin/env python3
"""Split _v80_presplit.md into M0.md ... M13.md + M2.6E.md"""

import re
from pathlib import Path

SPEC_PATH = Path("C:/ZombieEngine/spec/_v80_presplit.md")
OUT_DIR = Path("C:/ZombieEngine/spec/")

# Read the full spec
content = SPEC_PATH.read_text(encoding="utf-8")

# Find all milestone headers: "## M0 —", "## M1 —", "## M2 —", "## M2.6 —", "## M2.6E —", "## M2.7 —", etc.
# Pattern matches lines starting with ## M followed by number/decimal/letter and em dash
pattern = r'^(## M[\d\.E]+ — .+)$'

# Find all milestone start positions
matches = list(re.finditer(pattern, content, re.MULTILINE))

if not matches:
    print("ERROR: No milestone headers found")
    exit(1)

print(f"Found {len(matches)} milestones:")
for m in matches:
    print(f"  {m.group(1)} at line {content[:m.start()].count(chr(10)) + 1}")

# Write each milestone to its own file
for i, m in enumerate(matches):
    start = m.start()
    end = matches[i + 1].start() if i + 1 < len(matches) else len(content)
    milestone_text = content[start:end].rstrip() + "\n"
    
    # Extract milestone ID from header (e.g., "M0", "M2.6", "M2.6E")
    header = m.group(1)
    mid_match = re.match(r'## (M[\d\.E]+) —', header)
    if not mid_match:
        print(f"WARNING: Could not parse milestone ID from: {header}")
        continue
    mid = mid_match.group(1)
    
    # Sanitize filename
    fname = mid.replace(".", "_") + ".md"
    out_path = OUT_DIR / fname
    out_path.write_text(milestone_text, encoding="utf-8")
    print(f"  Wrote {fname} ({len(milestone_text)} chars)")

# Also write the front matter (everything before first milestone) as _frontmatter.md
frontmatter = content[:matches[0].start()].rstrip() + "\n"
(OUT_DIR / "_frontmatter.md").write_text(frontmatter, encoding="utf-8")
print(f"  Wrote _frontmatter.md ({len(frontmatter)} chars)")

print("\nDone. Now run: cd C:/ZombieEngine && cmd /c tools/build_skeleton.bat")