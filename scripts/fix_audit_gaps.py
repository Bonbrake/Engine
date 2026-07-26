text = """

#### [K-EXT-22] (provisional) Spatiotemporal Blue-Noise Jitter Array Interleaver

> **tags** · rendering, raytracing, denoise, aaa
> **tl;dr** · K. Fleshes the Spatiotemporal Blue-Noise Jitter Array Interleaver: deterministic low-discrepancy per-frame offsets for hybrid ray-tracing denoise, the jitter half that M4.5's denoiser consumes but never specifies how the per-frame sequence is generated/ordered.
> **ctx** · Production Engine Audit. Missing implementation for the blue-noise offsets.
> **meta** · depends-on: M4.5-EXT-01

##### Implementation
1. Precomputes a 128x128 2D Blue Noise texture array for spatiotemporal ray offsetting.
2. Modulates ray directions per frame using golden ratio Halton sequencing.


#### [M13-EXT-15] SLM Broadcast Text Determinism Seeder *(RECONSTRUCTED FROM CITATION CONTEXT ? VERIFY)*

> **tags** · ai, narrative, determinism, aaa
> **tl;dr** · M13. SLM Broadcast Text Determinism Seeder ensures that identically seeded game instances yield identical LLM token output streams.
> **ctx** · Production Engine Audit. Missing implementation.
> **meta** · depends-on: M13-EXT-01

##### Implementation
1. Seeds the `llama.cpp` RNG via the engine's global determinism tick.
2. Applies temperature 0.0 with deterministic sampling for reproducible dialogue.
"""

# Append missing blocks
with open(r'C:\ZombieEngine\spec\_v80_presplit.md', 'a', encoding='utf-8') as f:
    f.write(text)

# Patch broken dependency
presplit = open(r'C:\ZombieEngine\spec\_v80_presplit.md', encoding='utf-8').read()
presplit = presplit.replace('depends-on: M13-EXT-30', 'depends-on: M13-EXT-01')
with open(r'C:\ZombieEngine\spec\_v80_presplit.md', 'w', encoding='utf-8') as f:
    f.write(presplit)

print('Restored missing blocks and patched M13-EXT-30 dependency!')
