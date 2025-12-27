#ifndef CLOCK_CONFIG_H
#define CLOCK_CONFIG_H

#include <Preferences.h>

/** 
 * Clock connection's modes
 */
enum wireless_modes
{
  HOTSPOT,
  EXT_CONN
};

/**
 * Clock animation's modes
 * See docs/CHOREOGRAPHIES.md for detailed descriptions
 */
enum clock_modes
{
  LAZY,        // Direct transition, shortest path
  FUN,         // Clockwise rotation to time
  WAVES,       // Horizontal lines then cascade
  SPINNING,    // 360° sync rotation then time
  SQUARES,     // Diamond pattern then time
  SYMMETRICAL, // Mirror effect left/right
  WIND,        // Organic wave movement
  CASCADE,     // Top-to-bottom waterfall
  FIREWORK,    // Center explosion outward
  OBLIQUES,    // Diagonal lines rotation
  RIPPLE,      // Concentric rings from center
  BREATHE,     // Organic expansion/contraction
  RAIN,        // Vertical falling pattern
  HEARTBEAT,   // Pulsing heart rhythm
  DANCE,       // Random 2-4 shapes chained, then time
  OFF          // All hands to 6:00, drivers disabled
};

/**
 * Load configuration from the EEPROM
 */
void begin_config();

/**
 * Clear EEPROM configuration
 */
void clear_config();

/**
 * Closes the preferencies object
 */
void end_config();

/**
 * Get current clock mode
 */
int get_clock_mode();

/**
 * Gets current sleep time at a given day and hour
 * @param day   day of the week
 * @param hour  hour of the day
 */
bool get_sleep_time(int day, int hour);

/**
 * Gets current connection mode
 */
int get_connection_mode();

/**
 * Gets current time zone based on UTC offset
 */
int get_timezone();

/**
 * Gets current SSID
 */
char *get_ssid();

/**
 * Gets current password
 */
char *get_password();

/**
 * Sets clock mode
 * @param value   mode value of type clock_modes
 */
void set_clock_mode(int value);

/**
 *  Sets current sleep time at a given day and hour
 * @param day   day of the week
 * @param hour  hour of the day
 * @param value true if clock is  disabled, false otherwise
 */
void set_sleep_time(int day, int hour, bool value);

/**
 *  Saves sleep time array on EEPROM
 */
void save_sleep_time();

/**
 *  Sets connection mode
 * @param value   mode value of type wireless_modes
 */
void set_connection_mode(int value);

/**
 *  Sets the time zone
 * @param value   time zone based on UTC offset
 */
void set_timezone(int value);

/**
 *  Sets SSID value
 * @param value   SSID string
 */
void set_ssid(const char *value);

/**
 *  Sets password value
 * @param value   password string
 */
void set_password(const char *value);

/**
 * Gets choreography auto-play mode
 * @return mode value (0=OFF, 1=MANUAL, 2=AUTO, 3=RANDOM)
 */
int get_choreo_mode();

/**
 * Sets choreography auto-play mode
 * @param value   mode value (0=OFF, 1=MANUAL, 2=AUTO, 3=RANDOM)
 */
void set_choreo_mode(int value);

/**
 * Gets choreography enabled mask (bitmask for up to 16 choreographies)
 * @return bitmask where bit N = choreography N enabled
 */
uint16_t get_choreo_enabled_mask();

/**
 * Sets choreography enabled mask
 * @param value   bitmask where bit N = choreography N enabled
 */
void set_choreo_enabled_mask(uint16_t value);

#endif