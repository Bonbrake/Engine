text = """

### [M4-EXT-106] Spherical Harmonics Volumetric Cloud & Atmospheric Scattering

> **tags** · rendering, volumetrics, clouds, atmosphere, aaa
> **tl;dr** · M4. Renders multi-scattering dynamic atmospheric skies and real-time volumetric cloud banks using spherical harmonic evaluation.
> **ctx** · 2026 Atmospheric Standard (Paper 81). Replaces static skyboxes with dynamic, time-of-day volumetric skies.
> **meta** · depends-on: M4-EXT-01

##### Implementation
1. Evaluates Rayleigh and Mie atmospheric scattering equations in a compute shader `sky_scatt.comp`.
2. Encodes directional sun and sky radiance into 3rd-order Spherical Harmonics (SH) coefficients.
3. Raymarches volumetric cloud densities, scattering light dynamically through rainstorms, clear days, and apocalyptic blood-red sunsets.


### [M4.5-EXT-112] GPU Dynamic Foliage Wind & Bending Physics Engine

> **tags** · rendering, foliage, physics, compute, aaa
> **tl;dr** · M4.5. Simulates dynamic tree trunk bending, branch swaying, and grass leaf turbulence under storm winds using compute shaders.
> **ctx** · 2026 Foliage Standard (Paper 82). Brings open-world forests and overgrown urban grass alive during weather storms.
> **meta** · depends-on: M4.5-EXT-01

##### Implementation
1. Foliage vertices encode stiffness parameters and wind hierarchy weights in color channels.
2. Compute shader `foliage_wind.comp` evaluates 3D Perlin noise wind gusts and rotor turbulence from helicopter/explosion impulses.
3. Updates vertex offsets dynamically in GPU buffers, delivering realistic swaying for 100,000+ grass blades and trees with 0 CPU overhead.


### [M4.5-EXT-113] Screen-Space Subsurface Scattering (SSSS) for Skin & Flesh

> **tags** · rendering, ssss, skin, flesh, aaa
> **tl;dr** · M4.5. Simulates realistic skin translucency, ear rim light diffusion, and blood-vessel scattering on survivors and zombies using screen-space blurring.
> **ctx** · 2026 Skin Rendering Standard (Paper 83). Eliminates the "plastic look" on character faces and zombie flesh.
> **meta** · depends-on: M4.5-EXT-01

##### Implementation
1. G-Buffer records a Subsurface Scattering (SSS) profile ID and diffuse color.
2. In post-processing, `ssss_blur.comp` applies a multi-pass separable Gaussian blur in screen-space, scaled by surface depth variance.
3. Simulates light penetrating human ears, nostrils, and zombie wounds, scattering realistic red/pink illumination.


### [M4.5-EXT-114] Adaptive Screen-Space Reflections (SSSR) with Hi-Z Tracing

> **tags** · rendering, ssr, reflections, vulkan, aaa
> **tl;dr** · M4.5. Renders realistic glossy reflections on wet asphalt, rain puddles, and glass windows using Hi-Z ray-marched screen-space reflections.
> **ctx** · 2026 Reflection Standard (Paper 84). Delivers crisp SSR reflections with zero ray-marching performance stutter.
> **meta** · depends-on: M4.5-EXT-01

##### Implementation
1. Traces reflection rays through downsampled Hierarchical Z-Buffer (Hi-Z) depth pyramids in a Vulkan compute shader.
2. Evaluates material roughness using GGX BRDF importance sampling.
3. Blends SSR seamlessly with environment reflection probes for missing off-screen reflection data.


### [M6-EXT-105] Rigid Body Impact Sound Synthesis via Modal Resonance

> **tags** · audio, modal, physics, dsp, aaa
> **tl;dr** · M6. Generates procedural impact audio (metal clanging, wood splintering, glass shattering) directly from Jolt collision impulse vectors.
> **ctx** · 2026 Impact Audio Standard (Paper 85). Eliminates repetitive impact sample playback during physics destruction.
> **meta** · depends-on: M6-EXT-01

##### Implementation
1. When Jolt physics registers an impact, it passes the impulse magnitude, surface material ID, and contact point to the audio engine.
2. Synthesizes impact audio live using a bank of damped modal resonant filters: $y(t) = \sum A_i e^{-d_i t} \sin(\omega_i t)$.
3. Dynamically alters pitch, timbre, and decay rate based on object mass and impact velocity.


### [M2-EXT-105] Dynamic FFT Water Waves & Buoyancy Physics Engine

> **tags** · physics, water, fft, buoyancy, jolt, aaa
> **tl;dr** · M2. Renders dynamic FFT ocean waves, river currents, and evaluates real-time zombie/vehicle flotation buoyancy forces in Jolt Physics.
> **ctx** · 2026 Water Physics Standard (Paper 86). Provides realistic water interaction for swimming, flooded city zones, and floating debris.
> **meta** · depends-on: M2-EXT-01

##### Implementation
1. Evaluates Tessendorf Fast Fourier Transform (FFT) wave spectrums on GPU in `water_fft.comp`.
2. Computes water surface height and velocity vectors in a shared GPU/CPU buffer.
3. Jolt physics integrates Archimedes buoyancy forces $\mathbf{F}_{buoyant} = \rho V_{submerged} \mathbf{g}$ across submerged mesh colliders, floating vehicles, and drowning zombies.


### [M4-EXT-107] GPU Geometry Clipmaps for Seamless Infinite Terrain

> **tags** · rendering, terrain, clipmaps, vulkan, aaa
> **tl;dr** · M4. Renders multi-LOD terrain mesh geometry seamlessly using GPU Geometry Clipmaps without vertex gaps or visual popping.
> **ctx** · 2026 Terrain Standard (Paper 87). Enables smooth open-world terrain rendering across vast 1000 km² maps.
> **meta** · depends-on: M4-EXT-01

##### Implementation
1. Maintains concentric nested grid rings around the player camera (Geometry Clipmaps).
2. Compute shader updates vertex heights from the terrain heightmap RVT atlas as the camera moves.
3. Smoothly morphs vertex edges between LOD rings using transition regions, eliminating vertex seams and LOD popping.


### [M3-EXT-103] Particle-Based Dynamic Fire Propagation Engine

> **tags** · physics, fire, destruction, particles, aaa
> **tl;dr** · M3. Simulates dynamic fire spreading across wooden structures, trees, foliage, and zombies based on heat transfer equations.
> **ctx** · 2026 Fire Physics Standard (Paper 88). Brings realistic molotov cocktail and flamethrower fire propagation to gameplay.
> **meta** · depends-on: M3-EXT-01

##### Implementation
1. Combustible world objects maintain heat and fuel cell grid values.
2. Heat transfers between adjacent cells via thermal conduction and convective wind vectors: $\frac{\partial T}{\partial t} = \alpha \nabla^2 T + \mathbf{u} \cdot \nabla T$.
3. When cell temperature exceeds ignition threshold, spawns fire particles, degrades structural integrity (M3-EXT-102), and ignites nearby entities.


### [M4.5-EXT-115] Order-Independent Translucency (OIT) via Fragment Linked Lists

> **tags** · rendering, oit, translucency, vulkan, aaa
> **tl;dr** · M4.5. Resolves sorting artifacts for overlapping glass, smoke, water, and transparent particles using GPU Fragment Linked Lists.
> **ctx** · 2026 Translucency Standard (Paper 89). Eliminates visual sorting glitches in complex particle effects.
> **meta** · depends-on: M4.5-EXT-01

##### Implementation
1. Fragment shader uses Vulkan Atomic Operations (`imageAtomicExchange`) to store color, depth, and next-pointer in an Unordered Access View (UAV) buffer.
2. Post-processing compute shader sorts transparent fragment linked lists per pixel by depth.
3. Blends translucent layers in back-to-front order, rendering perfect transparent particles and glass windows without CPU sorting.


### [M4.5-EXT-116] Contact-Hardening Soft Shadow Filtering (PCSS)

> **tags** · rendering, shadows, pcss, vulkan, aaa
> **tl;dr** · M4.5. Renders dynamic penumbra soft shadows for sun, streetlights, and torchlights using Percentage-Closer Soft Shadows (PCSS).
> **ctx** · 2026 Shadow Standard (Paper 90). Delivers realistic shadow softness that hardens near contact points.
> **meta** · depends-on: M4.5-EXT-01

##### Implementation
1. Step 1: Search shadow map region to estimate average blocker depth.
2. Step 2: Calculate penumbra size based on light source radius and blocker distance: $w_{\text{penumbra}} = \frac{d_{\text{receiver}} - d_{\text{blocker}}}{d_{\text{blocker}}} w_{\text{light}}$.
3. Step 3: Filters shadow map with variable kernel size Percentage-Closer Filtering (PCF), producing razor-sharp contact shadows and soft distant penumbras.


### [M5-EXT-107] Utility AI Fallback Zombie Mutation Behavior Trees

> **tags** · ai, behavior-tree, utility-ai, zombies, aaa
> **tl;dr** · M5. Combines Hierarchical Behavior Trees with Utility AI evaluations to drive special infected zombie mutations (Tank, Runner, Spitter).
> **ctx** · 2026 Zombie AI Standard (Paper 91). Creates unpredictable, intelligent special zombie encounter behaviors.
> **meta** · depends-on: M5-EXT-01

##### Implementation
1. High-level state selection uses a formal Behavior Tree structure (Selector, Sequence, Parallel).
2. Action selection nodes evaluate Utility AI curves ($U = \prod f_i(x_i)$) based on player health, line-of-sight, and cover availability.
3. Drives special zombie tactics: Spitters retreat behind cover to reload acid, while Tanks target the player dealing the highest DPS.


### [M8-EXT-103] Dynamic NPC Faction Opinion & Emergent Alliance Graphs

> **tags** · gameplay, factions, npcs, political, aaa
> **tl;dr** · M8. Simulates survivor faction politics, trust decay, and procedural alliance shifts based on player trade and conflict choices.
> **ctx** · 2026 Faction System Standard (Paper 92). Turns survivor factions into living political entities with dynamic warfare.
> **meta** · depends-on: M8-EXT-01

##### Implementation
1. Factions maintain dynamic opinion matrices $O_{i,j} \in [-100, 100]$ measuring mutual trust, threat, and economic trade dependencies.
2. Player choices (trading supplies, rescuing survivors, attacking outposts) trigger opinion drift across the network graph.
3. Triggers emergent faction wars, trade blockades, or combined survivor assaults against player bases based on graph thresholds.


### [M4-EXT-108] Procedural Structural Building Interior Generator

> **tags** · procedural, interiors, buildings, world, aaa
> **tl;dr** · M4. Generates fully explorable, structurally realistic urban building floor plans with furniture placement and loot spawns.
> **ctx** · 2026 Building Generator Standard (Paper 93). Ensures every building in open-world cities has an explorable, realistic interior.
> **meta** · depends-on: M4-EXT-01

##### Implementation
1. Extrudes building footprint polygons into floor slabs and structural support pillars.
2. Uses rectangular space partitioning (BSP trees) to divide floor space into realistic rooms (kitchens, bedrooms, corridors, stairs).
3. Spawns contextual furniture and loot containers (M11-EXT-104) while validating navmesh connectivity for zombie pathfinding.


### [M11-EXT-105] Encumbrance Inventory Grid & Weight Physics

> **tags** · ui, inventory, weight, stamina, aaa
> **tl;dr** · M11. Grid-based item inventory management with real-time weight encumbrance physics affecting player stamina and movement speed.
> **ctx** · 2026 Inventory Standard (Paper 94). Adds tactical depth to looting and resource management.
> **meta** · depends-on: M11-EXT-01

##### Implementation
1. Inventory UI displays items as 2D grid tiles (Tetris-style packing).
2. Calculates total carried weight: $W_{\text{total}} = \sum m_i + W_{\text{container}}$.
3. If $W_{\text{total}}$ exceeds player strength threshold, applies encumbrance debuffs to max stamina, sprint speed, and weapon aim sway in `M2-EXT`.


### [M12-EXT-102] Server-Rewind Hitscan & Projectile Verification Netcode

> **tags** · netcode, rewind, hitscan, multiplayer, aaa
> **tl;dr** · M12. Performs server-side history rewinds to verify player gunshots and melee hits in laggy multiplayer co-op sessions.
> **ctx** · 2026 Co-op Netcode Standard (Paper 95). Eliminates "I shot him on my screen!" network desync complaints.
> **meta** · depends-on: M12-EXT-01

##### Implementation
1. Server maintains a rolling 1000ms buffer of entity transforms and collision bounding boxes.
2. When client submits a fire command with timestamp $T_{\text{sub}}$, server rewinds world state to $T_{\text{sub}} - \text{Latency}$.
3. Evaluates bullet raycast against rewound entity hitboxes, guaranteeing 100% fair hit registration regardless of client ping.


### [M8-EXT-104] Modular Weapon Customization & Ballistic Caliber Engine

> **tags** · gameplay, weapons, ballistics, crafting, aaa
> **tl;dr** · M8. Modular weapon attachment system with realistic bullet velocity, drag coefficients, and armor penetration physics.
> **ctx** · 2026 Ballistics Standard (Paper 96). Deepens combat gunplay with realistic ballistics and weapon modding.
> **meta** · depends-on: M8-EXT-01

##### Implementation
1. Weapons accept modular attachments (suppressors, scopes, barrels, stocks) that dynamically modify weapon stats.
2. Projectiles evaluate 2D ballistic trajectory equations: $v(t) = v_0 / (1 + k t)$, calculating gravity drop and velocity degradation over distance.
3. Evaluates bullet armor penetration against zombie helmet and body armor ratings before applying damage.


### [M5-EXT-108] Zombie Pheromone Trail & Scent Diffusion Model

> **tags** · ai, scent, pheromones, horde, zombies, aaa
> **tl;dr** · M5. Simulates player scent and blood odor diffusion in air, luring distant zombie swarms upwind toward wounded players.
> **ctx** · 2026 Zombie AI Standard (Paper 97). Forces players to bandage bleeding wounds to avoid attracting horde swarms.
> **meta** · depends-on: M5-EXT-01

##### Implementation
1. Wounded or unwashed players emit scent particles into a 3D grid voxel volume `scent_grid.bin`.
2. Diffuses scent intensity over time based on wind velocity vectors: $\frac{\partial C}{\partial t} = D \nabla^2 C - \mathbf{u} \cdot \nabla C$.
3. Zombie sensing logic queries scent grid intensity, triggering horde migration toward high-concentration scent sources.


### [M0-EXT-108] Vulkan Bindless Descriptor Buffer Allocation Pipeline

> **tags** · vulkan, bindless, descriptors, optimization, aaa
> **tl;dr** · M0. Completely eliminates CPU descriptor set binding overhead using Vulkan 1.4 `VK_EXT_descriptor_buffer` extensions.
> **ctx** · 2026 Vulkan Driver Standard (Paper 98). Provides maximum GPU draw call throughput with zero driver CPU overhead.
> **meta** · depends-on: M0-EXT-01

##### Implementation
1. Replaces traditional `vkCmdBindDescriptorSets` with `VK_EXT_descriptor_buffer` direct memory offsets.
2. Shader reads texture indices directly from GPU memory buffers via uint64_t pointers (`VkDescriptorBufferBindingInfoEXT`).
3. Allows draw calls to access all 100,000+ engine textures and buffers without ever binding descriptor sets.


### [M6-EXT-106] Acoustic Voxel Distance Field Sound Dampening

> **tags** · audio, acoustics, dampening, occlusion, aaa
> **tl;dr** · M6. Simulates realistic sound dampening through closed doors, concrete walls, and barricades using acoustic distance fields.
> **ctx** · 2026 Audio Occlusion Standard (Paper 99). Allows players to hear zombies clawing at doors with accurate low-pass muffling.
> **meta** · depends-on: M6-EXT-01

##### Implementation
1. Constructs an Acoustic Distance Field (ADF) voxel grid representing material thickness and sound isolation ratings.
2. Traces sound paths between source and listener, calculating material absorption loss: $\text{Atten}_{\text{dB}} = \sum d_i \alpha_i$.
3. Dynamically adjusts FMOD/Wwise low-pass filter cutoff frequencies and gain levels based on obstacle thickness.


### [M0-EXT-109] Automated Spec Parity & Continuous CI Verification Pipeline

> **tags** · engine, verification, ci, protocol, aaa
> **tl;dr** · M0. Enforces 100% mechanical parity between C++ engine source code, markdown spec files, and sidecar JSON indices in automated CI.
> **ctx** · 2026 Quality Assurance Standard (Paper 100). Prevents spec drift or un-documented code implementation.
> **meta** · depends-on: M0-EXT-01

##### Implementation
1. Automated CI runner executes `python scripts/verify_m0_parity.py` and `python scripts/verify_ext_block_counts.py` on every git commit.
2. Scans C++ source code for `// [M1-EXT-XX]` tags, validating that every implemented function maps directly to a verified spec block.
3. Rejects pull requests or commits if block count mismatch, dangling cross-references, or un-documented logic blocks are detected.
"""

with open(r'C:\ZombieEngine\spec\_v80_presplit.md', 'a', encoding='utf-8') as f:
    f.write(text)
print('Appended Papers 81-100 blocks successfully!')
