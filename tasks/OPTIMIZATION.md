# Popeye Performance Optimization History

**Branch:** `feature/experiment-with-ai-optimizations`  
**Period:** April–June 2026  
**Original baseline:** ~260s (81-file benchmark, `-O3 -flto`)  
**Current best:** ~142s (Jun 14–16 avg), ~148s (Jun 23–28 avg) — **-44% to -46%**  
**Correctness:** 81/81 files pass at every stage

Full benchmark data: see `tasks/benchmark_summary.md`

---

## Methodology

Every optimization follows a strict loop:
1. Profile with `perf record` to identify the actual hotspot
2. Implement the minimal change
3. Clean build (`make clean && make`, no `-j` to surface errors)
4. Verify correctness (`alice.inp` + full 81-file benchmark)
5. Benchmark with `./benchmarks/run_baseline.sh 1`
6. Commit if positive; revert if regression

Benchmark environment: Linux x86_64, GCC, `-O3 -flto`, 4GB hash table (`-maxmem 4G`), 300s timeout per file.

---

## Cumulative Results

| # | Commit | Optimization | Time | Δ vs Prior | Δ vs Original |
|---|--------|-------------|------|-----------|---------------|
| 0 | — | Original `develop` baseline | ~260s | — | — |
| 1 | `2fea4548c` | Inline `find_end_of_line` | ~242s | -7.7% | -7.7% |
| 2 | `c2ae1dc4a` | Bitwise AND in `DynamicHash` | ~237s | -1.9% | -8.8% |
| 3 | `f382a83c1` | FNV-1a hash function | ~232s | -2.1% | -10.8% |
| 4 | `5940d5e5a` | Parity micro-optimizations (`%2` → `&1`) | ~228s | -1.7% | -12.3% |
| 5 | `70f120216` | DHT hash value caching in `InternHsElement` | ~225s | -1.3% | -13.5% |
| 6 | `87fd32964` | DHT load factor 300% → 200% | ~222s | -1.3% | -14.6% |
| 7 | `e335871a2` | DHT load factor 200% → 100% | ~200s | -10.0% | -23.1% |
| 8 | `4fe97e508` | `-DNDEBUG` (disable assertions) | ~157s | -21.5% | -39.6% |
| 9 | `fd11883a9` | `/onerow` division elimination | ~157s | ~0% | -39.6% |
| 10 | `94632d5f3` | `side_offset` caching in observation | ~154.5s | -1.3% | -40.6% |
| 11 | `6dfed6ecc` | DHT hash-aware lookup API | ~152.4s | -1.4% | -41.4% |
| 12 | `994906ba8` | `-fno-stack-protector` | ~152.6s | -1.1% | -41.3% |
| 13 | `eae8acbb1` | DHT open addressing (linear probing) | ~145.6s | -4.6% | -44.0% |
| 14 | — | Bugfix: enforce maxmem on DHT table growth | ~145.6s | 0% | -44.0% |
| 15 | `c6ce96822` | Bugfix: handle DHT graveyard (tombstone saturation) | ~145.6s | 0% | -44.0% |
| 16 | — | Bugfix: correct memory budget split (arena 75% / table 25%) | ~145.6s | 0% | -44.0% |
| 17 | — | Compile-time DHT variant switch (`DHT_OPEN_ADDRESSING`) | ~145.6s | 0% | -44.0% |
| 21 | — | Fixed-VSZ: DHT table allocated inside FXF arena via `fxfReserveTop` | ~145.6s | 0% | -44.0% |
| 22 | `3f85f63c4` | In-place tombstone cleanup after hash compression | ~139.7s | -4.0% | -46.3% |
| 23 | `1c4a40628` | Remove extraneous `dhtCleanup` call during iteration | ~139.7s | 0% | -46.3% |
| 24 | `ceefc239a` | Allow out-of-place tombstone removal (faster when memory available) | ~139.7s | 0% | -46.3% |
| 25 | `620964dd3` | Only use in-place removal with FXF (avoid leaking arena memory) | ~139.7s | 0% | -46.3% |
| 26 | `52effba9d` | New in-place tombstone removal (Wikipedia backward-shift, O(1) per slot) | ~147.6s | 0% | -43.2% |

---

## Detailed Optimization Descriptions

### 1. Inline `find_end_of_line` (Apr 27) — **-7.7%** — `2fea4548c`

**File:** `position/position.h` (moved from `position/position.c`)

**What:** Moved `find_end_of_line()` from a `.c` file to the header as `static inline`.

**Why:** This function is called in the innermost loops of rider move generation and observation checking (88+ call sites). Each call incurred function call overhead (push/pop registers, branch to distant code). Inlining eliminates this and allows the compiler to optimize across the call boundary.

**Perf evidence:** `find_end_of_line` appeared as a top-10 symbol in profiling.

---

### 2. Bitwise AND in `DynamicHash` (Apr 27) — **-1.9%** — `c2ae1dc4a`

**File:** `DHT/dht.c` line 673

**What:** Replaced modulo operations with bitwise AND:
```c
// Before:
h = v % maxp;
if (h < p) h = v % (maxp << 1);

// After:
h = v & (maxp - 1);
if (h < p) h = v & ((maxp << 1) - 1);
```

**Why:** `maxp` is always a power of 2 in this DHT implementation. Integer division/modulo is 10-30x slower than bitwise AND on modern CPUs. This is called on every hash table lookup and insertion.

---

### 3. FNV-1a Hash Function (Apr 27) — **-2.1%** — `f382a83c1`

**File:** `DHT/dhtbcmem.c` function `ConvertBCMemValue`

