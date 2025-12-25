#ifndef digit_h
#define digit_h

#include <Arduino.h>
#include "clock_state.h"

/**
 * Digits
 * structure: {
 * h0, m0,
 * h1, m1,
 * ...
 * h5, m5
 * }
*/

const t_digit digit_0 = {
  270, 0,
  270, 90,
  0, 90,
  270, 180,
  270, 90,
  180, 90
};

const t_digit digit_1 = {
  225, 225,
  225, 225,
  225, 225,
  270, 270,
  270, 90,
  90, 90
};

const t_digit digit_2 = {
  0, 0,
  270, 0,
  90, 0,
  180, 270,
  90, 180,
  180, 180
};

const t_digit digit_3 = {
  0, 0,
  0, 0,
  0, 0,
  180, 270,
  180, 90,
  180, 90
};

const t_digit digit_4 = {
  270, 270,
  90, 0,
  225, 225,
  270, 270,
  270, 90,
  90, 90
};

const t_digit digit_5 = {
  270, 0,
  90, 0,
  0, 0,
  180, 180,
  270, 180,
  90, 180
};

const t_digit digit_6 = {
  270, 0,
  270, 90,
  90, 0,
  180, 180,
  270, 180,
  90, 180
};

const t_digit digit_7 = {
  0, 0,
  225, 225,
  225, 225,
  270, 180,
  270, 90,
  90, 90
};

const t_digit digit_8 = {
  270, 0,
  90, 0,
  90, 0,
  270, 180,
  90, 180,
  90, 180
};

const t_digit digit_9 = {
  270, 0,
  0, 90,
  0, 0,
  270, 180,
  270, 90,
  90, 180
};

const t_digit digit_null = {
  270, 270,
  270, 270,
  270, 270,
  270, 270,
  270, 270,
  270, 270
};

const t_digit digit_I = {
  270, 90,
  270, 90,
  270, 90,
  270, 90,
  270, 90,
  270, 90
};

const t_digit digit_fun = {
  225, 45,
  225, 45,
  225, 45,
  225, 45,
  225, 45,
  225, 45,
};

const t_full_clock d_stop = {digit_null, digit_null, digit_null, digit_null};

const t_full_clock d_fun = {digit_fun, digit_fun, digit_fun, digit_fun};

const t_full_clock d_IIII = {digit_I, digit_I, digit_I, digit_I};

// ============================================
// NEW CHOREOGRAPHY SHAPES
// See docs/CHOREOGRAPHIES.md for documentation
// ============================================

// SPINNING: All hands pointing up (0°)
const t_digit digit_spin_up = {
  0, 0,
  0, 0,
  0, 0,
  0, 0,
  0, 0,
  0, 0
};

// SPINNING: All hands pointing down (180°)
const t_digit digit_spin_down = {
  180, 180,
  180, 180,
  180, 180,
  180, 180,
  180, 180,
  180, 180
};

// SPINNING: All hands pointing right (90°)
const t_digit digit_spin_right = {
  90, 90,
  90, 90,
  90, 90,
  90, 90,
  90, 90,
  90, 90
};

const t_full_clock d_spin_up = {digit_spin_up, digit_spin_up, digit_spin_up, digit_spin_up};
const t_full_clock d_spin_down = {digit_spin_down, digit_spin_down, digit_spin_down, digit_spin_down};
const t_full_clock d_spin_right = {digit_spin_right, digit_spin_right, digit_spin_right, digit_spin_right};

// SQUARES: Diamond/square pattern
// Top row: pointing to bottom-right corner (135°)
// Middle row: X pattern (135° and 45° alternating)
// Bottom row: pointing to top-left corner (315°)
const t_digit digit_squares_a = {
  135, 135,   // top: ↘↘
  135, 45,    // middle: ↘↗ (X)
  315, 315,   // bottom: ↖↖
  135, 135,
  45, 135,    // middle: ↗↘ (X inverted)
  315, 315
};

