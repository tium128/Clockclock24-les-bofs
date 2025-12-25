# ClockClock24 Choreographies Documentation

This document describes all available choreographies for the ClockClock24 replica.
It serves as a reference for future development and context recovery.

## Clock Matrix Layout

The clock consists of 24 individual clocks arranged in an 8x3 matrix (8 columns, 3 rows).
Each digit uses 2 columns (6 clocks per digit), for a total of 4 digits.

```
        Digit 0       Digit 1       Digit 2       Digit 3
      Col0  Col1    Col2  Col3    Col4  Col5    Col6  Col7
     ┌────┬────┐   ┌────┬────┐   ┌────┬────┐   ┌────┬────┐
Row0 │ 0  │ 3  │   │ 6  │ 9  │   │ 12 │ 15 │   │ 18 │ 21 │
     ├────┼────┤   ├────┼────┤   ├────┼────┤   ├────┼────┤
Row1 │ 1  │ 4  │   │ 7  │ 10 │   │ 13 │ 16 │   │ 19 │ 22 │
     ├────┼────┤   ├────┼────┤   ├────┼────┤   ├────┼────┤
Row2 │ 2  │ 5  │   │ 8  │ 11 │   │ 14 │ 17 │   │ 20 │ 23 │
     └────┴────┘   └────┴────┘   └────┴────┘   └────┴────┘

Half-digit index: 0=col0, 1=col1, 2=col2, ... 7=col7
```

## Angle Reference

```
         0° (360°)
            ↑
            │
   270° ←───┼───→ 90°
            │
            ↓
          180°
```

- **0° / 360°** : 12 o'clock (up)
- **90°** : 3 o'clock (right)
- **180°** : 6 o'clock (down)
- **270°** : 9 o'clock (left)
- **225°** : Deactivated position (both hands hidden at 7:30)
- **45°** : Diagonal top-right
- **135°** : Diagonal bottom-right
- **315°** : Diagonal top-left

## Data Structure

```cpp
// Single clock state (sent over I2C)
typedef struct clock_state {
    uint16_t angle_h, angle_m;    // Target angles (0-360°)
    uint16_t speed_h, speed_m;    // Motor speed (steps/sec)
    uint16_t accel_h, accel_m;    // Acceleration (steps/sec²)
    uint8_t mode_h, mode_m;       // Direction mode
    signed char adjust_h, adjust_m; // Calibration offset
} t_clock;

// Half-digit = 3 clocks (one column)
typedef struct half_digit {
    t_clock clocks[3];
    uint32_t change_counter[3];
} t_half_digit;

// Digit definition (lite version for storage)
typedef struct {
    t_half_digitl halfs[2];  // Left and right columns
} t_digit;
```

## Direction Modes

- `MIN_DISTANCE` : Shortest path to target
- `CLOCKWISE` : Always rotate clockwise
- `COUNTERCLOCKWISE` : Always rotate counter-clockwise
- `CLOCKWISE2` : Clockwise with 360° minimum rotation
- `ADJUST_HAND` : Manual calibration mode

## Speed/Acceleration Guidelines

| Animation Type | Speed | Acceleration | Notes |
|---------------|-------|--------------|-------|
| Slow/Lazy | 800-1000 | 400-500 | Smooth, elegant |
| Normal | 1000-1500 | 500-800 | Balanced |
| Fast/Dynamic | 1500-2500 | 800-1200 | Energetic |
| Very Fast | 2500-4000 | 1000-2000 | Dramatic |

---

## Choreographies

### 1. LAZY (Original)

**Description**: Direct transition to time display using shortest path.

**Sequence**:
1. Calculate target time angles
2. Send all clocks to target (MIN_DISTANCE mode)

**Parameters**:
- Speed: 1000
- Acceleration: 500
- Direction: MIN_DISTANCE

**Visual**: Smooth, minimal movement. Each hand takes the shortest route.

---

### 2. FUN (Original)

**Description**: All hands rotate clockwise with forced full rotation.

**Sequence**:
1. Calculate target time angles
2. Send all clocks to target (CLOCKWISE2 mode)

**Parameters**:
- Speed: 1200
- Acceleration: 600
- Direction: CLOCKWISE2

**Visual**: Energetic, all hands spin in same direction before settling.

---

### 3. WAVES (Original)

