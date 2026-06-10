# Popeye DHT Optimization — Session State (Jun 09, 2026)

## Current Status

The project has two compile-time DHT variants controlled by `DHT_OPEN_ADDRESSING`:
- **Default** (`DHT/dht.c`): Original chained DHT, all memory via FXF
- **Open addressing** (`DHT/dht_open_addressing.c`): Linear probing, 50/50 budget split

### Build Commands
```bash
# Default (original):
make -f makefile.unx

# Open addressing:
make -f makefile.unx DEFS="-DSIGNALS -DMSG_IN_MEM -DFXF -DFXF_MAX_ALIGNMENT_TYPE=void* -DFXF_NOT_MULTIPLE_ALIGNMENT_TYPE=short -DNDEBUG -DDHT_OPEN_ADDRESSING"
```

## Current 50/50 Budget Split Design

- `allochash()` in `optimisations/hash.c`: arena = `nr_kilos / 2`
- `growTable()` in `DHT/dht_open_addressing.c`: table capped at `fxfArenaSize()` (= budget/2)
- Total VSZ = arena + table ≤ budget
- Added `fxfAvailable()` and `fxfArenaSize()` to `DHT/fxf.c` / `DHT/fxf.h`

## Benchmark Results (FS10852.memtest, -maxmem 10G)

| Variant            | VSZ(MB) | RSS peak(MB) | Time   | VSZ ≤ budget? |
|--------------------|---------|--------------|--------|---------------|
| OA 50/50           | 8208    | 7471         | 4:29   | ✓             |
| OA 33/67           | 6501    | 6489         | 6:43   | ✓             |
| OA 25/75           | 5648    | 5635         | 11:31  | ✓             |
| OA 100% + dynamic  | 13328   | 7471         | 5:30   | ✗ (13.3GB)    |
| Original (chained) | 10256   | 8743         | 11:12  | ✓             |

### Why 50/50 is optimal:
- **Same RSS** as 100% dynamic (7471MB) — the problem's data needs are fixed
- **Best time** of all VSZ-bounded variants (4:29)
- **VSZ stays within budget** (~8.2GB < 10GB)
- Pre-allocating the full table upfront causes TLB/cache thrashing (tested: 5.6s on alice.inp vs 3.2s)
- The 100% arena approach gives no speed benefit (5:30 vs 4:29) because `fxfAvailable()` limits the table to the same effective capacity

## Files Modified (relative to develop baseline)

- `DHT/dht_open_addressing.c` — open addressing implementation + growTable budget cap
- `DHT/dht.c` — original chained DHT (with const-correctness fixes for current dht.h)
- `DHT/dht.h` — WithHash APIs guarded by `#ifdef DHT_OPEN_ADDRESSING`
- `DHT/fxf.c` — added `fxfArenaSize()`, `fxfAvailable()`
- `DHT/fxf.h` — declarations for above
- `DHT/makefile.local` — conditional source file selection
- `DHT/dhtbcmem.c` — FNV-1a hash (opt 3, shared by both variants)
- `optimisations/hash.c` — budget split, `DHT_LOOKUP`/`DHT_ENTER` dispatch macros, precomputed hash
- `optimisations/orthodox_square_observation.c` — side_offset caching (opt 10)
- `position/position.h` — inlined find_end_of_line (opt 1)
- `makefile.defaults` — NDEBUG, default has no DHT_OPEN_ADDRESSING
- `toolchains/gcc/make.incl` — -fno-stack-protector
- Various parity fixes (opt 4) in solving/find_shortest.c, stipulation/help_play/branch.c, etc.

## Open Questions / Next Steps

1. **50/50 is confirmed optimal** — tested 25/75, 33/67, 50/50, 100%+dynamic, and pre-allocated. 50/50 wins on both speed and VSZ compliance.
2. Fixed VSZ (like original) requires the table to live inside the arena. Pre-allocation was tested but causes cache/TLB thrashing with large tables. Would need FXF-internal changes (carving table from arena top) to achieve fixed VSZ + good performance.
3. Consider: should the default be changed to include `DHT_OPEN_ADDRESSING`? The OA variant is 2.5× faster on memory-intensive problems with no regression on others.
4. The `dw_ng.04.memtest` and `FS10852.memtest` files are available for regression testing budget enforcement.
