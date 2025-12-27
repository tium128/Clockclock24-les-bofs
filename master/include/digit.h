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

// SPINNING: Vertical lines pointing up/down (visible |)
const t_digit digit_spin_up = {
  0, 180,    // vertical line |
  0, 180,
  0, 180,
  0, 180,
  0, 180,
  0, 180
};

// SPINNING: Vertical lines pointing down/up (visible |)
const t_digit digit_spin_down = {
  180, 0,    // vertical line | (inverted rotation)
  180, 0,
  180, 0,
  180, 0,
  180, 0,
  180, 0
};

// SPINNING: Horizontal lines (visible -)
const t_digit digit_spin_right = {
  270, 90,   // horizontal line -
  270, 90,
  270, 90,
  270, 90,
  270, 90,
  270, 90
};

const t_full_clock d_spin_up = {digit_spin_up, digit_spin_up, digit_spin_up, digit_spin_up};
const t_full_clock d_spin_down = {digit_spin_down, digit_spin_down, digit_spin_down, digit_spin_down};
const t_full_clock d_spin_right = {digit_spin_right, digit_spin_right, digit_spin_right, digit_spin_right};

// SQUARES: Diamond/square pattern with L-shapes
// Each clock forms an L-shape or corner
const t_digit digit_squares_a = {
  0, 90,      // top: L shape ┐
  270, 180,   // middle: L shape └
  0, 90,      // bottom: L shape ┐
  180, 270,   // top: L shape ┘
  90, 0,      // middle: L shape ┌
  180, 270    // bottom: L shape ┘
};

const t_digit digit_squares_b = {
  180, 270,   // top: L shape ┘
  90, 0,      // middle: L shape ┌
  180, 270,   // bottom: L shape ┘
  0, 90,      // top: L shape ┐
  270, 180,   // middle: L shape └
  0, 90       // bottom: L shape ┐
};

const t_full_clock d_squares = {digit_squares_a, digit_squares_b, digit_squares_a, digit_squares_b};

// SYMMETRICAL: Left side - arrows pointing left (< shape)
const t_digit digit_sym_left = {
  315, 225,   // top: V pointing left <
  270, 270,   // middle: both left (will overlap but that's OK for center line)
  225, 315,   // bottom: V pointing left <
  315, 225,
  270, 270,
  225, 315
};

// SYMMETRICAL: Right side - arrows pointing right (> shape)
const t_digit digit_sym_right = {
  45, 135,    // top: V pointing right >
  90, 90,     // middle: both right
  135, 45,    // bottom: V pointing right >
  45, 135,
  90, 90,
  135, 45
};

// SYMMETRICAL: Converge to center - left digits point right (> shape)
const t_digit digit_sym_converge_right = {
  45, 135,
  90, 90,
  135, 45,
  45, 135,
  90, 90,
  135, 45
};

// SYMMETRICAL: Converge to center - right digits point left (< shape)
const t_digit digit_sym_converge_left = {
  315, 225,
  270, 270,
  225, 315,
  315, 225,
  270, 270,
  225, 315
};

// WIND: Wave pattern with diagonal lines
// h and m at opposite angles create visible diagonal lines
const t_digit digit_wind_1 = {
  315, 135,   // top: diagonal \
  270, 90,    // mid: horizontal -
  225, 45,    // bot: diagonal /
  315, 135,
  270, 90,
  225, 45
};

const t_digit digit_wind_2 = {
  225, 45,    // top: diagonal /
  270, 90,    // mid: horizontal -
  315, 135,   // bot: diagonal \
  225, 45,
  270, 90,
  315, 135
};

const t_digit digit_wind_3 = {
  270, 90,    // all horizontal
  315, 135,
  270, 90,
  270, 90,
  225, 45,
  270, 90
};

const t_digit digit_wind_4 = {
  270, 90,
  225, 45,
  270, 90,
  270, 90,
  315, 135,
  270, 90
};

const t_full_clock d_wind_1 = {digit_wind_1, digit_wind_2, digit_wind_3, digit_wind_4};
const t_full_clock d_wind_2 = {digit_wind_2, digit_wind_3, digit_wind_4, digit_wind_1};
const t_full_clock d_wind_3 = {digit_wind_3, digit_wind_4, digit_wind_1, digit_wind_2};

