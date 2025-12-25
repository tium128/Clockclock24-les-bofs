#include <Arduino.h>
#include <Wire.h>
#include <TimeLib.h>
#include <Adafruit_NeoPixel.h>
#define PIN_RGB     21      // GPIO38 comme indiqué
#define NUMPIXELS   1       // 1 LED

#include "i2c.h"
#include "clock_state.h"
#include "clock_manager.h"
#include "digit.h"
#include "wifi_utils.h"
#include "web_server.h"
#include "clock_config.h"
#include "ntp.h"


int last_hour = -1;
int last_minute = -1;
bool is_stopped = false;

/**
 * Sets clock to the current time
*/
void set_time();

/**
 * Sets clock time using lazy animation
*/
void set_lazy();

/**
 * Sets clock time using fun animation
*/
void set_fun();

/**
 * Sets clock time using waves animation
*/
void set_waves();

/**
 * Sets clock time using spinning animation (360° sync rotation)
*/
void set_spinning();

/**
 * Sets clock time using squares animation (diamond pattern)
*/
void set_squares();

/**
 * Sets clock time using symmetrical animation (mirror effect)
*/
void set_symmetrical();

/**
 * Sets clock time using wind animation (organic wave)
*/
void set_wind();

/**
 * Sets clock time using cascade animation (top-to-bottom)
*/
void set_cascade();

/**
 * Sets clock time using firework animation (center explosion)
*/
void set_firework();

/**
 * Sets clock time using dance animation (random shapes chained)
*/
void set_dance();

/**
 * Sets clock time using obliques animation (diagonal lines)
*/
void set_obliques();

/**
 * Sets clock time using ripple animation (concentric rings)
*/
void set_ripple();

/**
 * Sets clock time using breathe animation (expansion/contraction)
*/
void set_breathe();

/**
 * Sets clock time using rain animation (falling pattern)
*/
void set_rain();

/**
 * Sets clock time using heartbeat animation (pulsing)
*/
void set_heartbeat();

/**
 * Sets clock to stop state
*/
void stop();

/**
 * Custom delay to update web clients
 * @param value   time in milliseconds
*/
void _delay(int value);

void setup() {
  Serial.begin(115200);
  Serial.println("\nclockclock24 replica by Vallasc master v1.0");
  delay(3000);
  // Load configuration from EEPROM
  begin_config();

  Wire.begin(9, 8, 100000);
  pixels.begin();            // Initialise la LED
  pixels.setBrightness(10);  // Réduit l’intensité globale (0–255)
  pixels.setPixelColor(0, pixels.Color(255, 0, 0)); // Rouge
  pixels.show();

  if(get_connection_mode() == HOTSPOT)
    wifi_create_AP("ClockClock 24", "clockclock24");
  else if( !wifi_connect(get_ssid(), get_password(), "clockclock24") )
  {
    set_connection_mode(HOTSPOT);
    wifi_create_AP("ClockClock 24", "clockclock24");
  }

  if(get_connection_mode() == EXT_CONN)
  {
    // Initialize NTP
    begin_NTP();
    setSyncProvider(get_NTP_time);
    // Sync every 30 minutes
    setSyncInterval(60 * 30);
  }
  set_wifi_status_led(get_connection_mode());
  // Starts web server
  server_start();
}

void loop() {

  if(get_connection_mode() == HOTSPOT && is_time_changed_browser())
  {
    t_browser_time browser_time = get_browser_time();
    setTime(browser_time.hour, 
      browser_time.minute, 
      browser_time.second, 
      browser_time.day, 
      browser_time.month,  
      browser_time.year);
  }

  if(get_connection_mode() == EXT_CONN && get_timezone() != get_ntp_timezone())
  {
    set_ntp_timezone(get_timezone());
    setSyncProvider(get_NTP_time);
  }

  get_clock_mode() != OFF ? set_time() : stop();

  update_MDNS();
  handle_webclient();
}

