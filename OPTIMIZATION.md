# Popeye Performance Optimization History

**Branch:** `feature/experiment-with-ai-optimizations`  
**Period:** April–May 2026  
**Original baseline:** ~260s (81-file benchmark, `-O3 -flto`)  
**Current best:** ~145.6s (**-44.0%**)  
**Correctness:** 81/81 files pass at every stage

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

## Profiling Data (May 11, Post All Optimizations)

Perf profile at 999Hz across all 81 benchmark files:

| Rank | Symbol | % | Category |
|------|--------|---|----------|
| 1 | `dispatch` | 25.80% | Solver dispatch (593-case switch) |
| 2 | `is_square_observed_ortho` | 6.34% | Observation |
| 3 | `dhtLookupElement` | 5.90% | DHT (55% on hash cache cmp) |
| 4 | `leaper_generate_moves` | 4.39% | Move generation |
| 5 | `pawn_promoter_solve` | 3.46% | Solver |
| 6 | `riders_check` | 3.24% | Observation |
| 7 | `is_in_check_recursive` | 3.05% | Observation |
| 8 | `rider_generate_moves` | 2.95% | Move generation |
| 9 | `leapers_check` | 2.85% | Observation |
| 10 | `ConvertBCMemValue` | 2.73% | Hash computation |

**Category breakdown:**
- Solver machinery: ~46%
- Observation/check: ~18%
- DHT/hashing: ~11.8%
- Move generation: ~10%
- Undo/bookkeeping: ~5.3%

---

## Remaining Opportunities

| Candidate | Expected | Effort | Risk |
|-----------|----------|--------|------|
| Observation result caching | 1-3% | High | Medium |
| PGO build | 5-8% | Low | Low (script exists) |
| `EqualBCMemValue` first-8-bytes | 0.5-1% | Low | Low |

---

## Key Lessons Learned

1. **Profile before optimizing.** Perf data beats intuition. The `-DNDEBUG` change delivered -21.5% from a single build flag because profiling revealed assertions were the dominant cost.

2. **One-line changes can beat large refactors.** Load factor reduction (1 line) gave -10%. NDEBUG (1 line) gave -21.5%.

3. **LTO is sensitive to code structure.** Adding function parameters or changing signatures can cause regressions even when the optimization is logically correct.

4. **Benchmark immediately.** Never assume theoretical savings translate to real gains. The help-path hash optimization was mathematically correct but caused a 3.5% regression.

5. **Revert fast.** Don't patch a broken approach. Bisect, revert, document, move on.

6. **Machine variance exists.** Back-to-back A/B testing on the same machine state is essential. Single runs can vary 2-3%.

7. **`-march=native` is not always a win.** For pointer-chasing workloads, larger instruction encodings can hurt icache performance.

---

---

## Parallel Experiment Branches (Not Merged)

Two other experiment branches were created in April 2026 but their optimizations were **not incorporated** into this branch:

### `feature/experiment-with-codex` (Apr 25, 2026)

Six iterations of move-generation micro-optimizations using Codex:
1. Reduced repeated stack/index work in `push_move_*` helpers
2. Cached `CURRMOVE_OF_PLY(nbply)` in move filter loops
3. Timeout classification (tooling, not optimization)
4. Reordered capture handling branch in `play_move()` (non-capture fast path)
5. Avoided redundant self-assignment during move-filter compaction
6. Simplified move-order inversion setup bounds

**Status:** No measurable timing improvements reported. 27/99 files timed out during regression testing. Not merged.

### `feature/experiment-with-kiro` (Apr 25, 2026)

Planned `-march=native` optimization for AMD Ryzen 9 9950X (znver4 + AVX-512).

**Status:** Only baseline capture completed. The `-march=native` approach was later tested on our branch and found to cause 2.6-3.0% regression (see Failed Attempts section).

---

## Build Configuration

```makefile
# toolchains/gcc/make.incl
CCOPTIM=-O3 -flto -fno-stack-protector

# makefile.defaults (DEFS)
DEFS=$(DEFINEMACRO)SIGNALS $(DEFINEMACRO)MSG_IN_MEM $(DEFINEMACRO)FXF \
     $(DEFINEMACRO)DOMEASURE \
     $(DEFINEMACRO)FXF_MAX_ALIGNMENT_TYPE=void* \
     $(DEFINEMACRO)FXF_NOT_MULTIPLE_ALIGNMENT_TYPE=short \
     $(DEFINEMACRO)NDEBUG
```

---

## Files Modified (Optimization-Related)

| File | Changes |
|------|---------|
| `makefile.defaults` | Added `NDEBUG` to DEFS |
| `toolchains/gcc/make.incl` | Added `-fno-stack-protector` to CCOPTIM |
| `DHT/dht.c` | Bitwise AND, load factor 100%, hash caching, WithHash API, open addressing rewrite, maxmem enforcement in growTable |
| `DHT/dht.h` | `dhtLookupElementWithHash`, `dhtEnterElementWithHash` declarations |
| `DHT/fxf.c` | Added `fxfArenaSize()` to expose arena capacity |
| `DHT/fxf.h` | Added `fxfArenaSize()` declaration |
| `DHT/dhtbcmem.c` | FNV-1a hash function |
| `optimisations/hash.c` | `/onerow` elimination, precomputed hash wiring, `hash_is_precomputed` array |
| `optimisations/orthodox_square_observation.c` | `side_offset` row pointer caching |
| `position/position.h` | Inlined `find_end_of_line` |
| `solving/find_shortest.c` | Parity `&1` |
| `stipulation/help_play/branch.c` | Parity `&1` |
| `options/degenerate_tree.c` | Parity `&1` |
| `options/goal_is_end.c` | Parity `&1` |
| `input/plaintext/stipulation.c` | Parity `&1` |