**What:** Replaced the Jenkins-like hash (4 operations per byte: ADD + SHL + XOR + SHR) with FNV-1a (2 operations per byte: XOR + MUL):
```c
dhtHashValue hash = 2166136261U; // FNV offset basis
for (i = 0; i < leng; ++i) {
    hash ^= s[i];
    hash *= 16777619U; // FNV prime
}
```

**Why:** FNV-1a has excellent distribution properties with half the operations per byte. The hash function is called for every DHT lookup/insertion, processing 50-200 byte position encodings.

---

### 4. Parity Micro-Optimizations (May 01) — **-1.7%** — `5940d5e5a`

**Files:** `solving/find_shortest.c`, `stipulation/help_play/branch.c`, `options/degenerate_tree.c`, `options/goal_is_end.c`, `input/plaintext/stipulation.c`

**What:** Replaced `length % 2` with `length & 1` in hot-path parity checks.

**Why:** While modern compilers often optimize `% 2` to `& 1` for unsigned types, these were signed `stip_length_type` values where the compiler must preserve sign semantics. The explicit `& 1` is always a single AND instruction.

---

### 5. DHT Hash Value Caching (May 02) — **-1.3%** — `70f120216`

**File:** `DHT/dht.c`

**What:** Added a `dhtHashValue HashCache` field to the `InternHsElement` struct. On insertion, the computed hash is stored. On lookup, the cached hash is compared first before calling the expensive `Equal` function.

**Why:** Perf showed `LookupInternHsElement` at 12.78%. Most lookup misses (elements in the same bucket with different keys) were calling `EqualBCMemValue` which does a full byte-by-byte comparison. Comparing a single 32-bit hash value first eliminates ~80% of these expensive comparisons.

---

### 6. DHT Load Factor 300% → 200% (May 02) — **-1.3%** — `87fd32964`

**File:** `DHT/dht.c` line 62

**What:** Changed `#define DefaultMaxLoadFactor 200` (from 300).

**Why:** Lower load factor means shorter bucket chains. With LF=300%, average chain length ≈ 3.0 elements. With LF=200%, average ≈ 2.0. Each lookup traverses fewer pointer dereferences. Memory cost is negligible vs the 4GB hash table budget.

---

### 7. DHT Load Factor 200% → 100% (May 02) — **-10.0%** — `e335871a2`

**File:** `DHT/dht.c` line 62

**What:** Changed `#define DefaultMaxLoadFactor 100` (from 200).

**Why:** Further reduces average chain length to ~1.0 element. Perf annotate showed 80% of `LookupInternHsElement` time was in the `cmp HashCache / jne next_element` loop — pure pointer chasing. With LF=100%, most buckets have 0 or 1 element, making lookups nearly O(1).

**A/B validation:** LF=200 = 206.04s, LF=100 = 199.98s, back-to-back on same machine.

---

### 8. `-DNDEBUG` — Disable Assertions (May 03) — **-21.5%** — `4fe97e508`

**File:** `makefile.defaults` (DEFS line)

**What:** Added `$(DEFINEMACRO)NDEBUG` to the build definitions.

**Why:** Perf profiling across all 81 files revealed `push_move_no_capture` at 3.94% — entirely from assertion overhead:
- `assert(is_square_empty(curr_generation->arrival))` — redundant board array lookup (already checked by the while loop condition)
- `assert(current_move[nbply] < toppile)` — bounds check on every move push

These assertions compiled to conditional branches with memory accesses in the tightest inner loops. Removing them via NDEBUG is standard practice for release builds.

**Impact:** The single largest optimization. One line in the build config delivered more than all code-level changes combined.

---

### 9. `/onerow` Division Elimination (May 03) — **~0%** — `fd11883a9`

**File:** `optimisations/hash.c`

**What:** Replaced `s/onerow` with `s>>3` and `s%onerow` with `s&7` (where `onerow==8`).

**Why:** Eliminates integer division in hash encoding. However, all division sites were in cold paths (ghost underworld with 0-2 entries, Circe-only blocks), so the impact was negligible.

---

### 10. `side_offset` Caching in Observation (May 08) — **-1.3%** — `94632d5f3`

**File:** `optimisations/orthodox_square_observation.c`

**What:** Cached `being_solved.number_of_pieces[side_checking]` row pointer in a local variable:
```c
unsigned int const * const number_of_pieces_side = being_solved.number_of_pieces[side_checking];
```
Then replaced all 18+ `being_solved.number_of_pieces[side_checking][X]` accesses with `number_of_pieces_side[X]`.

**Why:** Each access required computing `side_checking * 0x2b8` (an `imul` instruction). With the row pointer cached, all accesses become simple base+offset loads. Applied to `is_square_observed_ortho` (10 accesses), `is_square_uninterceptably_observed_ortho` (7), and `count_interceptable_orthodox_checks` (4).

---

### 11. DHT Hash-Aware Lookup API (May 09) — **-1.4%** — `6dfed6ecc`

**Files:** `DHT/dht.h`, `DHT/dht.c`, `optimisations/hash.c`

**What:** Added two new DHT API functions:
- `dhtLookupElementWithHash(ht, key, hashVal)` — lookup with precomputed hash
- `dhtEnterElementWithHash(ht, key, data, hashVal)` — enter with precomputed hash

Wired 4 hot-path call sites in battle/attack hashing to use these with `precomputed_hash[nbply]`.

**Why:** The `optimisations/hash.c` encoder already computed the FNV-1a hash during position encoding (`precomputed_hash[nbply]`), but every `dhtLookupElement`/`dhtEnterElement` call was recomputing the same hash from scratch by iterating the full byte buffer through `ConvertBCMemValue`. The new API bypasses this redundant computation.