void set_time()
{
  int day_week = (weekday() + 5) % 7;
  if(get_sleep_time(day_week, hour()))
    stop();
  else if(hour() != last_hour || minute() != last_minute)
  {
    // Re-enable drivers if coming from stopped state
    if(is_stopped)
    {
      set_all_drivers_enabled(true);
      delay(500); // Wait for all drivers to be fully enabled before sending positions
    }
    is_stopped = false;
    last_hour = hour();
    last_minute = minute();
    switch(get_clock_mode())
    {
      case LAZY:
        set_lazy();
        break;
      case FUN:
        set_fun();
        break;
      case WAVES:
        set_waves();
        break;
      case SPINNING:
        set_spinning();
        break;
      case SQUARES:
        set_squares();
        break;
      case SYMMETRICAL:
        set_symmetrical();
        break;
      case WIND:
        set_wind();
        break;
      case CASCADE:
        set_cascade();
        break;
      case FIREWORK:
        set_firework();
        break;
      case OBLIQUES:
        set_obliques();
        break;
      case RIPPLE:
        set_ripple();
        break;
      case BREATHE:
        set_breathe();
        break;
      case RAIN:
        set_rain();
        break;
      case HEARTBEAT:
        set_heartbeat();
        break;
      case DANCE:
        set_dance();
        break;
    }
  }
}

void set_lazy()
{
  set_speed(200);
  set_acceleration(100);
  set_direction(MIN_DISTANCE);
  set_clock_time(last_hour, last_minute);
}

void set_fun()
{
  set_speed(400);
  set_acceleration(150);
  set_direction(CLOCKWISE2);
  set_clock_time(last_hour, last_minute);
}

void set_waves()
{
  set_speed(800);
  set_acceleration(150);
  set_direction(MIN_DISTANCE);
  set_clock(d_IIII);
  _delay(9000);
  set_speed(400);
  set_acceleration(100);
  set_direction(CLOCKWISE2);
  t_full_clock clock = get_clock_state_from_time(last_hour, last_minute);
  for (int i = 0; i <8; i++)
  {
    set_half_digit(i, clock.digit[i/2].halfs[i%2]);
    delay(400);
  }
}

void stop()
{
  if(!is_stopped)
  {
    is_stopped = true;
    last_hour = -1;
    last_minute = -1;
    set_direction(MIN_DISTANCE);
    set_speed(200);
    set_acceleration(100);
    set_clock(d_stop);
    // Request drivers disable (will happen when motors reach position)
    set_all_drivers_enabled(false);
  }
}

void _delay(int value)
{
  for (int i = 0; i <value/100; i++)
  {
    update_MDNS();
    handle_webclient();
    delay(value/100);
  }
}

// ============================================
// NEW CHOREOGRAPHIES
// See docs/CHOREOGRAPHIES.md for documentation
// ============================================

void set_spinning()
{
  // Phase 1: All hands pointing up (0°)
  set_speed(1500);
  set_acceleration(800);
  set_direction(CLOCKWISE);
  set_clock(d_spin_up);
  _delay(3000);

  // Phase 2: Rotate to down (180°) - clockwise
  set_clock(d_spin_down);
  _delay(3000);

  // Phase 3: Rotate back to up (360°/0°) - clockwise
  set_clock(d_spin_up);
  _delay(3000);

  // Final: Transition to time
  set_speed(1000);
  set_acceleration(500);
  set_direction(MIN_DISTANCE);
  set_clock_time(last_hour, last_minute);
}

void set_squares()
{
  // Phase 1: Form diamond/square pattern
  set_speed(1200);
  set_acceleration(600);
  set_direction(MIN_DISTANCE);
  set_clock(d_squares);
  _delay(4000);

  // Final: Transition to time
  set_speed(1000);
  set_acceleration(500);
  set_clock_time(last_hour, last_minute);
}

void set_symmetrical()
{
  // Phase 1: Left half points left, right half points right
  set_speed(1000);
  set_acceleration(500);
  set_direction(MIN_DISTANCE);

  // Create full clock state for phase 1: left→left, right→right
  t_full_clock sym_phase1 = {digit_sym_left, digit_sym_left, digit_sym_right, digit_sym_right};
  set_clock(sym_phase1);
  _delay(3000);

  // Phase 2: Converge - left points right, right points left
  t_full_clock sym_phase2 = {digit_sym_right, digit_sym_right, digit_sym_left, digit_sym_left};
  set_clock(sym_phase2);
  _delay(3000);

  // Final: Transition to time
  set_clock_time(last_hour, last_minute);
}

void set_wind()
{
  // Phase 1: First wave pattern
  set_speed(800);
  set_acceleration(400);
  set_direction(MIN_DISTANCE);
  set_clock(d_wind_1);
  _delay(2500);

  // Phase 2: Shift wave
  set_clock(d_wind_2);
  _delay(2500);

  // Phase 3: Shift wave again
  set_clock(d_wind_3);
  _delay(2500);

  // Final: Transition to time
  set_speed(1000);
  set_acceleration(500);
  set_clock_time(last_hour, last_minute);
}

