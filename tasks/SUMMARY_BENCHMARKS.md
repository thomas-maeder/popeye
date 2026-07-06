# Benchmark Summary — Popeye DHT Optimization

## Overview

This document consolidates benchmark results across multiple runs (Jun 14–28, 2026) comparing the **Original (chained DHT)** against the **Open Addressing (OA) variant** enabled via `-DDHT_OPEN_ADDRESSING`.

## Test Environment
- Ubuntu "Noble" - 5.15 microsoft-standard-WSL2 x86_64, GCC `-O3 -flto -fno-stack-protector -DNDEBUG`
- AMD Ryzen 9 9950X @ 5600 Mhz
- DDR5 2x 32GB @ 6000 Mhz

## Configuration:
- Commit: `52effba9d`
- Default build (original chained DHT): `make -f makefile.unx`
- Optimized open-addressing DHT: `make -f makefile.unx DEFS="$(DEFS) -DDHT_OPEN_ADDRESSING"`

---

## 1. 81-File General Benchmark (3 runs averaged)

Tests all problem types: directmates, helpmates, proofgames, series movers, fairy conditions.

| Date | Commit | Original (avg) | OA (avg) | Improvement |
|------|--------|---------------|----------|-------------|
| Jun 14 | `552403c5c` | 165.7s | 142.2s | -14.2% |
| Jun 16 | `552403c5c` | 173.8s | 141.9s | -18.4% |
| Jun 23 | `52effba9d` | 173.8s* | 147.6s | -15.1% |
| Jun 28 | `52effba9d` | 173.8s* | 143.8s | -17.3% |

*Original unchanged across runs; reused Jun 16 baseline.*

**Conclusion:** OA variant is consistently **15–18% faster** on the general benchmark.

---

## 2. helpdirectmate.inp — CPU-bound (`-maxmem 4G`)

Pure computation benchmark — low hash pressure, tests solver dispatch overhead.

| Date | Original | OA | Improvement |
|------|----------|-----|-------------|
| Jun 14 | 200s | 159s | -20.5% |
| Jun 16 | 214s | 170s | -20.6% |
| Jun 23 | 214s* | 170s | -20.6% |
| Jun 28 | 214s* | 157s | -26.6% |

**Conclusion:** OA variant is consistently **~20% faster** on CPU-bound helpmates.

---

## 3. EXAMPLES/hypervolage.inp — Memory-intensive (`-maxmem` sweep)

Hypervolage h#4.5 — exercises hash table heavily, triggers compression at lower budgets.

### 3a. Timing comparison (all runs)

| maxmem | Original | OA Jun14 | OA Jun16 | OA Jun23 | OA Jun28 | OA avg |
|--------|----------|----------|----------|----------|----------|--------|
| 1G | >900s | 383s | — | 418s | 395s | ~399s |
| 2G | >900s | 272s | — | 261s | 280s | ~271s |
| 4G | >900s | 231s | 261s | 237s | 270s | ~250s |
| 8G | 654–861s | 234s | 289s | 252s | 275s | ~263s |
| 10G | 583–785s | 336s | 390s | 358s | 375s | ~365s |
| 12G | 574–800s | 234s | 266s | 247s | 281s | ~257s |
| 16G | 571–795s | 228s | 242s | 250s | 280s | ~250s |
| 20G | 578–756s | 215s | 221s | 230s | 265s | ~233s |

### 3b. Memory usage (Jun 28, with monitoring)

| maxmem | Orig Time | Orig RSS | OA Time | OA RSS | Speedup |
|--------|-----------|----------|---------|--------|---------|
| 1G | >900s | — | 395s | 817MB (78%) | ∞ |
| 2G | >900s | — | 280s | 1465MB (70%) | ∞ |
| 4G | >900s | — | 270s | 2846MB (69%) | ∞ |
| 8G | 861s | 8195MB (99%) | 275s | 5536MB (67%) | **3.1×** |
| 10G | 785s | 8763MB (85%) | 375s | 10243MB (99%) | **2.1×** |
| 12G | 800s | 8763MB (71%) | 281s | 10543MB (85%) | **2.8×** |
| 16G | 795s | 8763MB (53%) | 280s | 10543MB (64%) | **2.8×** |
| 20G | 756s | 8763MB (42%) | 265s | 17958MB (87%) | **2.9×** |

### 3c. Key observations

1. **Original cannot solve at ≤4G** (times out at 15 min)
2. **OA solves at every memory level** including 1G (~6:35)
3. **U-curve effect** in OA: 10G is consistently the slowest (~365s avg) due to chronic compression — enough memory to keep hashing but not enough to avoid repeated eviction
4. **Original RSS caps at ~8.7GB** regardless of budget — the problem generates ~8.7GB of hash data total
5. **OA uses more RSS at large budgets** (dead-table overhead from doublings) but solves 2–3× faster
6. **VSZ is identical** for both variants at same budget (fixed arena allocation)

---

## Performance Summary

| Metric | Original | OA | Improvement |
|--------|----------|-----|-------------|
| General benchmark (81 files) | ~174s | ~142s | **-18%** |
| CPU-bound helpmate | ~207s | ~165s | **-20%** |
| Memory-intensive (4G) | >900s | ~250s | **>3.6×** |
| Memory-intensive (20G) | ~667s | ~233s | **2.9×** |

---

## Variance & Reliability

- Run-to-run variance: ±15% (WSL2 system load)
- The OA advantage is consistent across all 4 measurement sessions
- No regressions observed across commits `552403c5c` → `52effba9d` (opt 23–26)
