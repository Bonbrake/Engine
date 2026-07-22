# ZombieEngine Repo Hygiene and Branch Cleanup Plan

Evidence date: 2026-07-21. Derived from `git branch`, `git ls-remote`, and the
live `STATUS.md` on origin `Main`.

## Current confirmed state

- Active long-lived branches on remote: `Main`, `beta`, `spec/m0-parity-reformat`
- Local branches: `main`, `beta`, `spec/m0-parity-reformat`, `wt/proof-1`
- Stale local branches from past agent runs: 3 `subagent-*` branches
- Top-level duplication: `REFERENCE_GAME_ANALYSIS.md` duplicates `docs/research/`
- Stale directories: `Testing/Temporary`, empty `Testing`
- Recon plans:`recon/plans/` has 14 plan files. Only one is the current
  consolidated plan. The rest are superseded snapshots.
- Default remote HEAD is `origin/Main`.

## Goals

1. One canonical source of truth per artifact.
2. Branch structure a stranger can read in 10 seconds.
3. No destructive actions without explicit evidence and user approval for
   one-shot or remote branches with unknowns.

## Hard rules

- Do not delete `Main`, `beta`, or `spec/m0-parity-reformat` without explicit
  approval.
- Do not delete any branch with an open PR without explicit approval.
- Do not move or delete `SPEC FILES READ-ONLY` milestone docs without explicit
  approval.
- Every destructive step requires a verification command first.

## Step 1: Branch audit report

Action: produce a branch map with last commit date, unique commit count, and
whether the branch is already merged into any long-lived branch.

Verification command:
```sh
cd /c/ZombieEngine
for ref in $(git for-each-ref --format='%(refname:short)' refs/heads/ refs/remotes/origin/); do
  merge_main=$(git merge-base --is-ancestor $ref origin/Main && echo MERGED || echo UNIQUE)
  echo "$ref $merge_main $(git log -1 --format='%h %ad %s' --date=short --no-walk $ref 2>/dev/null)"
done | sort
```

Expected outcome: exact list of branches that are safe to close, merged, or keep.

## Step 2: Retire local-only duplicate/one-shot branches

Safe to delete locally if the branch’stip commit is already reachable from
`spec/m0-parity-reformat`, `main`, or `Main`, or if the branch name is clearly
one-shot from agent/subagent runs.

Actions:
- Delete local branches whose tips are already merged into an active branch.
- Move their notes into `recon/plans/` or `docs/` if the content has value.

Verification:
```sh
cd /c/ZombieEngine
git branch -D <branch>   # only after confirming merge status from Step 1
git branch -vv
git status
```

Pass condition: Step 1 output matches the deletion plan before any delete.

## Step 3: Map remote branch ownership before deleting anything remote

For each remote branch outside `Main`/`beta`/`spec/`, inspect the underlying
commit date and metadata before deciding.

Verification:
```sh
cd /c/ZombieEngine
for ref in $(git branch -r | grep -vE 'HEAD|Main|beta|spec/' | sed 's/^  //'); do
  echo "--- $ref ---"
  git log -1 --format='%h %ad %s' --date=short $ref 2>/dev/null
done
```

Actions after review:
- If branch is an automated one-shot or stale one, remove its remote ref and
  inform the user before deleting remote history.
- If branch has open PRs, leave it and label accordingly.

## Step 4: Doc deduplication

Confirmed facts:
- `docs/research/engine_architecture_lessons.md` already exists.
- `REFERENCE_GAME_ANALYSIS.md` at root overlaps with research content.

Action: move `REFERENCE_GAME_ANALYSIS.md` into `docs/research/reference_game_analysis.md`
and update any links in `README.md` and `STATUS.md`/`docs/README.md`.

Verification:
```sh
cd /c/ZombieEngine
test ! -f REFERENCE_GAME_ANALYSIS.md
test -f docs/research/reference_game_analysis.md
grep -R "REFERENCE_GAME_ANALYSIS" README.md docs/ || true
```

Pass condition: old path absent, new path present, link updates committed.

## Step 5: Remove stale empty directories

Action: delete `Testing/Temporary` and the empty `Testing` folder.

Verification:
```sh
ls Testing   # should fail
ls Testing/Temporary  # should fail
```

## Step 6: Recon plans consolidation

Action: rename superseded recon plans into `recon/archive/plan-snapshots/` and
leave exactly one active current plan at `recon/plans/2026-07-20_RESEARCHED_PROFESSIONAL_PLAN.md`.
If `2026-07-21_*` plans are superseded by the current plan, move them.

Verification:
```sh
ls recon/plans/
ls recon/archive/plan-snapshots/
```

Pass condition: recon/plans/ contains exactly one plan and one README-like file.

## Step 7: Final repo-hygiene gate

Action: write and run `scripts/repo_hygiene.sh` that checks:
- no `Testing/` dirs, no root duplicate research markdown,
- no local `subagent-*` branches visible,
- no merged stale branches remaining

Verification:
```sh
cd /c/ZombieEngine
bash scripts/repo_hygiene.sh
```

Pass condition: script exits 0 on clean tree.

## Execution order

1. Steps 1-2 and 3: branches in that order
2. Step 4: docs untangle
3. Step 5: empty dirs
4. Step 6: recon consolidation
5. Step 7: hygiene gate script, then commit the script

## What is explicitly excluded until approved

- Merging `beta`, `Main`, or `spec/m0-parity-reformat` into each other
- Deleting any remote branch with an open PR
- Removing the SPEC READ-ONLY milestone files
- Changing GitHub web UI branch shadowing/default behavior without changing
  branch names

## Commit messages to use

- Branch cleanup: `chore: retire stale one-shot local branches after audit`
- Remote branch audit: `chore: archive remote one-shot branches with explicit log evidence`
- Doc consolidation: `chore: consolidate duplicate research markdown under docs/research`
- Stale dirs: `chore: remove empty Testing/ directory`
- Recon cleanup: `chore: move superseded plan snapshots into recon/archive/`
- Hygiene gate: `chore: add repo hygiene verification gate`