**Description**: Horizontal lines form, then cascade left-to-right into time.

**Sequence**:
1. All clocks → horizontal lines (h=270°, m=90°) [d_IIII]
2. Wait 8 seconds
3. Cascade: half-digits 0→7 transition to time with 400ms delay each

**Parameters**:
- Phase 1: Speed 1200, Accel 600, MIN_DISTANCE
- Phase 2: Speed 1000, Accel 500, CLOCKWISE2

**Visual**:
```
Phase 1 (Lines):          Phase 2 (Cascade):
←→ ←→ ←→ ←→ ←→ ←→ ←→ ←→   [1][2]:[3][4] forming left to right
←→ ←→ ←→ ←→ ←→ ←→ ←→ ←→   with wave effect
←→ ←→ ←→ ←→ ←→ ←→ ←→ ←→
```

---

### 4. SPINNING (New)

**Description**: All hands rotate 360° in sync, then transition to time.

**Sequence**:
1. All clocks → 0° (all pointing up)
2. Wait for completion
3. All clocks → 180° (all pointing down) via CLOCKWISE
4. Wait for completion
5. All clocks → 360° (back to up) via CLOCKWISE
6. Transition to time display

**Parameters**:
- Phase 1-3: Speed 1500, Accel 800, CLOCKWISE
- Final: Speed 1000, Accel 500, MIN_DISTANCE

**Visual**:
```
Phase 1:    Phase 2:    Phase 3:    Final:
↑↑ ↑↑ ...   ↓↓ ↓↓ ...   ↑↑ ↑↑ ...   Time display
All sync    All sync    All sync
```

**Shape definition** (d_spin_up):
```cpp
// All hands at 0° (pointing up)
const t_digit digit_spin_up = {
  0, 0,    // clock 0: h=0°, m=0°
  0, 0,    // clock 1
  0, 0,    // clock 2
  0, 0,    // clock 3
  0, 0,    // clock 4
  0, 0     // clock 5
};
```

---

### 5. SQUARES (New)

**Description**: Forms square/diamond patterns before showing time.

**Sequence**:
1. All clocks → square pattern (corners pointing to form squares)
2. Wait 3 seconds
3. Transition to time display

**Parameters**:
- Phase 1: Speed 1200, Accel 600, MIN_DISTANCE
- Final: Speed 1000, Accel 500, MIN_DISTANCE

**Visual**:
```
Squares pattern:
↘↙ ↘↙ ↘↙ ↘↙    Top row: both hands 135° (bottom-right corner)
↗↙ ↘↖ ↗↙ ↘↖    Middle: alternating X patterns
↗↖ ↗↖ ↗↖ ↗↖    Bottom row: both hands 315° (top-left corner)
```

**Shape definition** (d_squares):
```cpp
const t_digit digit_squares_left = {
  225, 225,   // top-left: ↙↙ (deactivated look)
  135, 45,    // middle: ↘↗ (X pattern)
  315, 315,   // bottom: ↖↖
  225, 225,
  135, 45,
  315, 315
};
```

---

### 6. SYMMETRICAL (New)

**Description**: Mirror effect - left half and right half move symmetrically.

**Sequence**:
1. Left half (digits 0-1) → all pointing left (270°)
2. Right half (digits 2-3) → all pointing right (90°)
3. Wait 2 seconds
4. Both halves converge to center (pointing inward)
5. Wait 2 seconds
6. Transition to time

**Parameters**:
- All phases: Speed 1000, Accel 500

**Visual**:
```
Phase 1:              Phase 2:              Final:
←← ←← →→ →→          →→ →→ ←← ←←          Time
←← ←← →→ →→    →     →→ →→ ←← ←←    →     Display
←← ←← →→ →→          →→ →→ ←← ←←
```

**Shape definitions**:
```cpp
// Left side pointing left
const t_digit digit_sym_left = {
  270, 270, 270, 270, 270, 270,
  270, 270, 270, 270, 270, 270
};

// Right side pointing right
const t_digit digit_sym_right = {
  90, 90, 90, 90, 90, 90,
  90, 90, 90, 90, 90, 90
};
```

---

### 7. WIND (New)

**Description**: Wavy, organic movement like wind blowing through the clocks.

**Sequence**:
1. Create wave pattern with progressive angles across columns
2. Wave oscillates (angles shift)
3. Transition to time