const t_digit digit_squares_b = {
  225, 225,   // top: ↙↙
  45, 135,    // middle: ↗↘
  45, 45,     // bottom: ↗↗
  225, 225,
  135, 45,    // middle: ↘↗
  45, 45
};

const t_full_clock d_squares = {digit_squares_a, digit_squares_b, digit_squares_a, digit_squares_b};

// SYMMETRICAL: Left side pointing left (270°)
const t_digit digit_sym_left = {
  270, 270,
  270, 270,
  270, 270,
  270, 270,
  270, 270,
  270, 270
};

// SYMMETRICAL: Right side pointing right (90°)
const t_digit digit_sym_right = {
  90, 90,
  90, 90,
  90, 90,
  90, 90,
  90, 90,
  90, 90
};

// SYMMETRICAL: Converge to center - left digits point right
const t_digit digit_sym_converge_right = {
  90, 90,
  90, 90,
  90, 90,
  90, 90,
  90, 90,
  90, 90
};

// SYMMETRICAL: Converge to center - right digits point left
const t_digit digit_sym_converge_left = {
  270, 270,
  270, 270,
  270, 270,
  270, 270,
  270, 270,
  270, 270
};

// WIND: Wave pattern with progressive angles
// Creates an S-curve effect across the display
const t_digit digit_wind_1 = {
  95, 95,     // slightly tilted right
  90, 90,     // horizontal
  85, 85,     // slightly tilted left
  100, 100,   // more tilted right
  90, 90,
  80, 80      // more tilted left
};

const t_digit digit_wind_2 = {
  85, 85,
  90, 90,
  95, 95,
  80, 80,
  90, 90,
  100, 100
};

const t_digit digit_wind_3 = {
  100, 100,
  95, 95,
  90, 90,
  95, 95,
  85, 85,
  85, 85
};

const t_digit digit_wind_4 = {
  80, 80,
  85, 85,
  90, 90,
  85, 85,
  95, 95,
  95, 95
};

const t_full_clock d_wind_1 = {digit_wind_1, digit_wind_2, digit_wind_3, digit_wind_4};
const t_full_clock d_wind_2 = {digit_wind_2, digit_wind_3, digit_wind_4, digit_wind_1};
const t_full_clock d_wind_3 = {digit_wind_3, digit_wind_4, digit_wind_1, digit_wind_2};

// FIREWORK: Center explosion pattern
// Hands point outward from center of display

// Left outer columns (0,1) - point left/outward
const t_digit digit_firework_outer_left = {
  315, 315,   // top: point top-left ↖
  270, 270,   // middle: point left ←
  225, 225,   // bottom: point bottom-left ↙
  315, 315,
  270, 270,
  225, 225
};

// Left inner columns (2,3) - point slightly left
const t_digit digit_firework_inner_left = {
  315, 0,     // top: ↖ and ↑
  270, 270,   // middle: ← ←
  225, 180,   // bottom: ↙ and ↓
  0, 45,
  270, 90,
  180, 135
};

// Right inner columns (4,5) - point slightly right
const t_digit digit_firework_inner_right = {
  0, 45,      // top: ↑ and ↗
  270, 90,    // middle: ← →
  180, 135,   // bottom: ↓ and ↘
  315, 0,
  270, 90,
  225, 180
};

// Right outer columns (6,7) - point right/outward
const t_digit digit_firework_outer_right = {
  45, 45,     // top: point top-right ↗
  90, 90,     // middle: point right →
  135, 135,   // bottom: point bottom-right ↘
  45, 45,
  90, 90,
  135, 135
};

const t_full_clock d_firework = {digit_firework_outer_left, digit_firework_inner_left, digit_firework_inner_right, digit_firework_outer_right};

// CASCADE: Uses d_stop initially, then row-by-row animation
// (Implemented in code, not as static shapes)

// ============================================
// OBLIQUES: Diagonal lines pattern
// All hands form parallel diagonal lines across display
// ============================================

