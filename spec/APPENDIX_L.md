## Appendix L — Modding & Extensibility Architecture (v71)

Added because a heavily data-driven engine (every weapon/vehicle/scarcity/caravan table is already JSON, per `[K-EXT-13]`) is most of the way to being moddable already — this appendix closes the remaining gap between "JSON hot-reload for me during dev" and "safe, conflict-free multi-mod support for players," using the same layered-override pattern (Polymod/Bethesda-style: data-driven core, mods merge as named layers, not raw file replacement) that the current best-of-breed moddable engines converge on.

#### [L-EXT-01] Layered Multi-Mod Manifest & Load-Order Resolver

**Systems Touched:** Extends `[K-EXT-13]`'s single-mod-directory `ResolveModAssetPath` from one override folder to an ordered stack of them — this is that system's direct successor, not a parallel loader.

**How It Works:** Each mod is a folder containing a `manifest.json` (`{ "id": "...", "name": "...", "version": "...", "author": "...", "priority": int, "requires": ["other_mod_id@>=1.0"] }`) plus the same JSON table structure the base game already reads. At boot, all installed mod folders are sorted by `priority` (ties broken alphabetically by `id` for determinism) into an ordered stack; `requires` version constraints are checked before any table loads, and a mod whose dependency isn't satisfied is skipped with a loud boot-log entry rather than silently loading half-broken. Asset/table resolution walks the stack top-down, same lookup shape as `[K-EXT-13]`'s two-path check, just generalized to N paths instead of 2.

**Reference Implementation**

```cpp
// Resolves a data-table path by walking the sorted mod stack top-down; falls through to the base game table if no mod overrides it.
struct ModManifest { std::string id, version; int priority; std::vector<std::string> requires_; };

std::string ResolveLayeredAssetPath(const std::string& filename, const std::vector<ModManifest>& sortedModStack,
                                     const std::string& modsRootDir, const std::string& baseDir) {
    for (const auto& mod : sortedModStack) { // already sorted by priority at boot
        std::string candidate = modsRootDir + mod.id + "/" + filename;
        if (std::filesystem::exists(candidate)) return candidate;
    }
    return baseDir + filename; // no mod overrides this file — base game table wins
}
```

**Player-Facing Impact:** Multiple mods combine automatically instead of one mod silently clobbering another's files, with a deterministic, inspectable load order.


#### [L-EXT-02] Boot-Time Conflict & Schema Validation Report

**Systems Touched:** Runs immediately after `[L-EXT-01]`'s stack resolves, before any table is handed to gameplay systems — extends `[M0-EXT-08]`'s existing "validate loudly at boot" pattern already established for the single-mod-dir case in `[K-EXT-13]`.

**How It Works:** For every JSON table key, if two or more mods in the stack define the *same* key (a weapon archetype hash, a scarcity-tuning constant), the winner (highest priority) is logged alongside every mod it overrode — visible in a `mod_conflicts.log`, not silently swallowed. Every loaded table is validated against its schema (same field-presence/type check `[K-EXT-13]` already runs for a single mod dir) before it reaches gameplay code; a malformed entry disables just that entry (falls back to the base game's version) and logs which mod/key failed, rather than crashing the whole table load.

**Player-Facing Impact:** A broken or conflicting mod produces a readable log line pointing at exactly which mod and key caused it, instead of an unexplained crash or silently wrong behavior — the single biggest driver of "why won't my modlist load" support burden in every moddable game's community.


#### [L-EXT-03] Namespaced Content ID Convention

**Systems Touched:** Extends the existing Two-Tier archetype/instance hash seed (§ near M8 itemization) — this is the namespacing rule that seed system needs once more than one mod can define a `Hash64("Weapon_...")` string.

**How It Works:** Every mod-defined content key is required to be prefixed with the mod's `manifest.json` `id` (e.g. `mymod.Weapon_RustySickle` rather than a bare `Weapon_RustySickle`) before it's hashed into the archetype seed. Base-game content has no prefix (implicitly `core.`). This is a pure naming convention enforced at `[L-EXT-02]`'s schema-validation step — it costs nothing at runtime and is the single change that lets two unrelated mods both add a "Weapon_Shotgun" without their `SplitMix64` archetype hashes colliding.

**Player-Facing Impact:** Two mods can both add content with the same human-readable name without one silently overwriting the other's item in save files.


#### [L-EXT-04] Modding API Surface Document (auto-generated, not hand-maintained)

**Systems Touched:** M0 build step — a small tool run at the end of every engine build, not a runtime system.

**How It Works:** A build-time script walks every JSON schema struct already declared across M8/M8.5/M9/M5.4 (weapon archetypes, vehicle chassis tables, scarcity-tuning constants, caravan route weights) and emits a single `MODDING_API.md` documenting every moddable field, its type, and its valid range — generated from the same schema `[L-EXT-02]` validates against, so the published documentation can never silently drift out of sync with what the engine actually accepts (a common failure mode in hand-maintained modding docs).

**Player-Facing Impact:** Modders get accurate, always-current documentation of exactly what they can change, generated from the real schema instead of a stale wiki page.


**Explicitly out of scope for this pass:** a Lua/scripting hook for mod *logic* (not just data) — the polymod-style research above confirms this is the natural next step once data-driven modding is solid, but it's a real scope increase (sandboxing, a scripting VM, an API-stability contract) that deserves its own milestone slot rather than being folded in here. Flagged in `[K-EXT]`-style form for a future pass: **Scripted Mod Hook Layer** — a sandboxed script VM (Wren or a stripped Lua build are the usual lightweight choices for a C++ engine this size) exposed read/write access to a deliberately small, versioned subset of ECS components, not the whole registry.