**Parameters**:
- Speed: 800 (slower for organic feel)
- Acceleration: 400

**Visual**:
```
Wind wave pattern (angles vary by column):
Col0: 80°   Col2: 90°   Col4: 100°  Col6: 90°
Col1: 85°   Col3: 95°   Col5: 95°   Col7: 85°

Creates a subtle S-curve across the display
```

**Shape definition** (d_wind):
```cpp
// Progressive angles creating wave effect
const t_digit digit_wind_1 = {
  95, 95,    // slightly right of horizontal
  90, 90,    // horizontal
  85, 85,    // slightly left of horizontal
  100, 100,
  90, 90,
  80, 80
};
```

---

### 8. CASCADE (New)

**Description**: Top-to-bottom waterfall effect, row by row.

**Sequence**:
1. Row 0 (top) → time positions
2. Wait 500ms
3. Row 1 (middle) → time positions
4. Wait 500ms
5. Row 2 (bottom) → time positions

**Parameters**:
- Speed: 1200, Accel: 600
- Direction: CLOCKWISE

**Implementation Note**: Requires sending individual clock commands by row,
not by half-digit. Use `adjust_hands()` or custom row-based function.

**Visual**:
```
Step 1:     Step 2:     Step 3:
[TIME]      [TIME]      [TIME]
  ↓↓          [TIME]    [TIME]
  ↓↓            ↓↓      [TIME]
```

---

### 9. FIREWORK (New)

**Description**: Explosion from center outward, like a firework.

**Sequence**:
1. All hands → center point (all at 270°, hidden/neutral)
2. Wait 1 second
3. Explode outward:
   - Center clocks (columns 3-4) move first
   - Then columns 2,5
   - Then columns 1,6
   - Finally columns 0,7
4. Each "ring" points outward from center
5. Transition to time

**Parameters**:
- Explosion: Speed 2000, Accel 1000, CLOCKWISE
- Final: Speed 1000, Accel 500

**Visual**:
```
Before:           Explosion:         After:
←← ←← ←← ←←      ←← ←↗ ↖→ →→       Radial pattern
←← ←← ←← ←←  →   ←← →→ ←← →→   →   then time
←← ←← ←← ←←      ←← ←↘ ↙→ →→
```

**Center explosion angles**:
```cpp
// Clocks point away from center
// Center-left (col 3): point left (270°)
// Center-right (col 4): point right (90°)
// With diagonal variations for rows
```

---

### 10. DANCE (New)

**Description**: Random mode that chains 2-4 random shapes before displaying time. Mimics the original ClockClock24 behavior where it "dances" between geometric forms.

**Sequence**:
1. Randomly select 2 to 4 shapes from the available pool
2. Display each shape with variable delay (2-4 seconds)
3. Avoid repeating the same shape consecutively
4. Finally transition to time display

**Parameters**:
- Speed: 1200, Accel: 600
- Direction: MIN_DISTANCE

**Shape Pool** (26 patterns):
- Spin: up, down
- Geometric: squares, IIII (lines), fun (diagonals)
- Wind: phases 1-3
- Firework: explosion pattern
- Obliques: 4 directions (br, bl, tr, tl)
- Ripple: in, out
- Breathe: expand, contract, neutral
- Rain: phases 1-3, splash
- Heartbeat: systole, diastole, peak

---

## Additional Shape Patterns (for DANCE mode)

### OBLIQUES

**Description**: Parallel diagonal lines across all clocks.

**Visual**:
```
d_obliques_br (135°):    d_obliques_tl (315°):
↘↘ ↘↘ ↘↘ ↘↘             ↖↖ ↖↖ ↖↖ ↖↖
↘↘ ↘↘ ↘↘ ↘↘             ↖↖ ↖↖ ↖↖ ↖↖
↘↘ ↘↘ ↘↘ ↘↘             ↖↖ ↖↖ ↖↖ ↖↖
```

---

### RIPPLE

**Description**: Concentric rings effect, like a drop in water.

**Sequence**:
1. `d_ripple_out`: Hands point outward in rings from center
2. `d_ripple_in`: Hands point inward toward center

