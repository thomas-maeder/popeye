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
| OA fixed-VSZ       | 4G     | 4112    | 3614 (87%)   | 6:44   | ✓          |
| OA fixed-VSZ       | 10G    | 10256   | 10243 (99%)  | 6:02   | ✓          |
| OA fixed-VSZ       | 20G    | 20496   | 17958 (87%)  | 7:11   | ✓          |
| OA 50/50           | 4G     | 3600    | 3587 (99%)   | 9:31   | ✓          |
| OA 50/50           | 10G    | 8208    | 7471 (91%)   | 4:29   | ✓          |
| OA 100% + dynamic  | 10G    | 13328   | 7471 (56%)   | 5:30   | ✗          |
| Original (chained) | 4G     | 4112    | 4100 (99%)   | >15:00 | ✓          |
| Original (chained) | 10G    | 10256   | 8743 (85%)   | 11:12  | ✓          |

### Key findings:
- Fixed-VSZ is the best approach for constrained environments (fixed memory footprint)
- At 4G: only variant that actually finishes (6:44 vs >15min for original)
- Dead-table overhead (~13%) is inherent to growing within a non-freeable arena
- At 10G the problem fills to 99% (dead tables fit within the extra headroom)
- At 20G there's more budget than needed; 87% utilization = problem just doesn't need 20G

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

1. **Fixed-VSZ (opt 21) is the current implementation** — table lives inside FXF arena, VSZ matches the original exactly.
2. Dead-table overhead (~13%) is unavoidable without either: (a) in-place rehashing (impossible with mask-based OA) or (b) temporary external allocation (violates fixed-VSZ goal).
3. Consider: should the default be changed to include `DHT_OPEN_ADDRESSING`? The OA variant is 2.2× faster on memory-intensive problems with no regression on others.
4. The `dw_ng.04.memtest` and `FS10852.memtest` files are available for regression testing.