void set_cascade()
{
  // Get target time state
  t_full_clock target = get_clock_state_from_time(last_hour, last_minute);

  set_speed(1200);
  set_acceleration(600);
  set_direction(CLOCKWISE);

  // Row 0 (top) - clocks 0, 3, 6, 9, 12, 15, 18, 21
  // In half-digit terms: clock index 0 in each half-digit
  for (int hd = 0; hd < 8; hd++)
  {
    t_half_digitl row0_half = {0};
    // Only set row 0 (index 0), keep others at current
    row0_half.clocks[0] = target.digit[hd/2].halfs[hd%2].clocks[0];
    row0_half.clocks[1].angle_h = 270;
    row0_half.clocks[1].angle_m = 270;
    row0_half.clocks[2].angle_h = 270;
    row0_half.clocks[2].angle_m = 270;
    set_half_digit(hd, row0_half);
  }
  _delay(1500);

  // Row 1 (middle) - clock index 1 in each half-digit
  for (int hd = 0; hd < 8; hd++)
  {
    t_half_digitl row1_half = {0};
    row1_half.clocks[0] = target.digit[hd/2].halfs[hd%2].clocks[0];
    row1_half.clocks[1] = target.digit[hd/2].halfs[hd%2].clocks[1];
    row1_half.clocks[2].angle_h = 270;
    row1_half.clocks[2].angle_m = 270;
    set_half_digit(hd, row1_half);
  }
  _delay(1500);

  // Row 2 (bottom) - complete the time display
  set_direction(MIN_DISTANCE);
  set_clock_time(last_hour, last_minute);
}

void set_firework()
{
  // Phase 1: All hands to center (neutral position)
  set_speed(1000);
  set_acceleration(500);
  set_direction(MIN_DISTANCE);
  set_clock(d_stop);  // All at 270° (pointing left/hidden)
  _delay(2000);

  // Phase 2: Explosion from center outward
  set_speed(2000);
  set_acceleration(1000);
  set_direction(CLOCKWISE);

  // Center columns first (half-digits 3,4)
  set_half_digit(3, digit_firework_inner_left.halfs[1]);
  set_half_digit(4, digit_firework_inner_right.halfs[0]);
  _delay(400);

  // Next ring (half-digits 2 and 5)
  set_half_digit(2, digit_firework_inner_left.halfs[0]);
  set_half_digit(5, digit_firework_inner_right.halfs[1]);
  _delay(400);

  // Outer ring (half-digits 1 and 6)
  set_half_digit(1, digit_firework_outer_left.halfs[1]);
  set_half_digit(6, digit_firework_outer_right.halfs[0]);
  _delay(400);

  // Outermost (half-digits 0 and 7)
  set_half_digit(0, digit_firework_outer_left.halfs[0]);
  set_half_digit(7, digit_firework_outer_right.halfs[1]);
  _delay(2000);

  // Final: Transition to time
  set_speed(1000);
  set_acceleration(500);
  set_direction(MIN_DISTANCE);
  set_clock_time(last_hour, last_minute);
}

// ============================================
// OBLIQUES - Diagonal lines rotation
// ============================================
void set_obliques()
{
  set_speed(1000);
  set_acceleration(500);
  set_direction(CLOCKWISE);

  // Phase 1: Bottom-right diagonals
  set_clock(d_obliques_br);
  _delay(2500);

  // Phase 2: Bottom-left diagonals
  set_clock(d_obliques_bl);
  _delay(2500);

  // Phase 3: Top-right diagonals
  set_clock(d_obliques_tr);
  _delay(2500);

  // Phase 4: Top-left diagonals
  set_clock(d_obliques_tl);
  _delay(2500);

  // Final: Transition to time
  set_direction(MIN_DISTANCE);
  set_clock_time(last_hour, last_minute);
}

// ============================================
// RIPPLE - Concentric rings from center
// ============================================
void set_ripple()
{
  set_speed(1200);
  set_acceleration(600);
  set_direction(MIN_DISTANCE);

  // Phase 1: Ripple outward
  set_clock(d_ripple_out);
  _delay(3000);

  // Phase 2: Ripple inward
  set_clock(d_ripple_in);
  _delay(3000);

  // Phase 3: Ripple outward again
  set_clock(d_ripple_out);
  _delay(3000);

  // Final: Transition to time
  set_clock_time(last_hour, last_minute);
}

