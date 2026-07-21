# Architecture

This directory contains ZombieEngine architecture documentation.

- `render.md` — Renderer pipeline, bindless descriptor model, RVT terrain layers
- `physics.md` — Jolt 5.6 integration, IPC barrier, destruction pipeline
- `ecs.md` — EnTT entity/component layout, SpatialHash, archetype streaming
- `audio.md` — M6 audio propagation, M6.5 particle/VFX system
- `persistence.md` — M7 save format, delta compression, schema versioning
- `networking.md` — M12 co-op transport, authority model, lag compensation
- `slm.md` — M13 local SLM backend, MiniCPM5-1B integration, SPSC queue

Each file maps directly to a milestone spec block in `spec/`.