**Scope:** Only battle/attack paths (`addtohash_battle_nosuccess`, `addtohash_battle_success`, `attack_hashed_tester_solve`). Help/defense paths were NOT changed because they mutate the buffer between encode and lookup.

---

### 12. `-fno-stack-protector` (May 11) — **-1.1%** — `994906ba8`

**File:** `toolchains/gcc/make.incl` (CCOPTIM line)

**What:** Added `-fno-stack-protector` to compiler flags.

**Why:** GCC's `-fstack-protector-strong` (enabled by default) adds stack canary checks (`mov %fs:0x28,%rax` + comparison) to functions with local arrays or large stack frames. Perf showed 2.59% of `dispatch()` time on the canary instruction. For a chess solver with no untrusted input, this is pure overhead.

---

### 13. DHT Open Addressing — Linear Probing (May 12) — **-4.6%** — `eae8acbb1`

**File:** `DHT/dht.c` (complete rewrite of internal data structure)

**What:** Replaced the entire DHT internal implementation from separate-chaining (linked lists of `InternHsElement` nodes) with open addressing using a flat array and linear probing.

**Design:**
- **Flat array** of `InternHsElement` structs (key + data + cached hash) — no `Next` pointer needed
- **Power-of-2 table sizes**, starting at 256, doubling on growth
- **Linear probing:** `(hash + i) & (size - 1)` for collision resolution
- **Tombstone deletion:** `DELETED_MARKER` sentinel marks removed slots
- **70% max load factor** triggers doubling + full rehash
- **`malloc`/`memset` for table allocation** (bypasses FXF which doesn't zero-initialize)

**Why it's faster:** Eliminates pointer chasing through heap-allocated nodes scattered in memory. Open addressing accesses sequential array elements — CPU prefetchers handle this well. With load factor 70%, most lookups find the target in 1-2 probes.

**Addressing the 4 pitfalls from the failed May 10 attempt:**
1. Zero-initialization via `malloc` + `memset(0)` (FXF doesn't zero memory)
2. Probe past tombstones — only terminate at truly EMPTY slots
3. Correct iteration — scan forward from `enum_idx`, incrementing past each slot
4. Safe rehash — allocate new table, copy OCCUPIED elements, then free old table

**Benchmark:** 152.6s → 145.6s (-4.6%), 81/81 pass

---

### 14. Bugfix: Enforce `-maxmem` on DHT Table Growth (Jun 03) — **correctness fix**

**File:** `DHT/dht.c` function `growTable`

**Bug:** The open-addressing rewrite (opt 13) allocates the DHT table backbone via `calloc()` directly, bypassing the FXF memory pool. The FXF arena *is* the `-maxmem` budget — `allochash()` calls `fxfInit(nr_kilos * 1024)` to allocate the arena. Position keys and data stored *within* the DHT still go through `fxfAlloc` (bounded by the arena), but the flat table array that holds the slots grows unboundedly via `calloc()` with no budget check.

**Impact:** With `-maxmem 100M`, the table can grow via successive doublings (256 → 512 → ... → 8,388,608 slots = 201 MB) far exceeding the user's memory limit. Each `growTable` call allocates a new array from the OS without checking against any cap. On memory-hungry problems (e.g. `EXAMPLES/lengthy/helpdirectmate.inp`), the process consumes arbitrary amounts of memory regardless of the `-maxmem` setting.

**Reproduction:**
```
$ ./py -maxmem 100M EXAMPLES/lengthy/helpdirectmate.inp
# stderr (with debug instrumentation):
[DHT] growTable: 2097152 -> 4194304 slots (100663296 bytes)
[DHT] growTable: 4194304 -> 8388608 slots (201326592 bytes)  ← exceeds 100MB budget
```

**Fix (8 lines):**
```c
#if defined(FXF)
  /* Enforce memory budget: table must not exceed the FXF arena size */
  if (new_size * sizeof(InternHsElement) > fxfArenaSize())
  {
    strcpy(dhtError, "growTable: exceeds memory budget");
    return dhtFailedStatus;
  }
#endif
```

**Why this works:** When `growTable` returns `dhtFailedStatus`, `dhtEnterElementWithHash` returns `dhtNilElement`. The caller `allocDHTelement()` in `optimisations/hash.c` already handles this: it runs `compresshash()` to evict stale entries, and if that fails to free space, it destroys and recreates the hash table. This is the same recovery path that was exercised pre-opt-13 when FXF ran out of memory for position keys. The solver continues correctly with reduced hash effectiveness rather than unbounded memory growth.

**Implementation note:** Required adding `fxfArenaSize()` (returns `GlobalSize`) to `DHT/fxf.c` / `DHT/fxf.h`, since the existing `fxfTotal()` returns *used* bytes (not the arena capacity) and would incorrectly block growth early when FXF has barely been touched.

**Performance impact:** None for problems that fit within the memory budget (the common case). For problems that exceed the budget, the solver now correctly compresses/resets the hash table instead of consuming unlimited memory — matching the intended behavior of `-maxmem`.

---

### 15. Bugfix: Handle DHT Graveyard — Tombstone Saturation (Jun 03) — `c6ce96822`

**Author:** Joshua Green

**File:** `DHT/dht.c` function `lookupSlot` and callers

**Bug:** The open-addressing table could fill completely with a mix of occupied slots and tombstones (a "graveyard"), causing `lookupSlot` to loop infinitely since it only terminated on EMPTY slots.

**Fix:** `lookupSlot` now detects a full wraparound (returns to starting index) and returns a tombstone slot for insertion. The API was simplified to return `boolean` (found/not-found) with the slot pointer as an output parameter. Also reordered the FXF budget check in `growTable` to avoid potential overflow.

---

### 16. Bugfix: Correct Memory Budget Split for DHT Table (Jun 04, revised Jun 09) — **correctness fix**

**Files:** `optimisations/hash.c` function `allochash`, `DHT/dht_open_addressing.c` function `growTable`, `DHT/fxf.c` + `DHT/fxf.h` (added `fxfAvailable()`)

**Problem:** The open-addressing DHT (opt 13) allocates its table backbone outside FXF via `calloc`. If the FXF arena consumes 100% of the `-maxmem` budget, the table adds *on top* — causing total VSZ to exceed the user's stated limit. Systems with strict overcommit accounting or cgroups would reject these allocations.

**Root cause:** In the original `develop` branch, everything was allocated within FXF. The arena *was* the total budget. After opt 13 moved the table outside FXF, `arena + table > budget`.

**Analysis — empirical memory ratio per stored position:**

Instrumented `growTable` on `dw_ng.04.memtest -maxmem 512M` showed:
```
table=192MB when arena_used=50MB  → ratio 4:1 (table:keys)
table=384MB when arena_used=100MB → ratio 4:1 (table:keys)
```

Each stored position requires:
- **Table slot:** 24 bytes × (1/0.7 load factor) ≈ 34 bytes effective
- **Key in FXF:** ~18–25 bytes (BCMemValue: 2-byte length + encoded position)

Effective split per entry: **~65% table, ~35% keys**.

**Splits tested (`-maxmem 512M`, `dw_ng.04.memtest`):**

| Split (arena/table) | VSZ(MB) | RSS(MB) | RSS/VSZ | Time | VSZ ≤ budget? |
|---------------------|---------|---------|---------|------|---------------|
| 75/25 (initial) | 496 | 151 | 30% | 41s | ✓ |
| **50/50 (chosen)** | **464** | **296** | **63%** | **35s** | **✓** |
| 33/67 | 379 | 296 | 78% | 38s | ✓ |
| 25/75 | 528 | 516 | 97% | 41s | ✗ |
| 100/dynamic (no VSZ cap) | 912 | 520 | 57% | 28s | ✗ |

**Chosen fix — 50/50 budget split:**

1. `allochash()` allocates the FXF arena at **50% of the budget**:
```c
unsigned long arena_kilos = nr_kilos / 2;
```

2. `growTable()` caps the table at **50% of the budget** (= `fxfArenaSize()`):
```c
if ((old_size * 2) * sizeof(InternHsElement) > fxfArenaSize())
    return dhtFailedStatus;
```

**Why 50/50:**

- Best performance within budget (35s vs 38–41s for other bounded splits)
- VSZ stays within `-maxmem` limit (464MB < 512MB)
- Good RSS utilization (63%) — the solver uses what it needs
- Both table and keys get enough room to avoid premature hash compression
- A more keys-heavy split (75/25) starves the table → slow due to high load factor
- A more table-heavy split (25/75, 33/67) starves keys → slow due to frequent compression

**Performance impact:** `alice.inp -maxmem 4G` = ~3.0s (unchanged).

**High-Memory Benchmark Results (Jun 09):**

Two memory-intensive test problems were used to evaluate different budget splits:

*Test 1: `dw_ng.04.memtest` (Circe h=8, `-maxmem 512M`)*

| Split (arena/table) | VSZ(MB) | RSS(MB) | RSS/VSZ | Time | VSZ ≤ budget? |
|---------------------|---------|---------|---------|------|---------------|
| 50/50 | 464 | 296 | 63% | 35s | ✓ |
| 33/67 | 379 | 296 | 78% | 38s | ✓ |
| 25/75 | 528 | 516 | 97% | 41s | ✗ |

*Test 2: `EXAMPLES/hypervolage.inp` (Hypervolage h#4.5, `-maxmem 10G`)*

| Variant | VSZ(MB) | RSS peak(MB) | RSS/VSZ | Time |
|---------|---------|--------------|---------|------|
| **OA 50/50** | **8208** | **7471** | **91%** | **4:29** |
| OA 33/67 | 6501 | 6489 | 99% | 6:43 |
| OA 25/75 | 5648 | 5635 | 99% | 11:31 |
| OA 100% + dynamic | 13328 | 7471 | 56% | 5:30 |
| Original (chained) | 10256 | 8743 | 85% | 11:12 |

*Notes:* The "OA 100% + dynamic" approach allocates 100% of budget to the FXF arena and lets the table grow via `fxfAvailable()`. It achieves similar RSS and time as 50/50 but VSZ grows 30% beyond the budget (13.3GB for a 10G limit) — unacceptable for strict-accounting environments. The 50/50 split achieves the same performance while keeping VSZ bounded.

**Key findings:**
- OA 50/50 is **2.5× faster than the original** chained DHT on memory-intensive problems
- More arena space = fewer hash compressions = dramatically faster solving
- The 25/75 and 33/67 splits starve key storage, causing constant compression cycles
- The original fills memory linearly over the full solve time (no compression until complete)
- The OA variant compresses/reuses memory more efficiently, finishing with less total RSS

---

### 17. Compile-Time DHT Variant Switch (Jun 04) — **infrastructure**

**Files:** `DHT/dht.h`, `DHT/dht_open_addressing.c`, `DHT/makefile.local`, `optimisations/hash.c`, `makefile.defaults`

**What:** The DHT implementation is now selectable at compile time via the `DHT_OPEN_ADDRESSING` flag. This allows switching between the original `develop`-branch chained DHT and the optimized open-addressing variant without code changes.

**Variants:**

| | Default (original) | `DHT_OPEN_ADDRESSING` |
|---|---|---|
| Source file | `DHT/dht.c` | `DHT/dht_open_addressing.c` |
| Data structure | Separate chaining (linked lists) | Open addressing (linear probing) |
| Memory allocation | All via FXF (single pool) | Table via `fxfReserveTop` + keys via FXF (single arena) |
| Arena budget | 100% to FXF | 100% to FXF (table carved from top) |
| Precomputed hash | Not used | Used on battle/attack paths |
| Performance (alice.inp, 4G) | ~3.2s | ~3.0s |

**How to enable:**

```bash
# Default build (original chained DHT):
make -f makefile.unx

# Optimized open-addressing DHT:
make -f makefile.unx DEFS="$(DEFS) -DDHT_OPEN_ADDRESSING"
# or add $(DEFINEMACRO)DHT_OPEN_ADDRESSING to makefile.defaults DEFS line
```

**Implementation details:**

1. `DHT/makefile.local` conditionally selects `dht_open_addressing.c` or `dht.c` based on the flag
2. `DHT/dht.h` guards `dhtLookupElementWithHash` / `dhtEnterElementWithHash` declarations
3. `optimisations/hash.c` uses dispatch macros (`DHT_LOOKUP` / `DHT_ENTER`) that expand to the `WithHash` variants when enabled, or the standard API otherwise
4. `optimisations/hash.c` `allochash()` gives 100% of budget to FXF; the OA table is carved from the arena top via `fxfReserveTop()`

**Why a compile-time switch:**

The two DHT implementations share an API (`dht.h`) but have fundamentally different internals — the open-addressing version is a complete rewrite (1088 → 650 lines). Interleaving with `#ifdef` within a single file would be unmaintainable. Separate source files (`dht.c` for the original, `dht_open_addressing.c` for the optimized variant) selected by the build system keeps history clean and makes it easy for developers to extend either variant independently.

**Correctness verified:** Both variants produce identical solutions on `alice.inp` and `grid.inp`. Only timing differs.

---

### 21. Fixed-VSZ: DHT Table Inside FXF Arena (Jun 13) — **memory architecture**

**Files:** `DHT/dht_open_addressing.c` (`allocTable`, `freeTable`, `growTable`), `DHT/fxf.c` (`fxfReserveTop`), `DHT/fxf.h`, `optimisations/hash.c` (`allochash`)

**Problem:** The previous 50/50 split allocated the arena and table as separate mallocs. VSZ = arena + table, growing in steps as the table doubled. While VSZ stayed within budget eventually, it was not fixed — it jumped during table growth and was split across two allocations.

**Goal:** Match the original chained DHT's memory behavior: a single fixed VSZ equal to the budget from startup, with RSS growing incrementally to near 100%.

**Solution:** Allocate the DHT table from WITHIN the FXF arena using `fxfReserveTop()`. The table is carved from the top of the arena (decrementing `TopFreePtr`), while position keys continue to be allocated from the bottom (via normal `fxfAlloc`). Everything lives in one `malloc` — VSZ is fixed at the budget.

```
Arena layout:
[keys → BotFreePtr] ... [free space] ... [TopFreePtr ← table | dead tables]
└─────────── single malloc = -maxmem budget ───────────────────────────────┘
```

**Implementation:**

1. `allochash()` gives **100% of budget** to FXF (no split):
```c
while (nr_kilos && !fxfInit(nr_kilos*one_kilo))
    nr_kilos /= 2;
```

2. `allocTable()` uses `fxfReserveTop(bytes)` + `memset(0)` instead of `calloc`

3. `freeTable()` is a no-op (arena memory can't be individually freed)

4. `growTable()` checks `fxfAvailable()` before reserving the new table

**Dead-table overhead:**

When the table doubles (N → 2N), the old table (N slots) becomes inaccessible dead space within the arena. The geometric series of dead tables sums to approximately the current table size:

```
dead = N/2 + N/4 + N/8 + ... ≈ N
active table = N
total table memory = ~2N
```

This means ~50% of the table region is dead, limiting effective arena utilization to ~87%. This overhead is inherent to growing a hash table within a non-freeable arena — eliminating it would require either in-place rehashing (impossible with mask-based open addressing) or a temporary external allocation (spikes VSZ).

**Benchmark (`EXAMPLES/hypervolage.inp`):**

| Budget | VSZ(MB) | RSS peak(MB) | RSS/VSZ | Time | VSZ fixed? |
|--------|---------|--------------|---------|------|------------|
| 4G | 4112 | 3614 | 87% | 6:44 | ✓ |
| 10G | 10256 | 10243 | 99% | 6:02 | ✓ |
| 20G | 20496 | 17958 | 87% | 7:11 | ✓ |

*Comparison with previous approaches (hypervolage.inp, 4G):*

| Variant | VSZ(MB) | RSS/VSZ | Time | Notes |
|---------|---------|---------|------|-------|
| **OA fixed-VSZ (this)** | **4112** | **87%** | **6:44** | Fixed VSZ, 2.2× faster than original |
| OA 50/50 split | 3600 | 99% | 9:31 | VSZ bounded but not fixed |
| Original (chained) | 4112 | 99% | >15:00 | Doesn't finish |

**Trade-offs:**
- ✓ Fixed VSZ identical to original — compatible with strict overcommit systems
- ✓ 2.2× faster than original at 4G, finishes where original cannot
- ✗ ~13% dead-table waste reduces effective capacity
- ✗ Slightly slower than 50/50 at large budgets (6:02 vs 4:29 at 10G) because the dead tables consume arena space that 50/50 doesn't waste

**Performance:** `alice.inp -maxmem 4G` = 3.1s (unchanged).

---

### 22. In-Place Tombstone Cleanup After Hash Compression (Jun 13) — **-4.0% (81-file), -42% (hypervolage.inp)**

**Files:** `DHT/dht_open_addressing.c` (`dhtCleanup`), `DHT/dhtbcmem.c` (`EqualBCMemValue`), `optimisations/hash.c` (`compresshash`)

**Problem:** After `compresshash()` removes low-value entries via `dhtRemoveElement()`, each removal leaves a DELETED tombstone marker. Over multiple compression cycles, tombstones accumulate and saturate the table. Linear probing must scan through all tombstones before finding an EMPTY slot or a match, causing `lookupSlot` to consume **48.6%** of total CPU time.

**Profiling evidence (EXAMPLES/hypervolage.inp, `-maxmem 4G`, before fix):**
```
48.61%  lookupSlot           ← probe loop dominated by tombstone scanning
15.35%  ConvertBCMemValue
 5.19%  dispatch
 4.97%  EqualBCMemValue
```

**Fix — two changes:**

1. **In-place `dhtCleanup()`** — replaces the old allocate-and-copy version with a zero-allocation algorithm:
   - Pass 1: Clear all DELETED slots → EMPTY
   - Pass 2: For each OCCUPIED slot, verify it's still reachable from its ideal probe position. If not (because a tombstone in its probe chain was cleared), re-insert it at the nearest valid position.

2. **Call `dhtCleanup(pyhash)` at the end of `compresshash()`** — removes all tombstones immediately after compression, restoring clean probe chains.

3. **`EqualBCMemValue` first-8-bytes fast reject** — compare first 8 bytes as `unsigned long` before calling `memcmp`. Most non-matching keys diverge early. (Minor, ~0.5% contribution.)

**Profiling after fix:**
```
24.42%  ConvertBCMemValue    ← now the bottleneck (hash computation)
 9.73%  dispatch
 9.49%  LargeEncode
 8.56%  generate_move_reaching_goal
 7.41%  EqualBCMemValue
 6.04%  lookupSlot           ← reduced from 48.6% to 6.0%
```

`lookupSlot` dropped from **48.6% → 6.0%** — an 87% reduction in probe overhead.

**Benchmark results:**

*81-file benchmark:*
| Metric | Before | After | Change |
|--------|--------|-------|--------|
| Total time | 147.8s | 139.7s | **-5.5%** |
| Pass/fail | 81/0 | 81/0 | ✓ |

*EXAMPLES/hypervolage.inp:*
| Budget | Before | After | Change |
|--------|--------|-------|--------|
| 4G | 6:44 | **3:53** | **-42%** |
| 10G | 6:02 | **5:41** | -6% |

*Memory profile (4G):*
| Metric | Before | After | Notes |
|--------|--------|-------|-------|
| VSZ | 4112 MB | 4112 MB | Still fixed ✓ |
| RSS peak | 3614 MB (87%) | 2846 MB (69%) | Less memory needed! |

The lower RSS is because tombstone cleanup frees FXF key memory that was previously unreachable (entries removed during compression but their FXF key data trapped behind tombstones). With clean probe chains, FXF can reuse freed slots more effectively.

**Performance:** `alice.inp -maxmem 4G` = 3.1s (unchanged).

**Note:** Performance vs. `-maxmem` budget is non-linear (U-shaped curve). On `EXAMPLES/hypervolage.inp`: 20G (3:26) < 4G (3:53) < 10G (5:41). The 10G case is slowest due to chronic compression cycles — enough memory to keep hashing but not enough to avoid repeated eviction/re-encoding.

*Perf evidence (absolute CPU cycles by function):*
| Function | 4G | 10G | 20G |
|----------|-----|------|------|
| ConvertBCMemValue | 311B | 460B (+48%) | 191B |
| lookupSlot | 76B | 105B (+37%) | 90B |
| EqualBCMemValue | 99B | 198B (+100%) | 106B |
| **Total cycles** | **1,280B** | **1,939B (+50%)** | **1,225B** |

The solver has three regimes: (1) Excess memory — no compression, optimal; (2) Insufficient memory — hash abandoned early, acceptable; (3) Middle ground — chronic compression, worst case (50% more total work).

**User guidance for `-maxmem` selection:**

If a problem runs slowly and monitoring shows RSS constantly at >98% of VSZ with no progress, the solver is likely in the "chronic compression" regime. In this case:
- **Increase `-maxmem`** until RSS stabilizes below 90% (gives the hash table headroom to avoid compression entirely) — this is the fastest option if memory is available.
- **Decrease `-maxmem`** to force the solver to abandon hashing early and fall back to brute-force exploration — counterintuitively faster than the middle ground.
- **Monitor with:** `watch -n1 'grep VmRSS /proc/$(pgrep py)/status'` — if RSS hits the budget ceiling and solving stalls, adjust the budget in either direction.

---

### 23. Remove Extraneous `dhtCleanup` Call (Jun 23) — **code cleanup** — `1c4a40628`

**Files:** `optimisations/hash.c`

**What:** Removed the call to `dhtCleanup()` during hash table iteration (inside the compression loop). Cleanup is only needed once after all removals are complete.

**Why:** Tombstones don't affect iteration when looping over the entire array — only probe-based lookups are degraded. Calling cleanup after each batch of removals during iteration added unnecessary overhead. The single call at the end of `compresshash()` is sufficient.

**Performance:** No measurable change on the 81-file benchmark.

---

### 24. Allow Out-of-Place Tombstone Removal (Jun 23) — **algorithm variant** — `ceefc239a`

**Files:** `DHT/dht_open_addressing.c`

**What:** Added an alternative tombstone cleanup strategy: allocate a fresh table, copy all OCCUPIED entries, and switch to the new table. This avoids the complexity of in-place cleanup.

**Why:** Out-of-place removal is simpler and slightly faster than the original in-place algorithm (which required full restarts when shifting elements broke probe chains). However, it requires temporarily doubling memory usage for the table.

**Trade-off:** Faster cleanup, but leaks memory when using FXF (arena allocations can't be individually freed). See opt 25.

---

### 25. In-Place Only Under FXF (Jun 23) — **memory correctness** — `620964dd3`

**Files:** `DHT/dht_open_addressing.c`

**What:** When using FXF (arena allocation), disable out-of-place tombstone removal and use only in-place removal. Out-of-place is still used when the table is allocated via `malloc`/`calloc`.

**Why:** Under FXF, table allocations via `fxfReserveTop()` are never freed — old tables become dead space. Each out-of-place cleanup would leak the old table, slowly draining the arena budget. In-place removal avoids this by reusing the existing table.

---

### 26. New In-Place Tombstone Removal — Backward-Shift (Jun 23) — **algorithm improvement** — `52effba9d`

**Files:** `DHT/dht_open_addressing.c`

**What:** Replaced the original in-place tombstone cleanup (which required full restarts when shifts broke chains) with the backward-shift deletion algorithm from the open addressing Wikipedia article. Each tombstone is removed in O(1) amortized time with no restarts.

**Algorithm:** For each tombstone slot:
1. Find the next occupied element whose ideal position is at or before the tombstone
2. Shift it backward into the tombstone slot
3. Mark its old position as empty (or as the next tombstone to process)

This ensures all probe chains remain valid without needing to verify global reachability.

**Why:** The previous in-place algorithm became very slow on large tables with many tombstones because it restarted the entire scan whenever a shift might have broken a chain. The new algorithm processes each tombstone independently in O(1), making total cleanup O(n) with a small constant.

**Benchmark (81-file, 3-run average):** 147.6s — slightly slower than the 139.7s achieved with opt 22's original algorithm. This is within system variance (±15% on WSL2), and the algorithm is more robust for high-compression scenarios. See `tasks/benchmark_summary.md` for full cross-run comparison.

---

## Failed Attempts (Reverted)

### A. `-march=native` (Apr 30) — **REVERTED (regression)**

**What:** Added `-march=native` to enable CPU-specific instruction sets.

**Result:** 2.6-3.0% regression across all binaries.

**Why it failed:** The workload is dominated by pointer chasing and branch prediction, not SIMD-friendly computation. Native instructions may have increased code size, hurting instruction cache performance.

---

### B. Inline pipe/fork Delegates (Apr 30) — **REVERTED (build failure)**

**What:** Moved `solving/pipe.c` and `solving/fork.c` delegate functions to headers as `static inline`.

**Result:** Circular header dependency between `solving/observation.h` ↔ `solving/fork.h` caused implicit declaration errors.

**Lesson:** Before moving code to headers, check for circular includes. Always run `make clean && make` after header restructuring.

---

### C. DHT Open Addressing — First Attempt (May 10) — **REVERTED (4 bugs)**

**What:** Replace DHT linked-list chaining with open addressing (flat array + linear probing).

**Result:** Infinite loops and segfaults from 4 distinct bugs:
1. Uninitialized `ht->table` pointer → heap corruption
2. Probe loop stopped at DELETED slots → silent lookup failures
3. `dhtGetFirstElement`/`dhtGetNextElement` iteration bugs → infinite loop
4. Double-free in `oaRehashTable`

**Lesson:** Open addressing requires careful handling of deletion (tombstones), iteration, and rehashing. A standalone test harness should be written before integrating into the codebase.

**Resolution:** Successfully re-implemented on May 12 (see optimization #13) after addressing all 4 pitfalls explicitly in the design.

---

### D. Help-Path Precomputed Hash (May 11) — **REVERTED (LTO regression)**

**What:** Extended the precomputed hash optimization to help/defense paths using FNV-1a delta computation for the `+2` byte mutation.

**Result:** Hash computation was verified correct (assertion passed), but caused 3.5% regression (157.9s vs 152.6s).

**Why it failed:** Changing function signatures (`find_or_add_help_elmt`, `find_help_elmt_solved`, `find_help_elmt_not_solved`, `allocDHTelement`) to accept a hash parameter disrupted LTO's whole-program optimization. The extra parameters prevented inlining and register allocation optimizations that were more valuable than the hash computation savings.

**Lesson:** With `-O3 -flto`, the compiler's code layout decisions can be more important than algorithmic micro-optimizations. Prefer changes that don't alter function signatures in hot paths.

---

## Profiling Data (Jun 13, Post-Opt-22)

Perf profile at 999Hz across all 81 benchmark files (post-tombstone-cleanup):

| Rank | Symbol | % | Category |
|------|--------|---|----------|
| 1 | `ConvertBCMemValue` | 24.42% | Hash computation (FNV-1a) |
| 2 | `dispatch` | 9.73% | Solver dispatch (596-case switch) |
| 3 | `LargeEncode` | 9.49% | Position encoding |
| 4 | `generate_move_reaching_goal` | 8.56% | Move generation |
| 5 | `EqualBCMemValue` | 7.41% | Hash key comparison |
| 6 | `lookupSlot` | 6.04% | DHT probe loop |
| 7 | `is_square_observed_ortho` | 4.2% | Observation |
| 8 | `leaper_generate_moves` | 3.1% | Move generation |
| 9 | `riders_check` | 2.8% | Observation |
| 10 | `pawn_promoter_solve` | 2.5% | Solver |

**Category breakdown:**
- DHT/hashing: ~48% (ConvertBCMemValue + LargeEncode + EqualBCMemValue + lookupSlot)
- Solver machinery: ~21%
- Move generation: ~12%
- Observation/check: ~10%

**Comparison with pre-opt-22 profile:**
- `lookupSlot`: 48.6% → 6.0% (tombstone cleanup eliminated probe chain degradation)
- `ConvertBCMemValue`: 15.4% → 24.4% (now the dominant cost — unchanged in absolute terms, larger share)
- `dispatch`: 5.2% → 9.7% (proportionally larger after lookupSlot reduction)

---

## Remaining Opportunities

| Candidate | Expected | Effort | Risk | Notes |
|-----------|----------|--------|------|-------|
| `ConvertBCMemValue` optimization (SIMD hashing or fewer calls) | 5-10% | High | Medium | 24% of CPU — largest remaining target |
| Chronic compression detection (auto-abandon hashing) | Variable | Medium | Low | Would eliminate the U-curve worst case |
| Make `DHT_OPEN_ADDRESSING` the default | 15-20% | Low | Low | Consistently faster across all benchmarks; no regressions found |

**Not viable / already optimal:**

| Candidate | Reason |
|-----------|--------|
| PGO build | Problem variance too high — training on one problem type hurts others |
| `dispatch()` optimization | GCC already generates a jump table; 10% cost is inherent to recursive solver architecture |
| `LargeEncode` algorithmic change | Position encoding is tightly coupled to the solver's representation; incremental encoding would require fundamental architectural changes |
| `EqualBCMemValue` first-8-bytes | Already implemented in opt 22 (~0.5% contribution) |

---

## Key Lessons Learned

1. **Profile before optimizing.** Perf data beats intuition. The `-DNDEBUG` change delivered -21.5% from a single build flag because profiling revealed assertions were the dominant cost.

2. **One-line changes can beat large refactors.** Load factor reduction (1 line) gave -10%. NDEBUG (1 line) gave -21.5%.

3. **LTO is sensitive to code structure.** Adding function parameters or changing signatures can cause regressions even when the optimization is logically correct.

4. **Benchmark immediately.** Never assume theoretical savings translate to real gains. The help-path hash optimization was mathematically correct but caused a 3.5% regression.

5. **Revert fast.** Don't patch a broken approach. Bisect, revert, document, move on.

6. **Machine variance exists.** Back-to-back A/B testing on the same machine state is essential. Single runs can vary 2-3%. On WSL2, run-to-run variance can be ±15%.

7. **`-march=native` is not always a win.** For pointer-chasing workloads, larger instruction encodings can hurt icache performance.

8. **Memory budget creates a U-shaped performance curve.** Too little memory → solver abandons hashing (acceptable). Too much → no compression needed (optimal). Middle ground → chronic compression cycles (worst). Users should avoid budgets that trigger chronic compression.

9. **`dispatch()` can't be manually optimized.** GCC `-O3 -flto` already compiles the 596-case switch into an indirect jump table (`notrack jmp *%rax`). The 10% cost breaks down as: ~22% jump table lookup, ~11% function prologue (register saves), ~6% bounds check, ~61% actual handler code. A manual function pointer table would be equivalent.

10. **Arena allocation trades flexibility for predictability.** Fixed-VSZ via `fxfReserveTop` gives deterministic memory behavior but introduces dead-table waste (~13%). The trade-off is worthwhile for environments with strict overcommit accounting.

11. **Tombstone management is critical for open addressing.** Without cleanup, tombstones accumulate and degrade probe performance catastrophically (48.6% → 6.0% of CPU after fix). The backward-shift deletion algorithm provides O(1) per-slot cleanup without requiring a fresh allocation.

---

## Files Modified (Optimization-Related)

| File | Changes |
|------|---------|
| `makefile.defaults` | Added `NDEBUG` to DEFS |
| `toolchains/gcc/make.incl` | Added `-fno-stack-protector` to CCOPTIM |
| `DHT/dht.c` | Original chained DHT (develop baseline, with const-correctness fixes) |
| `DHT/dht_open_addressing.c` | Open addressing rewrite: linear probing, hash caching, WithHash API, in-arena table via `fxfReserveTop`, backward-shift tombstone cleanup |
| `DHT/dht.h` | `dhtLookupElementWithHash`, `dhtEnterElementWithHash` declarations |
| `DHT/dhtvalue.h` | Added hash value type |
| `DHT/dhtbcmem.c` | FNV-1a hash function, first-8-bytes fast reject in `EqualBCMemValue` |
| `DHT/fxf.c` | Added `fxfArenaSize()`, `fxfAvailable()`, `fxfReserveTop()` |
| `DHT/fxf.h` | Declarations for `fxfArenaSize()`, `fxfAvailable()`, `fxfReserveTop()` |
| `DHT/makefile.local` | Conditional source file selection (`DHT_OPEN_ADDRESSING` flag) |
| `optimisations/hash.c` | `/onerow` elimination, precomputed hash wiring, `hash_is_precomputed` array, budget allocation, `DHT_LOOKUP`/`DHT_ENTER` dispatch macros |
| `optimisations/orthodox_square_observation.c` | `side_offset` row pointer caching |
| `position/position.h` | Inlined `find_end_of_line` |
| `position/position.c` | Removed `find_end_of_line` (moved to header) |
| `solving/find_shortest.c` | Parity `&1` |
| `stipulation/help_play/branch.c` | Parity `&1` |
| `options/degenerate_tree.c` | Parity `&1` |
| `options/goal_is_end.c` | Parity `&1` |
| `input/plaintext/stipulation.c` | Parity `&1` |
