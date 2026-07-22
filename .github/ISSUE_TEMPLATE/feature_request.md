---
name: Feature request
about: Suggest a new feature or system
title: ''
labels: enhancement
assignees: ''

---

**Milestone**
Which milestone does this touch? e.g. M0, M1, M4...

**System**
Which system? e.g. renderer, ECS, physics, audio, UI...

**Description**
What should the system/feature do? What player-facing problem does it solve?

**Acceptance criteria**
- [ ] Compiles under MSVC 14.44 + Ninja
- [ ] Unit test added in `tests/unit/`
- [ ] `sanity_suite.exe` still passes
- [ ] If it touches EXT blocks, update `spec/<ms>.md` and `recon/plans/APPENDIX_EXECUTION_MAP.md`
