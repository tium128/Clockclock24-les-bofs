#ifndef BOARD_H
#define BOARD_H

#include "clock_accel_stepper.h"
#include "board_config.h"
#include "clock_state.h"

#define INIT_HANDS_ANGLE 270

/**
 * Initializes all motor objects and get the I2C address
*/
void board_begin();

/**
 * Needs to be called on the main loop to move steppers
*/
void board_loop();

/**
 * Gets the current I2C address set on the board
 * @return address
*/
uint8_t get_i2c_address();

/**
 * Gets the current clock state
 * @param index     clock index (0 <= index =< 3)
 * @return true if clock index is running, false otherwise
*/
bool clock_is_running(int index);

/**
 * Set the clock state by running motors
 * @param index     clock index (0 <= index =< 3)
 * @param state     clock state
*/
void set_clock(int index, t_clock state);

/**
 * Adjust hour hand
 * @param index     clock index (0 <= index =< 3)
 * @param amount    angle (< 0 clockwise, > 0 counterclockwise)
*/
void adjust_h_hand(int index, signed char amount);

/**
 * Adjust minute hand
 * @param index     clock index (0 <= index =< 3)
 * @param amount    angle (< 0 clockwise, > 0 counterclockwise)
*/
void adjust_m_hand(int index, signed char amount);

/**
 * Enable or disable stepper drivers (TMC_ENN pin)
 * @param enabled   true = drivers enabled (LOW), false = request disable (deferred until motors stop)
*/
void set_drivers_enabled(bool enabled);

/**
 * Check if all motors have stopped
 * @return true if all motors have distanceToGo() == 0
*/
bool all_motors_stopped();

/**
 * Process pending driver disable (call in loop)
 * Disables drivers only when all motors have stopped
*/
void process_pending_disable();

#endif