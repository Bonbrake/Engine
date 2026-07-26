text = """

### [M7-EXT-101] Double-Buffered Atomic Write-Ahead Logging (WAL) Save Persistence Engine

> **tags** · save, persistence, stability, aaa
> **tl;dr** · M7. Guarantees 100% save file corruption immunity during sudden power outages or OS crashes using atomic Write-Ahead Logging (WAL) transactions.
> **ctx** · Open-World Survival AAA Standard. Save file corruption due to power loss destroys player trust and invalidates 100+ hours of progression.
> **meta** · depends-on: M7-EXT-01

##### Implementation
1. Saves use double-buffered transaction logging: `world_state.wal` + `world_state.bin`.
2. When serializing world state or chunk data, the serializer writes to a temporary transaction file (`world_state.tmp`).
3. Executes `FlushFileBuffers(hFile)` to force all cached OS write pages directly to non-volatile physical storage (NVMe/SSD).
4. Appends a 64-bit BLAKE3 checksum header to `world_state.wal`.
5. Calls `MoveFileExW(L"world_state.tmp", L"world_state.bin", MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)` to perform an atomic file system pointer swap.
6. On startup recovery, if `.wal` checksum mismatches `.bin`, the engine automatically rolls back to the previous verified checkpoint without user intervention.
"""

with open(r'C:\ZombieEngine\spec\_v80_presplit.md', 'a', encoding='utf-8') as f:
    f.write(text)
print('Appended WAL block via script!')
