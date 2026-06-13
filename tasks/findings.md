# Performance Findings

## U-Shaped Performance Curve with `-maxmem` Budget (Jun 13, 2026)

**Observation:** On `FS10852.memtest` (Hypervolage h#4.5), performance is non-linear with memory budget:

| Budget | Time | Total CPU cycles | Compressions |
|--------|------|-----------------|--------------|
| 4G | 3:53 | 1,280B | Many → hash abandoned |
| 10G | 5:41 | 1,939B | Chronic (worst case) |
| 20G | 3:26 | 1,225B | Zero |

**The curve:** `20G (fastest) < 4G < 10G (slowest)`

### Root cause

The solver has three operational regimes depending on hash table pressure:

**Regime 1: Excess memory (20G)** — Table never fills. Every position is stored once and looked up in O(1). No wasted work. `ConvertBCMemValue` = 191B cycles. **Optimal.**

**Regime 2: Insufficient memory (4G)** — Table fills quickly. After several failed compressions, the hash table is destroyed and recreated. The solver falls back to exploring without hash for subtrees that exceed capacity. This avoids the overhead of encoding positions destined for eviction. `ConvertBCMemValue` = 311B cycles. **Acceptable degradation.**

**Regime 3: Middle ground (10G)** — Enough memory to keep the hash alive indefinitely (never triggers full reset), but not enough to hold ALL positions. The table fills → compresses → re-fills → compresses in a chronic cycle. Positions that were expensively encoded get evicted, only to be re-encoded when encountered again. The solver never gives up hashing (memory isn't exhausted), paying full encoding costs for churned positions. `ConvertBCMemValue` = 460B cycles. **Worst performance.**

### Perf evidence (absolute cycle estimates by function)

| Function | 4G | 10G | 20G |
|----------|-----|------|------|
| ConvertBCMemValue | 311B | **460B (+48%)** | 191B |
| lookupSlot | 76B | **105B (+37%)** | 90B |
| EqualBCMemValue | 99B | **198B (+100%)** | 106B |
| dhtCleanup | visible (0.93%) | visible (0.70%) | absent |

10G does 50% more total work than either 4G or 20G because it's trapped in the chronic compression regime.

### Implications

1. **Users should either give the solver plenty of memory (no compression) or very little (forces brute-force fallback).** An intermediate budget that triggers chronic compression is the worst choice.

2. A potential optimization: detect chronic compression (e.g., >N compressions without significant progress) and proactively abandon hashing for the current subtree — mimicking the 4G behavior at any budget level.

3. The `compresshash` threshold (`targetKeyCount -= targetKeyCount/16`) removes only 6.25% of entries per cycle. This is conservative — more aggressive eviction might reduce the number of compression cycles and the re-encoding overhead.

---

## `dispatch()` Is Already Optimally Compiled (Jun 13, 2026)

**Investigation:** The 596-case switch in `dispatch()` accounts for ~10% of CPU. Considered converting to a computed goto or function pointer table.

**Finding:** GCC `-O3 -flto` already compiles the switch into an indirect jump via a jump table:
```asm
lea    0x0(%r13,%rax,8),%rax   ; table lookup
notrack jmp *%rax               ; indirect jump
```

The 10% overhead breakdown:
- ~22% — jump table lookup + indirect jump (irreducible dispatch cost)
- ~11% — function prologue (push r15, r14, r13 — register saves)
- ~6% — bounds check (`cmp $0x2d4` — nr_slice_types guard)
- ~61% — inlined handler code and recursive calls

**Conclusion:** No manual optimization possible. The cost is inherent to the recursive solver architecture — `dispatch` is called millions of times per solve, each incurring function call overhead. A function pointer table would be equivalent to what GCC already generates.

**Not pursued as an optimization.**
