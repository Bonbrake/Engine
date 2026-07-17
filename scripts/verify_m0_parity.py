#!/usr/bin/env python3
# verify_m0_parity.py — check every spec/Mx.md against the REAL M0 skeleton.
#
# Calibrated to the actual on-disk format (verified against M0.md, the gold standard):
#   * Block header:  #### [Mx-EXT-NN] Title   (H4; backtick form `#### [Mx-EXT-NN]` also accepted)
#   * Injected layers (order matters):  > **tags**  ->  > **tl;dr**  ->  > **ctx**  ->  > **meta**
#   * Subsections (5+):  Systems Touched / Math / Algorithm / Examples / Failure Modes / Player-Facing Impact
#   * Deep-links use the GitHub header anchor (#### header id), NOT a manual `<a id>` tag.
#     M0.md has 0 manual <a id> tags by design, so the old "missing anchor" check was wrong and removed.
#   * JSON sidecar keys: id, title, cluster, depends_on, depended_by, line, anchor
#     (anchor = derived header slug, not a stored <a id>; recompute, don't assert presence of <a id>.)
#
# Scoped to each file's OWN label (so foreign-embedded blocks like M7-EXT-04 in M4.md
# are NOT counted as the file's own blocks — matches llms.txt own-count rule).
import re, os, json, glob

SPEC = r"C:\ZombieEngine\spec"

# REAL skeleton, confirmed from M0.md
REQ_LAYERS = ["tags", "tl;dr", "ctx", "meta"]
REQ_SUBS = {"Systems Touched", "Math", "Algorithm", "Examples", "Failure Modes", "Player-Facing Impact"}
SUB_RX = re.compile(r'^#####\s+(.*?)\s*$')
HDR_RX = re.compile(r'^####\s+`?\[((?:M\d[\d.]*|K|L)-EXT-(\d+))\]')

def header_anchor(bid):
	# GitHub-style slug: lowercase, spaces->hyphens, strip punctuation except hyphen
	s = bid.lower().replace(" ", "-").replace("_", "-")
	s = re.sub(r'[^a-z0-9\-]', '', s)
	return s

def own_blocks(rows, label):
	out = []
	rx = re.compile(r'^####\s+`?\[(' + re.escape(label) + r'-EXT-(\d+))\]')
	for i, l in enumerate(rows):
		m = rx.match(l)
		if m:
			out.append((i, m.group(1), int(m.group(2))))
	return out

def block_end(rows, b, label):
	for j in range(b + 1, len(rows)):
		if HDR_RX.match(rows[j]) and (rows[j].startswith("#### [" + label) or rows[j].startswith("#### `[" + label)):
			return j
		if rows[j].startswith("## "):
			return j
	return len(rows)

def main():
	results = {}
	for path in sorted(glob.glob(os.path.join(SPEC, "M*.md"))):
		fn = os.path.basename(path)
		label = fn[:-3]
		rows = open(path, encoding="utf-8").read().split("\n")
		issues = []

		# frontmatter
		fm_blocks = None
		if rows and rows[0].strip() == "---":
			try:
				end = rows.index("---", 1)
				fm = "\n".join(rows[1:end])
				m = re.search(r'ext_blocks:\s*(\d+)', fm)
				if m:
					fm_blocks = int(m.group(1))
				else:
					issues.append("no ext_blocks in frontmatter")
			except ValueError:
				issues.append("unclosed frontmatter (no trailing ---)")

		blocks = own_blocks(rows, label)
		n = len(blocks)
		if fm_blocks is not None and fm_blocks != n:
			issues.append("frontmatter ext_blocks=%s != actual %d" % (fm_blocks, n))

		# ascending order
		nums = [b[2] for b in blocks]
		if nums != sorted(nums):
			issues.append("IDs not ascending: %s" % nums)

		for (b, bid, num) in blocks:
			e = block_end(rows, b, label)
			seg = rows[b:e]
			# injected layer order tags->tl;dr->ctx->meta
			# (STUB blocks are intentionally unfinished — skip the layer-order check)
			is_stub = any("> **STUB**" in ln for ln in seg)
			if not is_stub:
				inj = []
				for ln in seg[1:]:
					s = ln.strip()
					if s.startswith("> **tags**"):
						inj.append("tags")
					elif s.startswith("> **tl;dr**"):
						inj.append("tl;dr")
					elif s.startswith("> **ctx**"):
						inj.append("ctx")
					elif s.startswith("> **meta**"):
						inj.append("meta")
					elif s.startswith("### Cluster") or s.startswith("## "):
						break
					elif re.match(r'^####\s', ln):
						break
				if inj != REQ_LAYERS:
					issues.append("%s: layer order %s" % (bid, inj))
			# subsection skeleton (skip STUB blocks)
			is_stub = any("> **STUB**" in ln for ln in seg)
			if not is_stub:
				subs = set()
				for ln in seg:
					mm = SUB_RX.match(ln)
					if mm:
						subs.add(mm.group(1).split(":")[0].strip())
				missing = REQ_SUBS - subs
				if missing:
					issues.append("%s: missing subsections %s" % (bid, sorted(missing)))

		# TOC
		if not any("<details>" in l and "block index" in l for l in rows[:60]):
			issues.append("no collapsible block-index TOC")
		# clusters (prose-only 0-block milestones exempt)
		if n > 0:
			clusters = [l for l in rows if l.startswith("### Cluster")]
			if not clusters:
				issues.append("no ### Cluster dividers")

		# fences
		fc = sum(1 for l in rows if l.strip().startswith("```"))
		if fc % 2 != 0:
			issues.append("fences unbalanced (%d)" % fc)

		# JSON sidecar
		jp = path.replace(".md", ".index.json")
		if not os.path.exists(jp):
			issues.append("no JSON sidecar")
		else:
			try:
				data = json.load(open(jp, encoding="utf-8"))
				if len(data) != n:
					issues.append("sidecar entries %d != blocks %d" % (len(data), n))
				for k in ("id", "title", "cluster", "depends_on", "depended_by", "line", "anchor"):
					if any(k not in d for d in data):
						issues.append("sidecar missing key '%s'" % k)
						break
				# anchor consistency (derived, not stored <a id>)
				for d in data:
					exp = header_anchor(d.get("id", ""))
					if d.get("anchor") != exp:
						issues.append("%s: anchor '%s' != expected '%s'" % (d.get("id"), d.get("anchor"), exp))
						break
			except Exception as ex:
				issues.append("sidecar parse error: %s" % ex)

		results[label] = (n, issues)

	print("%-9s | %3s | issues" % ("file", "blk"))
	for label, (n, issues) in results.items():
		flag = "OK" if not issues else "FIX(%d)" % len(issues)
		print("%-9s | %3d | %s" % (label, n, flag))
		for it in issues:
			print("            - %s" % it)

if __name__ == "__main__":
	main()
