# Popeye DHT Optimization — Session State (Jun 09, 2026)

## Current Status

The project has two compile-time DHT variants controlled by `DHT_OPEN_ADDRESSING`:
- **Default** (`DHT/dht.c`): Original chained DHT, all memory via FXF
- **Open addressing** (`DHT/dht_open_addressing.c`): Linear probing, table inside FXF arena (fixed VSZ)

### Build Commands
```bash
# Default (original):
make -f makefile.unx

# Open addressing:
make -f makefile.unx DEFS="-DSIGNALS -DMSG_IN_MEM -DFXF -DFXF_MAX_ALIGNMENT_TYPE=void* -DFXF_NOT_MULTIPLE_ALIGNMENT_TYPE=short -DNDEBUG -DDHT_OPEN_ADDRESSING"
```

## Current Design (opt 21): Fixed-VSZ In-Arena Table

- `allochash()` in `optimisations/hash.c`: 100% of budget to FXF arena
- `allocTable()` in `DHT/dht_open_addressing.c`: uses `fxfReserveTop()` + memset(0)
- `growTable()`: checks `fxfAvailable()`, allocates new table from arena top
- `freeTable()`: no-op (arena memory not individually freeable)
- VSZ = arena only (single malloc, fixed from startup)
- Dead-table overhead: ~13% of arena wasted by old tables after doublings

## Benchmark Results (FS10852.memtest)

| Variant            | Budget | VSZ(MB) | RSS peak(MB) | Time   | VSZ fixed? |
|--------------------|--------|---------|--------------|--------|------------|
| **OA opt(22) tombstone cleanup** | **4G** | **4112** | **2846 (69%)** | **3:53** | **✓** |
| **OA opt(22) tombstone cleanup** | **10G** | **10256** | **10243 (99%)** | **5:41** | **✓** |
| OA opt(21) fixed-VSZ | 4G | 4112 | 3614 (87%) | 6:44 | ✓ |
| OA opt(21) fixed-VSZ | 10G | 10256 | 10243 (99%) | 6:02 | ✓ |
| OA 50/50 | 4G | 3600 | 3587 (99%) | 9:31 | ✓ |
| OA 50/50 | 10G | 8208 | 7471 (91%) | 4:29 | ✓ |
| Original (chained) | 4G | 4112 | 4100 (99%) | >15:00 | ✓ |
| Original (chained) | 10G | 10256 | 8743 (85%) | 11:12 | ✓ |

### 81-file benchmark:
- Baseline (opt 21): 147.8s
- Current (opt 22): **139.7s** (-5.5%)

### Key findings (opt 22):
- Tombstone cleanup after compression reduced `lookupSlot` from 48.6% → 6.0% of CPU
- The 42% speedup on FS10852 (4G) is due to eliminating probe chain degradation
- RSS dropped from 87% to 69% — cleanup frees memory previously trapped behind tombstones
- The in-place `dhtCleanup` algorithm requires no extra memory allocation
- Universal improvement: the 81-file benchmark also improved by 5.5%

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

1. **Current bottleneck is `ConvertBCMemValue` (24.4%)** — the FNV-1a hash computation. Possible improvements: SIMD hashing, or reducing the number of hash computations per position.
2. `dispatch()` at 9.7% — computed goto / function pointer table could reduce indirect branch overhead.
3. `LargeEncode` at 9.5% — position encoding is now a significant cost. Could benefit from incremental encoding (delta from parent position).
4. The in-place `dhtCleanup` is O(n) per slot — could be optimized with backward-shift deletion during removal instead of bulk cleanup afterward.
5. Consider: should `DHT_OPEN_ADDRESSING` become the default? It's now 2-3× faster on heavy problems and 5.5% faster on the general benchmark with no correctness issues.
