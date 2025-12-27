#ifndef digit_h
#define digit_h

#include <Arduino.h>
#include "clock_state.h"

/**
 * Digits
 * Clock convention: 0° = 12h, 90° = 3h, 180° = 6h, 270° = 9h
 * structure: {
 * h0, m0,
 * h1, m1,
 * ...
 * h5, m5
 * }
*/

const t_digit digit_0 = {
  180, 90,
  180, 0,
  90, 0,
  180, 270,
  180, 0,
  270, 0
};

const t_digit digit_1 = {
  225, 225,
  225, 225,
  225, 225,
  180, 180,
  180, 0,
  0, 0
};

const t_digit digit_2 = {
  90, 90,
  180, 90,
  0, 90,
  270, 180,
  0, 270,
  270, 270
};

const t_digit digit_3 = {
  90, 90,
  90, 90,
  90, 90,
  270, 180,
  270, 0,
  270, 0
};

const t_digit digit_4 = {
  180, 180,
  0, 90,
  225, 225,
  180, 180,
  180, 0,
  0, 0
};

const t_digit digit_5 = {
  180, 90,
  0, 90,
  90, 90,
  270, 270,
  180, 270,
  0, 270
};

const t_digit digit_6 = {
  180, 90,
  180, 0,
  0, 90,
  270, 270,
  180, 270,
  0, 270
};

const t_digit digit_7 = {
  90, 90,
  225, 225,
  225, 225,
  180, 270,
  180, 0,
  0, 0
};

const t_digit digit_8 = {
  180, 90,
  0, 90,
  0, 90,
  180, 270,
  0, 270,
  0, 270
};

const t_digit digit_9 = {
  180, 90,
  90, 0,
  90, 90,
  180, 270,
  180, 0,
  0, 270
};

const t_digit digit_null = {
  180, 180,
  180, 180,
  180, 180,
  180, 180,
  180, 180,
  180, 180
};

const t_digit digit_I = {
  180, 0,
  180, 0,
  180, 0,
  180, 0,
  180, 0,
  180, 0
};

const t_digit digit_fun = {
  225, 45,
  225, 45,
  225, 45,
  225, 45,
  225, 45,
  225, 45
};

const t_full_clock d_stop = {digit_null, digit_null, digit_null, digit_null};

const t_full_clock d_fun = {digit_fun, digit_fun, digit_fun, digit_fun};

const t_full_clock d_IIII = {digit_I, digit_I, digit_I, digit_I};

#endif
