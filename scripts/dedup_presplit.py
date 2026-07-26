import re
import os

filepath = r'C:\ZombieEngine\spec\_v80_presplit.md'
text = open(filepath, 'r', encoding='utf-8').read()

# Split the text by EXT block headers
# We want to capture the header line and everything up to the next header (or end of file)
# But wait, there's text BETWEEN the last block and the next milestone header.
# Let's split by ANY header `^## ` or `^### ` or `^#### ` that contains an EXT block OR doesn't.
# Actually, just parse line by line.

lines = text.split('\n')
blocks = []
current_block = []
current_ext_id = None
current_is_ext = False

# We'll group lines into chunks. A chunk is either an EXT block, or normal text.
chunks = []

def parse_ext_id(line):
    m = re.match(r'^#{2,6}\s*`?\[([A-Z0-9\.-]+-EXT-\d+)\]`?', line)
    return m.group(1) if m else None

for line in lines:
    ext_id = parse_ext_id(line)
    if ext_id:
        if current_block:
            chunks.append({"type": "ext" if current_is_ext else "text", "id": current_ext_id, "lines": current_block})
        current_block = [line]
        current_ext_id = ext_id
        current_is_ext = True
    elif line.startswith('## ') and not current_is_ext:
        # If it's a major header, maybe start a new chunk? Not necessary, we can just lump normal text together.
        current_block.append(line)
    elif line.startswith('## ') and current_is_ext:
        # A new section that IS NOT an EXT block started!
        if current_block:
            chunks.append({"type": "ext", "id": current_ext_id, "lines": current_block})
        current_block = [line]
        current_ext_id = None
        current_is_ext = False
    else:
        current_block.append(line)

if current_block:
    chunks.append({"type": "ext" if current_is_ext else "text", "id": current_ext_id, "lines": current_block})

# Now find duplicates
from collections import defaultdict
ext_chunks = defaultdict(list)
for i, chunk in enumerate(chunks):
    if chunk["type"] == "ext":
        ext_chunks[chunk["id"]].append(i)

to_delete = set()
for ext_id, indices in ext_chunks.items():
    if len(indices) > 1:
        # We have duplicates!
        # Score each chunk. A chunk with '> **tl;dr**' gets 10 points.
        # Otherwise, the later chunk wins.
        best_score = -1
        best_idx = -1
        for idx in indices:
            chunk_text = '\n'.join(chunks[idx]["lines"])
            score = 10 if '> **tl;dr**' in chunk_text else 0
            # Tie breaker: later is better
            if score >= best_score:
                best_score = score
                best_idx = idx
        
        for idx in indices:
            if idx != best_idx:
                to_delete.add(idx)

# Reconstruct
new_lines = []
for i, chunk in enumerate(chunks):
    if i not in to_delete:
        new_lines.extend(chunk["lines"])

new_text = '\n'.join(new_lines)
with open(filepath, 'w', encoding='utf-8') as f:
    f.write(new_text)

print(f"Removed {len(to_delete)} duplicate EXT blocks.")