// Obliques pointing bottom-right (135°)
const t_digit digit_obliques_br = {
  135, 135,
  135, 135,
  135, 135,
  135, 135,
  135, 135,
  135, 135
};

// Obliques pointing bottom-left (225°)
const t_digit digit_obliques_bl = {
  225, 225,
  225, 225,
  225, 225,
  225, 225,
  225, 225,
  225, 225
};

// Obliques pointing top-right (45°)
const t_digit digit_obliques_tr = {
  45, 45,
  45, 45,
  45, 45,
  45, 45,
  45, 45,
  45, 45
};

// Obliques pointing top-left (315°)
const t_digit digit_obliques_tl = {
  315, 315,
  315, 315,
  315, 315,
  315, 315,
  315, 315,
  315, 315
};

const t_full_clock d_obliques_br = {digit_obliques_br, digit_obliques_br, digit_obliques_br, digit_obliques_br};
const t_full_clock d_obliques_bl = {digit_obliques_bl, digit_obliques_bl, digit_obliques_bl, digit_obliques_bl};
const t_full_clock d_obliques_tr = {digit_obliques_tr, digit_obliques_tr, digit_obliques_tr, digit_obliques_tr};
const t_full_clock d_obliques_tl = {digit_obliques_tl, digit_obliques_tl, digit_obliques_tl, digit_obliques_tl};

// ============================================
// RIPPLE: Concentric rings from center
// Hands point outward in rings, creating wave effect
// ============================================

// Center ring (columns 3,4) - tight angles pointing out
const t_digit digit_ripple_center = {
  315, 45,    // top: ↖ ↗
  270, 90,    // middle: ← →
  225, 135,   // bottom: ↙ ↘
  315, 45,
  270, 90,
  225, 135
};

// Middle ring (columns 2,5) - slightly more spread
const t_digit digit_ripple_mid = {
  300, 60,    // top
  270, 90,    // middle
  240, 120,   // bottom
  300, 60,
  270, 90,
  240, 120
};

// Outer ring (columns 0,1,6,7) - most spread out
const t_digit digit_ripple_outer = {
  315, 45,    // top
  270, 90,    // middle
  225, 135,   // bottom
  315, 45,
  270, 90,
  225, 135
};

// Ripple collapsed (all pointing to center-ish)
const t_digit digit_ripple_in_left = {
  45, 45,
  90, 90,
  135, 135,
  45, 45,
  90, 90,
  135, 135
};

const t_digit digit_ripple_in_right = {
  315, 315,
  270, 270,
  225, 225,
  315, 315,
  270, 270,
  225, 225
};

const t_full_clock d_ripple_out = {digit_ripple_outer, digit_ripple_mid, digit_ripple_center, digit_ripple_outer};
const t_full_clock d_ripple_in = {digit_ripple_in_left, digit_ripple_in_left, digit_ripple_in_right, digit_ripple_in_right};

// ============================================
// BREATHE: Expansion/contraction from center
// Organic breathing motion
// ============================================

// Breathe expanded - hands spread outward
const t_digit digit_breathe_expand_left = {
  315, 315,   // top: pointing up-left
  270, 270,   // middle: pointing left
  225, 225,   // bottom: pointing down-left
  315, 315,
  270, 270,
  225, 225
};

const t_digit digit_breathe_expand_right = {
  45, 45,     // top: pointing up-right
  90, 90,     // middle: pointing right
  135, 135,   // bottom: pointing down-right
  45, 45,
  90, 90,
  135, 135
};

// Breathe contracted - hands toward center
const t_digit digit_breathe_contract_left = {
  45, 45,     // pointing right
  90, 90,
  135, 135,
  45, 45,
  90, 90,
  135, 135
};

const t_digit digit_breathe_contract_right = {
  315, 315,   // pointing left
  270, 270,
  225, 225,
  315, 315,
  270, 270,
  225, 225
};

