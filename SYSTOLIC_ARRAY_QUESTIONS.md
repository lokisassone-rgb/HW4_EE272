# Systolic Array Pre-Implementation Questions

## Configuration Parameters Used
For a 16×16 systolic array:
- `ARRAY_DIMENSION = 16` (both OC0 and IC0)
- Example tile: OX0=14, OY0=14, FX=7, FY=7, STRIDE=1/2, IC1=varies, OC1=varies

## Answers

### 1. How many cycles does the systolic array run to produce a valid output tile?

**Answer: OX0 * OY0 * IC1 * FY * FX + ARRAY_WIDTH + ARRAY_HEIGHT - 1 (for one OC1 pass)**

**Explanation (based on review session slide and conv_controller):**

From the **Output Tiling slide**: Each output tile contains **OC0 × OX0 × OY0** entries.

**Key insight from loop structure:**
- **OC0 and IC0** are unrolled in hardware (parallelized across the systolic array)
- **OX0, OY0** are the output spatial tile dimensions
- **IC1, FY, FX** are the accumulation dimensions (must iterate sequentially)

**To produce one output tile (OC0 × OX0 × OY0):**
1. **Computation cycles**: OX0 × OY0 × IC1 × FY × FX
   - For each of the OX0 × OY0 output positions
   - Accumulate across IC1 input channel groups
   - Accumulate across FY × FX filter positions
   - OC0 outputs computed in parallel per cycle (unrolled)

2. **Pipeline overhead**: ARRAY_WIDTH + ARRAY_HEIGHT - 1
   - Time to fill the systolic array pipeline and flush final outputs

**From conv_controller**: The array is enabled for exactly this duration, confirmed by `systolic_array_en` spanning from cycle 2 to cycle `IC1_FY_FX_OY0_OX0 + ARRAY_WIDTH + ARRAY_HEIGHT`.

For a 16×16 array with OX0=14, OY0=14, IC1=3, FY=7, FX=7:
= 14×14×3×7×7 + 16 + 16 - 1 = 40,614 + 31 = **40,645 cycles**

**Note:** This produces one tile per OC1 iteration. The complete layer requires OC1 such tiles.

---

### 2. When will the first valid output pixel be output to the output skew registers?

**Answer (per controller): At cycle (2 * ARRAY_HEIGHT + 1) for the first partial sum**

**Explanation (matches conv_controller):**
- The controller asserts `ofmap_wen` when `loop_counter_r >= 2*ARRAY_HEIGHT + 1`.
- This is the first time a **partial sum** is written into the output skew path.

**Note from TA:** Either partial sum or full output pixel is acceptable:
- **First partial sum**: 2 * ARRAY_HEIGHT + 1
- **First full output pixel**: Approximately 2 * ARRAY_HEIGHT + 1 + (FX * FY * IC / IC0)
  where IC / IC0 = IC1 (requires FX * FY * IC1 accumulation passes to complete one output pixel)

For a 16×16 array:
- First partial sum: 2*16 + 1 = **33 cycles**
- First full pixel (with FX=7, FY=7, IC1=3): ≈ 33 + 7*7*3 = 33 + 147 = **~180 cycles**

---

### 3. How many weight lines (OC0 weights) will be loaded during the tile computation?

**Answer: IC0 * OC0 * FX * FY * IC1 entries per weight tile × OC1 tiles total**

**More precisely:**

**Each weight tile (per OC1 group) contains:** IC0 * OC0 * FX * FY * IC1 entries
- **IC0 × OC0**: Packing dimension (IC0-wide packed into OC0-wide output)
- **FX × FY**: All filter spatial positions  
- **IC1**: All input channel groups within one OC1 tier

**Total weight entries across full computation:** 
= (IC0 * OC0 * FX * FY * IC1) × OC1
= **IC0 * OC0 * FX * FY * IC1 * OC1** weight entries total

In terms of OC0-wide bus transactions (what "weight lines" typically means):
= IC0 * FX * FY * IC1 * OC1 transactions (each carrying OC0 weights)

**Matches the review session slide:** The slide labeled “Weight Tiling” explicitly shows the tile size as **IC0 * OC0 * FX * FY * IC1**, which is exactly the per-OC1 weight tile used above.