**Visual**:
```
d_ripple_out:            d_ripple_in:
↖↗ ↖↗ ↖↗ ↗↖             ↗↗ ↗↗ ↖↖ ↖↖
←→ ←→ ←→ →←             →→ →→ ←← ←←
↙↘ ↙↘ ↙↘ ↘↙             ↘↘ ↘↘ ↙↙ ↙↙
```

---

### BREATHE

**Description**: Organic expansion/contraction from center, like breathing.

**Sequence**:
1. `d_breathe_expand`: Hands spread outward from center
2. `d_breathe_contract`: Hands converge toward center
3. `d_breathe_neutral`: All horizontal (rest position)

**Visual**:
```
Expand:        Contract:      Neutral:
↖↖ ↖↖ ↗↗ ↗↗   ↗↗ ↗↗ ↖↖ ↖↖    ←→ ←→ ←→ ←→
←← ←← →→ →→   →→ →→ ←← ←←    ←→ ←→ ←→ ←→
↙↙ ↙↙ ↘↘ ↘↘   ↘↘ ↘↘ ↙↙ ↙↙    ←→ ←→ ←→ ←→
```

---

### RAIN

**Description**: Vertical falling pattern with staggered angles.

**Sequence**:
1. `d_rain_1/2/3`: Progressive phases of falling drops
2. `d_rain_splash`: Splash effect at bottom

**Visual**:
```
Rain phase (180° base with ±20° variations):
↓↓ ↙↘ ↓↓ ↘↙
↘↙ ↓↓ ↙↘ ↓↓
↓↓ ↘↙ ↓↓ ↙↘

Splash (bottom row spreads):
↓↓ ↓↓ ↓↓ ↓↓
↓↓ ↓↓ ↓↓ ↓↓
↙↘ ↙↘ ↙↘ ↙↘
```

---

### HEARTBEAT

**Description**: Pulsing heart rhythm - systole (contract) and diastole (expand).

**Sequence**:
1. `d_heart_systole`: All hands point inward (contracted)
2. `d_heart_diastole`: All hands spread outward (relaxed)
3. `d_heart_peak`: Dramatic outward burst (peak of beat)

**Visual**:
```
Systole (contract):      Diastole (expand):       Peak (burst):
↗↗ ↗↗ ↖↖ ↖↖             ↖↖ ↖↖ ↗↗ ↗↗             ↖↖ ↖↖ ↗↗ ↗↗
→→ →→ ←← ←←             ←← ←← →→ →→             ←← ←← →→ →→
↘↘ ↘↘ ↙↙ ↙↙             ↙↙ ↙↙ ↘↘ ↘↘             ↙↙ ↙↙ ↘↘ ↘↘
```

---

## Implementation Checklist

### Choreography Modes (selectable via web interface)
- [x] LAZY - Direct transition, shortest path
- [x] FUN - Clockwise rotation to time
- [x] WAVES - Horizontal lines then cascade
- [x] SPINNING - 360° sync rotation then time
- [x] SQUARES - Diamond pattern then time
- [x] SYMMETRICAL - Mirror effect left/right
- [x] WIND - Organic wave movement
- [x] CASCADE - Top-to-bottom waterfall
- [x] FIREWORK - Center explosion outward
- [x] DANCE - Random 2-4 shapes chained
- [x] OFF - All hands to 6:00, drivers disabled

### Shape Patterns (used by DANCE mode)
- [x] d_spin_up, d_spin_down, d_spin_right
- [x] d_squares
- [x] digit_sym_left, digit_sym_right
- [x] d_wind_1, d_wind_2, d_wind_3
- [x] d_firework
- [x] d_obliques_br, d_obliques_bl, d_obliques_tr, d_obliques_tl
- [x] d_ripple_out, d_ripple_in
- [x] d_breathe_expand, d_breathe_contract, d_breathe_neutral
- [x] d_rain_1, d_rain_2, d_rain_3, d_rain_splash
- [x] d_heart_systole, d_heart_diastole, d_heart_peak

## Files Modified

1. **digit.h** - All shape constants defined
2. **main.cpp** - All choreography functions implemented + dance_shapes array (26 patterns)
3. **clock_config.h** - All enum values added
4. **web_page.h** - All mode buttons in web interface

## Reference

Based on analysis of:
- Original ClockClock24 replica by Vallasc
- https://github.com/ArnaudSpanneut/ClockClock24 (web simulation)
- Humans Since 1982 original design