// FIREWORK: Center explosion pattern
// Hands point outward from center of display

// Left outer columns (0,1) - arrows pointing left/outward
const t_digit digit_firework_outer_left = {
  315, 225,   // top: V pointing left <
  270, 270,   // middle: both left (center line)
  225, 315,   // bottom: V pointing left <
  315, 225,
  270, 270,
  225, 315
};

// Left inner columns (2,3) - mixed arrows (transition)
const t_digit digit_firework_inner_left = {
  315, 45,    // top: diagonal spread
  270, 90,    // middle: horizontal line
  225, 135,   // bottom: diagonal spread
  0, 180,     // top: vertical line
  270, 90,    // middle: horizontal line
  180, 0      // bottom: vertical line
};

// Right inner columns (4,5) - mixed arrows (transition)
const t_digit digit_firework_inner_right = {
  0, 180,     // top: vertical line
  270, 90,    // middle: horizontal line
  180, 0,     // bottom: vertical line
  45, 315,    // top: diagonal spread
  270, 90,    // middle: horizontal line
  135, 225    // bottom: diagonal spread
};

// Right outer columns (6,7) - arrows pointing right/outward
const t_digit digit_firework_outer_right = {
  45, 135,    // top: V pointing right >
  90, 90,     // middle: both right (center line)
  135, 45,    // bottom: V pointing right >
  45, 135,
  90, 90,
  135, 45
};

const t_full_clock d_firework = {digit_firework_outer_left, digit_firework_inner_left, digit_firework_inner_right, digit_firework_outer_right};

// CASCADE: Uses d_stop initially, then row-by-row animation
// (Implemented in code, not as static shapes)

// ============================================
// OBLIQUES: Diagonal lines pattern
// All hands form parallel diagonal lines across display
// ============================================

// Obliques: diagonal line \ (top-left to bottom-right)
const t_digit digit_obliques_br = {
  315, 135,   // h points up-left, m points down-right = diagonal \
  315, 135,
  315, 135,
  315, 135,
  315, 135,
  315, 135
};

// Obliques: diagonal line / (top-right to bottom-left)
const t_digit digit_obliques_bl = {
  225, 45,    // h points down-left, m points up-right = diagonal /
  225, 45,
  225, 45,
  225, 45,
  225, 45,
  225, 45
};

// Obliques: diagonal line / (same as bl, alternative name)
const t_digit digit_obliques_tr = {
  45, 225,    // h points up-right, m points down-left = diagonal /
  45, 225,
  45, 225,
  45, 225,
  45, 225,
  45, 225
};