// Breathe neutral - all horizontal
const t_digit digit_breathe_neutral = {
  270, 90,
  270, 90,
  270, 90,
  270, 90,
  270, 90,
  270, 90
};

const t_full_clock d_breathe_expand = {digit_breathe_expand_left, digit_breathe_expand_left, digit_breathe_expand_right, digit_breathe_expand_right};
const t_full_clock d_breathe_contract = {digit_breathe_contract_left, digit_breathe_contract_left, digit_breathe_contract_right, digit_breathe_contract_right};
const t_full_clock d_breathe_neutral = {digit_breathe_neutral, digit_breathe_neutral, digit_breathe_neutral, digit_breathe_neutral};

// ============================================
// RAIN: Vertical falling pattern
// Hands point down in staggered pattern
// ============================================

// Rain phase 1 - alternating down angles
const t_digit digit_rain_1 = {
  180, 180,   // straight down
  200, 160,   // slightly angled
  180, 180,
  160, 200,
  180, 180,
  200, 160
};

// Rain phase 2 - shifted
const t_digit digit_rain_2 = {
  160, 200,
  180, 180,
  200, 160,
  180, 180,
  160, 200,
  180, 180
};

// Rain phase 3 - more shift
const t_digit digit_rain_3 = {
  200, 160,
  160, 200,
  180, 180,
  200, 160,
  180, 180,
  160, 200
};

// Rain splash - hands spread at bottom
const t_digit digit_rain_splash = {
  180, 180,
  180, 180,
  225, 135,   // bottom spreads out
  180, 180,
  180, 180,
  225, 135
};

const t_full_clock d_rain_1 = {digit_rain_1, digit_rain_2, digit_rain_3, digit_rain_1};
const t_full_clock d_rain_2 = {digit_rain_2, digit_rain_3, digit_rain_1, digit_rain_2};
const t_full_clock d_rain_3 = {digit_rain_3, digit_rain_1, digit_rain_2, digit_rain_3};
const t_full_clock d_rain_splash = {digit_rain_splash, digit_rain_splash, digit_rain_splash, digit_rain_splash};

// ============================================
// HEARTBEAT: Pulsing heart-like pattern
// Center expands and contracts like a beating heart
// ============================================

// Heartbeat systole (contracted) - hands point inward to center
const t_digit digit_heart_systole_left = {
  45, 45,     // all pointing toward center-right
  90, 90,
  135, 135,
  45, 45,
  90, 90,
  135, 135
};

const t_digit digit_heart_systole_right = {
  315, 315,   // all pointing toward center-left
  270, 270,
  225, 225,
  315, 315,
  270, 270,
  225, 225
};

// Heartbeat diastole (expanded) - hands spread outward
const t_digit digit_heart_diastole_left = {
  315, 315,   // pointing away from center
  270, 270,
  225, 225,
  315, 315,
  270, 270,
  225, 225
};

const t_digit digit_heart_diastole_right = {
  45, 45,     // pointing away from center
  90, 90,
  135, 135,
  45, 45,
  90, 90,
  135, 135
};

// Heartbeat peak - dramatic outward burst
const t_digit digit_heart_peak_left = {
  300, 300,   // strong outward angles
  270, 270,
  240, 240,
  300, 300,
  270, 270,
  240, 240
};

const t_digit digit_heart_peak_right = {
  60, 60,
  90, 90,
  120, 120,
  60, 60,
  90, 90,
  120, 120
};

const t_full_clock d_heart_systole = {digit_heart_systole_left, digit_heart_systole_left, digit_heart_systole_right, digit_heart_systole_right};
const t_full_clock d_heart_diastole = {digit_heart_diastole_left, digit_heart_diastole_left, digit_heart_diastole_right, digit_heart_diastole_right};
const t_full_clock d_heart_peak = {digit_heart_peak_left, digit_heart_peak_left, digit_heart_peak_right, digit_heart_peak_right};

#endif