// ============================================
// BREATHE - Organic expansion/contraction
// ============================================
void set_breathe()
{
  set_speed(800);
  set_acceleration(400);
  set_direction(MIN_DISTANCE);

  // Phase 1: Expand
  set_clock(d_breathe_expand);
  _delay(2500);

  // Phase 2: Contract
  set_clock(d_breathe_contract);
  _delay(2500);

  // Phase 3: Neutral
  set_clock(d_breathe_neutral);
  _delay(2000);

  // Phase 4: Expand again
  set_clock(d_breathe_expand);
  _delay(2500);

  // Phase 5: Contract
  set_clock(d_breathe_contract);
  _delay(2500);

  // Final: Transition to time
  set_clock_time(last_hour, last_minute);
}

// ============================================
// RAIN - Vertical falling pattern
// ============================================
void set_rain()
{
  set_speed(1500);
  set_acceleration(800);
  set_direction(CLOCKWISE);

  // Phase 1: Rain drops falling
  set_clock(d_rain_1);
  _delay(1500);

  // Phase 2: Drops continue
  set_clock(d_rain_2);
  _delay(1500);

  // Phase 3: Almost at bottom
  set_clock(d_rain_3);
  _delay(1500);

  // Phase 4: Splash
  set_clock(d_rain_splash);
  _delay(2000);

  // Repeat cycle
  set_clock(d_rain_1);
  _delay(1500);

  set_clock(d_rain_2);
  _delay(1500);

  // Final: Transition to time
  set_direction(MIN_DISTANCE);
  set_clock_time(last_hour, last_minute);
}

// ============================================
// HEARTBEAT - Pulsing heart rhythm
// ============================================
void set_heartbeat()
{
  set_speed(1800);
  set_acceleration(900);
  set_direction(MIN_DISTANCE);

  // Beat 1: Systole (contraction)
  set_clock(d_heart_systole);
  _delay(300);

  // Diastole (relaxation)
  set_clock(d_heart_diastole);
  _delay(600);

  // Peak
  set_clock(d_heart_peak);
  _delay(200);

  // Back to diastole
  set_clock(d_heart_diastole);
  _delay(800);

  // Beat 2: Systole
  set_clock(d_heart_systole);
  _delay(300);

  // Diastole
  set_clock(d_heart_diastole);
  _delay(600);

  // Peak
  set_clock(d_heart_peak);
  _delay(200);

  // Diastole
  set_clock(d_heart_diastole);
  _delay(800);

  // Beat 3: Systole
  set_clock(d_heart_systole);
  _delay(300);

  // Diastole
  set_clock(d_heart_diastole);
  _delay(1000);

  // Final: Transition to time
  set_clock_time(last_hour, last_minute);
}

// ============================================
// DANCE MODE - Random shapes chained before time
// ============================================

// Available shape patterns for DANCE mode
const t_full_clock* dance_shapes[] = {
  &d_spin_up,
  &d_spin_down,
  &d_squares,
  &d_IIII,
  &d_fun,
  &d_wind_1,
  &d_wind_2,
  &d_wind_3,
  &d_firework,
  // New organic/geometric patterns
  &d_obliques_br,
  &d_obliques_bl,
  &d_obliques_tr,
  &d_obliques_tl,
  &d_ripple_out,
  &d_ripple_in,
  &d_breathe_expand,
  &d_breathe_contract,
  &d_breathe_neutral,
  &d_rain_1,
  &d_rain_2,
  &d_rain_3,
  &d_rain_splash,
  &d_heart_systole,
  &d_heart_diastole,
  &d_heart_peak
};
const int NUM_DANCE_SHAPES = 26;

void set_dance()
{
  // Seed random with analog noise + time for better randomness
  randomSeed(analogRead(0) + millis());

  // Pick 2 to 4 shapes randomly
  int num_shapes = random(2, 5); // 2, 3, or 4 shapes

  set_speed(1200);
  set_acceleration(600);
  set_direction(MIN_DISTANCE);

  // Chain the random shapes
  int last_shape = -1;
  for (int i = 0; i < num_shapes; i++)
  {
    // Pick a shape different from the last one
    int shape_idx;
    do {
      shape_idx = random(0, NUM_DANCE_SHAPES);
    } while (shape_idx == last_shape);
    last_shape = shape_idx;

    // Apply the shape
    set_clock(*dance_shapes[shape_idx]);

    // Variable delay between shapes (2-4 seconds)
    int delay_time = random(2000, 4000);
    _delay(delay_time);
  }

  // Final: Transition to time display
  set_speed(1000);
  set_acceleration(500);
  set_direction(MIN_DISTANCE);
  set_clock_time(last_hour, last_minute);
}