// Obliques: diagonal line \ (same as br, alternative name)
const t_digit digit_obliques_tl = {
  135, 315,   // h points down-right, m points up-left = diagonal \
  135, 315,
  135, 315,
  135, 315,
  135, 315,
  135, 315
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

// Ripple collapsed (arrows pointing to center)
const t_digit digit_ripple_in_left = {
  45, 135,    // top: V pointing right >
  270, 90,    // middle: horizontal line -
  135, 45,    // bottom: V pointing right >
  45, 135,
  270, 90,
  135, 45
};

const t_digit digit_ripple_in_right = {
  315, 225,   // top: V pointing left <
  270, 90,    // middle: horizontal line -
  225, 315,   // bottom: V pointing left <
  315, 225,
  270, 90,
  225, 315
};

const t_full_clock d_ripple_out = {digit_ripple_outer, digit_ripple_mid, digit_ripple_center, digit_ripple_outer};
const t_full_clock d_ripple_in = {digit_ripple_in_left, digit_ripple_in_left, digit_ripple_in_right, digit_ripple_in_right};

// ============================================
// BREATHE: Expansion/contraction from center
// Organic breathing motion
// ============================================

// Breathe expanded - hands spread outward (V shapes pointing out)
const t_digit digit_breathe_expand_left = {
  315, 225,   // top: V pointing left <
  270, 270,   // middle: both left (center line)
  225, 315,   // bottom: V pointing left <
  315, 225,
  270, 270,
  225, 315
};

const t_digit digit_breathe_expand_right = {
  45, 135,    // top: V pointing right >
  90, 90,     // middle: both right (center line)
  135, 45,    // bottom: V pointing right >
  45, 135,
  90, 90,
  135, 45
};

// Breathe contracted - hands toward center (V shapes pointing in)
const t_digit digit_breathe_contract_left = {
  45, 135,    // V pointing right (toward center)
  90, 90,
  135, 45,
  45, 135,
  90, 90,
  135, 45
};

const t_digit digit_breathe_contract_right = {
  315, 225,   // V pointing left (toward center)
  270, 270,
  225, 315,
  315, 225,
  270, 270,
  225, 315
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

// Rain phase 1 - vertical lines (ready to fall)
const t_digit digit_rain_1 = {
  0, 180,     // vertical line |
  0, 180,
  0, 180,
  0, 180,
  0, 180,
  0, 180
};

// Rain phase 2 - tilted (falling)
const t_digit digit_rain_2 = {
  315, 135,   // diagonal \
  315, 135,
  315, 135,
  315, 135,
  315, 135,
  315, 135
};

// Rain phase 3 - horizontal (landed)
const t_digit digit_rain_3 = {
  270, 90,    // horizontal -
  270, 90,
  270, 90,
  270, 90,
  270, 90,
  270, 90
};

// Rain splash - V shapes at bottom spreading out
const t_digit digit_rain_splash = {
  270, 90,    // top horizontal
  270, 90,    // mid horizontal
  225, 135,   // bottom V spread
  270, 90,
  270, 90,
  315, 45     // bottom V spread (opposite)
};

const t_full_clock d_rain_1 = {digit_rain_1, digit_rain_2, digit_rain_3, digit_rain_1};
const t_full_clock d_rain_2 = {digit_rain_2, digit_rain_3, digit_rain_1, digit_rain_2};
const t_full_clock d_rain_3 = {digit_rain_3, digit_rain_1, digit_rain_2, digit_rain_3};
const t_full_clock d_rain_splash = {digit_rain_splash, digit_rain_splash, digit_rain_splash, digit_rain_splash};

// ============================================
// HEARTBEAT: Pulsing heart-like pattern
// Center expands and contracts like a beating heart
// ============================================

// Heartbeat systole (contracted) - V shapes pointing toward center
const t_digit digit_heart_systole_left = {
  45, 135,    // top: V pointing right >
  270, 90,    // middle: horizontal -
  135, 45,    // bottom: V pointing right >
  45, 135,
  270, 90,
  135, 45
};

const t_digit digit_heart_systole_right = {
  315, 225,   // top: V pointing left <
  270, 90,    // middle: horizontal -
  225, 315,   // bottom: V pointing left <
  315, 225,
  270, 90,
  225, 315
};

// Heartbeat diastole (expanded) - V shapes pointing away from center
const t_digit digit_heart_diastole_left = {
  315, 225,   // top: V pointing left <
  270, 90,    // middle: horizontal -
  225, 315,   // bottom: V pointing left <
  315, 225,
  270, 90,
  225, 315
};

const t_digit digit_heart_diastole_right = {
  45, 135,    // top: V pointing right >
  270, 90,    // middle: horizontal -
  135, 45,    // bottom: V pointing right >
  45, 135,
  270, 90,
  135, 45
};

// Heartbeat peak - dramatic outward burst (bigger V shapes)
const t_digit digit_heart_peak_left = {
  300, 240,   // top: wide V pointing left
  270, 90,    // middle: horizontal
  240, 300,   // bottom: wide V pointing left
  300, 240,
  270, 90,
  240, 300
};

const t_digit digit_heart_peak_right = {
  60, 120,    // top: wide V pointing right
  270, 90,    // middle: horizontal
  120, 60,    // bottom: wide V pointing right
  60, 120,
  270, 90,
  120, 60
};

const t_full_clock d_heart_systole = {digit_heart_systole_left, digit_heart_systole_left, digit_heart_systole_right, digit_heart_systole_right};
const t_full_clock d_heart_diastole = {digit_heart_diastole_left, digit_heart_diastole_left, digit_heart_diastole_right, digit_heart_diastole_right};
const t_full_clock d_heart_peak = {digit_heart_peak_left, digit_heart_peak_left, digit_heart_peak_right, digit_heart_peak_right};

#endif