For resnet_conv2_x: IC0=16, OC0=16, FX=3, FY=3, IC1=1, OC1=32
- Per weight tile: 16×16×3×3×1 = **2,304 entries**  
- Total: 2,304 × 32 = **73,728 weight entries** (or 4,608 OC0-wide transactions)

---

### 4. How many input pixel lines (IC0 pixels) will be loaded during the tile computation?

**Answer (per controller): OX0 * OY0 * IC1 * FY * FX**

**Explanation (matches conv_controller):**
- The controller drives `ifmap_ren` for `IC1_FY_FX_OY0_OX0` cycles.
- Each cycle corresponds to one **IC0-wide input line** into the array.
- Therefore, the number of IC0-wide input lines streamed per tile is
  $OX0 \times OY0 \times IC1 \times FY \times FX$.

For OX0=14, OY0=14, IC1=3, FY=7, FX=7:
= 14*14*3*7*7 = **40,614 IC0-wide lines**

**Note:** This is the controller’s streaming count. The **input tile storage size** from the review slides is
$IX \times IY \times IC0 \times IC1$, where $IX=(OX0-1)\cdot STRIDE + FX$ and $IY=(OY0-1)\cdot STRIDE + FY$.

---

### 5. How many partial sums (OC0 partial sums) will be loaded during the tile computation?

**Answer: OX0 * OY0 * OC1**

**Explanation:**
- Partial sums must be loaded for accumulation across IC1 and filter dimensions
- One partial sum per output pixel per output channel group
- Total output pixels in tile = OX0 * OY0
- Total OC groups = OC1
- Total partials = OX0 * OY0 * OC1

**Matches the review session slide:** The “Output Tiling” slide shows output tile entries as **OC0 * OX0 * OY0**. Since partial sums are carried per OC1 group, the number of OC0-wide lines loaded is OX0 * OY0 * OC1.

For OX0=14, OY0=14, OC1=32:
= 14 * 14 * 32 = **6,272 partial sum lines**

---

### 6. When should we load 0 instead of the output of the accumulation buffer for partial sums into the systolic array?

**Answer: When ic1_idx == 0**

**Explanation:**
- For the first ic1_idx in a convolution, the accumulation buffer doesn't contain valid partial sums
- The PE array must start with zero for the first accumulation pass across IC1 dimension
- Subsequent IC1 iterations read the previous partial result from the accumulation buffer for accumulation
- This ensures correctness: first pass initializes, subsequent passes accumulate

Logic (from evidence in code):
```
if (ic1_idx == 0):
  psum = 0  // First accumulation in this IC1 iteration, initialize
else:
  psum = accumulation_buffer[corresponding_output_pixel]  // Accumulate on previous result
```

**Matches the review session slide:** The “Output Tiling” slide shows each output element reused **FX * FY * IC1** times. That implies the first IC1 pass starts from zero and the remaining IC1 passes accumulate from the buffer.

---

## CORRECTED: Weight Double Buffer Iteration Order

**From WeightDoubleBufferTb.cpp (lines 50-65), the CORRECT order is:**

```cpp
for (int ro = 0; ro < params.OY1; ro++)          // OY1
  for (int co = 0; co < params.OX1; co++)       // OX1
    for(int koo = 0; koo < params.OC1; koo++)   // OC1
      for (int c = 0; c < params.IC1; c++)      // IC1  ← INNER!
        for (int wy = 0; wy < params.FY; wy++)  // FY
          for (int wx = 0; wx < params.FX; wx++) // FX
```

**NOT** `OY1 → OX1 → OC1 → FY → FX → IC1` (as I originally stated)

**CORRECT**: `OY1 → OX1 → OC1 → IC1 → FY → FX`

This is evident in [conv_gold_tiled.cpp](conv_gold_tiled.cpp#L39-L51) which shows the systolic array loop structure:
```cpp
OC1: for (int oc1 = 0; oc1 < OC1; oc1++) {
  IC1: for (int ic1 = 0; ic1 < IC1; ic1++) {  ← IC1 is INSIDE OC1
    FY:  for (int fy = 0; fy < FY; fy++) {
      FX:  for (int fx = 0; fx < FX; fx++) {
```



