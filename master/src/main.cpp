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
#include "choreography.h"


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
  // Initialize choreography system (LittleFS)
  choreo_init();
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

  // Update choreography player
  choreo_update();

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
  // Phase 1: All hands pointing up (0°) - progressive column by column
  set_speed(600);
  set_acceleration(300);
  set_direction(CLOCKWISE);

  // Progressive reveal from left to right
  t_full_clock target_up = d_spin_up;
  for (int i = 0; i < 8; i++)
  {
    set_half_digit(i, target_up.digit[i/2].halfs[i%2]);
    _delay(300);
  }
  _delay(2000);

  // Phase 2: Rotate to down (180°) - all together with slower speed
  set_speed(400);
  set_clock(d_spin_down);
  _delay(5000);

  // Phase 3: Rotate back to up (360°/0°)
  set_clock(d_spin_up);
  _delay(5000);

  // Final: Transition to time
  set_speed(400);
  set_acceleration(150);
  set_direction(MIN_DISTANCE);
  set_clock_time(last_hour, last_minute);
}

void set_squares()
{
  set_speed(600);
  set_acceleration(300);
  set_direction(CLOCKWISE);

  // Phase 1: Start with all horizontal (like WAVES)
  set_clock(d_IIII);
  _delay(3000);

  // Phase 2: Progressive reveal of squares pattern from center outward
  t_full_clock target = d_squares;

  // Center columns first (3, 4)
  set_half_digit(3, target.digit[1].halfs[1]);
  set_half_digit(4, target.digit[2].halfs[0]);
  _delay(1500);

  // Next ring (2, 5)
  set_half_digit(2, target.digit[1].halfs[0]);
  set_half_digit(5, target.digit[2].halfs[1]);
  _delay(1500);

  // Outer ring (1, 6)
  set_half_digit(1, target.digit[0].halfs[1]);
  set_half_digit(6, target.digit[3].halfs[0]);
  _delay(1500);

  // Outermost (0, 7)
  set_half_digit(0, target.digit[0].halfs[0]);
  set_half_digit(7, target.digit[3].halfs[1]);
  _delay(3000);

  // Final: Transition to time
  set_speed(400);
  set_acceleration(150);
  set_direction(MIN_DISTANCE);
  set_clock_time(last_hour, last_minute);
}

void set_symmetrical()
{
  set_speed(500);
  set_acceleration(200);
  set_direction(MIN_DISTANCE);

  // Phase 1: Start horizontal
  set_clock(d_IIII);
  _delay(3000);

  // Phase 2: Progressive diverge - outer columns first, then inner
  // Left side points left, right side points right
  t_full_clock sym_diverge = {digit_sym_left, digit_sym_left, digit_sym_right, digit_sym_right};

  // Outer columns (0, 7)
  set_half_digit(0, sym_diverge.digit[0].halfs[0]);
  set_half_digit(7, sym_diverge.digit[3].halfs[1]);
  _delay(1000);

  // Next (1, 6)
  set_half_digit(1, sym_diverge.digit[0].halfs[1]);
  set_half_digit(6, sym_diverge.digit[3].halfs[0]);
  _delay(1000);

  // Next (2, 5)
  set_half_digit(2, sym_diverge.digit[1].halfs[0]);
  set_half_digit(5, sym_diverge.digit[2].halfs[1]);
  _delay(1000);

  // Center (3, 4)
  set_half_digit(3, sym_diverge.digit[1].halfs[1]);
  set_half_digit(4, sym_diverge.digit[2].halfs[0]);
  _delay(3000);

  // Phase 3: Converge - progressive from center outward
  t_full_clock sym_converge = {digit_sym_right, digit_sym_right, digit_sym_left, digit_sym_left};

  // Center first
  set_half_digit(3, sym_converge.digit[1].halfs[1]);
  set_half_digit(4, sym_converge.digit[2].halfs[0]);
  _delay(1000);

  set_half_digit(2, sym_converge.digit[1].halfs[0]);
  set_half_digit(5, sym_converge.digit[2].halfs[1]);
  _delay(1000);

  set_half_digit(1, sym_converge.digit[0].halfs[1]);
  set_half_digit(6, sym_converge.digit[3].halfs[0]);
  _delay(1000);

  set_half_digit(0, sym_converge.digit[0].halfs[0]);
  set_half_digit(7, sym_converge.digit[3].halfs[1]);
  _delay(3000);

  // Final: Transition to time
  set_speed(400);
  set_acceleration(150);
  set_clock_time(last_hour, last_minute);
}

void set_wind()
{
  set_speed(600);
  set_acceleration(250);
  set_direction(CLOCKWISE);

  // Phase 1: All horizontal
  set_clock(d_IIII);
  _delay(3000);

  // Phase 2: Progressive tilt from left to right (wind blowing)
  // Wave effect - columns tilt progressively like grass in wind
  for (int wave = 0; wave < 2; wave++)
  {
    // Wave passes through - tilt columns progressively
    for (int col = 0; col < 8; col++)
    {
      // Tilt this column - diagonal lines pointing down-right
      // h=225 (down-left), m=45 (up-right) creates a diagonal /
      t_half_digitl tilted = {0};
      for (int row = 0; row < 3; row++)
      {
        tilted.clocks[row].angle_h = 225;  // down-left
        tilted.clocks[row].angle_m = 45;   // up-right
      }
      set_half_digit(col, tilted);
      _delay(300);
    }

    // Return to horizontal progressively
    _delay(1000);
    for (int col = 0; col < 8; col++)
    {
      t_half_digitl horizontal = {0};
      for (int row = 0; row < 3; row++)
      {
        horizontal.clocks[row].angle_h = 270;  // left
        horizontal.clocks[row].angle_m = 90;   // right
      }
      set_half_digit(col, horizontal);
      _delay(300);
    }
    _delay(1500);
  }

  // Final: Transition to time
  set_speed(400);
  set_acceleration(150);
  set_direction(MIN_DISTANCE);
  set_clock_time(last_hour, last_minute);
}

void set_cascade()
{
  // Get target time state
  t_full_clock target = get_clock_state_from_time(last_hour, last_minute);

  // Phase 1: Start with all vertical lines pointing up (waterfall source)
  // h=0 (up), m=180 (down) = vertical line |
  set_speed(600);
  set_acceleration(300);
  set_direction(CLOCKWISE);

  t_half_digitl vertical_up = {0};
  for (int row = 0; row < 3; row++)
  {
    vertical_up.clocks[row].angle_h = 0;    // up
    vertical_up.clocks[row].angle_m = 180;  // down
  }
  for (int hd = 0; hd < 8; hd++)
    set_half_digit(hd, vertical_up);
  _delay(3000);

  // Phase 2: Cascade down row by row - top row reveals first
  for (int hd = 0; hd < 8; hd++)
  {
    t_half_digitl row0_half = {0};
    row0_half.clocks[0] = target.digit[hd/2].halfs[hd%2].clocks[0];
    // Keep rows 1,2 as vertical lines
    row0_half.clocks[1].angle_h = 0;
    row0_half.clocks[1].angle_m = 180;
    row0_half.clocks[2].angle_h = 0;
    row0_half.clocks[2].angle_m = 180;
    set_half_digit(hd, row0_half);
  }
  _delay(3000);

  // Row 1 (middle) reveals
  for (int hd = 0; hd < 8; hd++)
  {
    t_half_digitl row1_half = {0};
    row1_half.clocks[0] = target.digit[hd/2].halfs[hd%2].clocks[0];
    row1_half.clocks[1] = target.digit[hd/2].halfs[hd%2].clocks[1];
    // Keep row 2 as vertical line
    row1_half.clocks[2].angle_h = 0;
    row1_half.clocks[2].angle_m = 180;
    set_half_digit(hd, row1_half);
  }
  _delay(3000);

  // Row 2 (bottom) - complete the cascade
  set_speed(400);
  set_acceleration(150);
  set_direction(MIN_DISTANCE);
  set_clock_time(last_hour, last_minute);
}

void set_firework()
{
  // Phase 1: All hands converge to center (build-up)
  set_speed(500);
  set_acceleration(200);
  set_direction(MIN_DISTANCE);
  set_clock(d_stop);  // All at 270° (pointing down/6 o'clock)
  _delay(4000);

  // Phase 2: Explosion from center outward - progressive reveal
  set_speed(800);
  set_acceleration(400);
  set_direction(CLOCKWISE);

  // Center columns first (half-digits 3,4) - the spark
  set_half_digit(3, digit_firework_inner_left.halfs[1]);
  set_half_digit(4, digit_firework_inner_right.halfs[0]);
  _delay(1500);

  // Next ring (half-digits 2 and 5) - explosion expands
  set_half_digit(2, digit_firework_inner_left.halfs[0]);
  set_half_digit(5, digit_firework_inner_right.halfs[1]);
  _delay(1500);

  // Outer ring (half-digits 1 and 6)
  set_half_digit(1, digit_firework_outer_left.halfs[1]);
  set_half_digit(6, digit_firework_outer_right.halfs[0]);
  _delay(1500);

  // Outermost (half-digits 0 and 7) - full explosion
  set_half_digit(0, digit_firework_outer_left.halfs[0]);
  set_half_digit(7, digit_firework_outer_right.halfs[1]);
  _delay(3000);

  // Phase 3: Fade - return to neutral before time
  set_speed(400);
  set_acceleration(150);
  set_direction(MIN_DISTANCE);
  set_clock(d_IIII);  // Horizontal lines
  _delay(2000);

  // Final: Transition to time
  set_clock_time(last_hour, last_minute);
}

// ============================================
// OBLIQUES - Diagonal lines rotation (progressive wave)
// ============================================
void set_obliques()
{
  set_speed(600);
  set_acceleration(300);
  set_direction(CLOCKWISE);

  // Phase 1: Start horizontal
  set_clock(d_IIII);
  _delay(3000);

  // Phase 2: Progressive diagonal rotation - wave from left to right
  // Each column rotates to diagonal progressively
  t_full_clock target_br = d_obliques_br;
  for (int col = 0; col < 8; col++)
  {
    set_half_digit(col, target_br.digit[col/2].halfs[col%2]);
    _delay(400);
  }
  _delay(2000);

  // Phase 3: Rotate all together to next diagonal
  set_clock(d_obliques_tr);
  _delay(4000);

  // Phase 4: Progressive return - wave from right to left
  t_full_clock target_tl = d_obliques_tl;
  for (int col = 7; col >= 0; col--)
  {
    set_half_digit(col, target_tl.digit[col/2].halfs[col%2]);
    _delay(400);
  }
  _delay(2000);

  // Final: Transition to time
  set_speed(400);
  set_acceleration(150);
  set_direction(MIN_DISTANCE);
  set_clock_time(last_hour, last_minute);
}

// ============================================
// RIPPLE - Concentric rings from center (progressive)
// ============================================
void set_ripple()
{
  set_speed(600);
  set_acceleration(300);
  set_direction(MIN_DISTANCE);

  // Phase 1: Start at center, hands hidden
  set_clock(d_stop);
  _delay(3000);

  // Phase 2: Ripple expands from center outward - progressive
  t_full_clock ripple_out = d_ripple_out;

  // Center first (columns 3, 4)
  set_half_digit(3, ripple_out.digit[1].halfs[1]);
  set_half_digit(4, ripple_out.digit[2].halfs[0]);
  _delay(1500);

  // Next ring (columns 2, 5)
  set_half_digit(2, ripple_out.digit[1].halfs[0]);
  set_half_digit(5, ripple_out.digit[2].halfs[1]);
  _delay(1500);

  // Next ring (columns 1, 6)
  set_half_digit(1, ripple_out.digit[0].halfs[1]);
  set_half_digit(6, ripple_out.digit[3].halfs[0]);
  _delay(1500);

  // Outer ring (columns 0, 7)
  set_half_digit(0, ripple_out.digit[0].halfs[0]);
  set_half_digit(7, ripple_out.digit[3].halfs[1]);
  _delay(2000);

  // Phase 3: Ripple contracts inward - progressive
  t_full_clock ripple_in = d_ripple_in;

  // Outer first
  set_half_digit(0, ripple_in.digit[0].halfs[0]);
  set_half_digit(7, ripple_in.digit[3].halfs[1]);
  _delay(1200);

  set_half_digit(1, ripple_in.digit[0].halfs[1]);
  set_half_digit(6, ripple_in.digit[3].halfs[0]);
  _delay(1200);

  set_half_digit(2, ripple_in.digit[1].halfs[0]);
  set_half_digit(5, ripple_in.digit[2].halfs[1]);
  _delay(1200);

  set_half_digit(3, ripple_in.digit[1].halfs[1]);
  set_half_digit(4, ripple_in.digit[2].halfs[0]);
  _delay(2000);

  // Final: Transition to time
  set_speed(400);
  set_acceleration(150);
  set_clock_time(last_hour, last_minute);
}

// ============================================
// BREATHE - Organic expansion/contraction (progressive)
// ============================================
void set_breathe()
{
  set_speed(500);
  set_acceleration(200);
  set_direction(MIN_DISTANCE);

  // Phase 1: Start neutral (horizontal)
  set_clock(d_breathe_neutral);
  _delay(3000);

  // Phase 2: Inhale - progressive expansion from center outward
  t_full_clock expand = d_breathe_expand;

  // Center expands first
  set_half_digit(3, expand.digit[1].halfs[1]);
  set_half_digit(4, expand.digit[2].halfs[0]);
  _delay(800);

  set_half_digit(2, expand.digit[1].halfs[0]);
  set_half_digit(5, expand.digit[2].halfs[1]);
  _delay(800);

  set_half_digit(1, expand.digit[0].halfs[1]);
  set_half_digit(6, expand.digit[3].halfs[0]);
  _delay(800);

  set_half_digit(0, expand.digit[0].halfs[0]);
  set_half_digit(7, expand.digit[3].halfs[1]);
  _delay(2500);

  // Phase 3: Exhale - progressive contraction from outer inward
  t_full_clock contract = d_breathe_contract;

  set_half_digit(0, contract.digit[0].halfs[0]);
  set_half_digit(7, contract.digit[3].halfs[1]);
  _delay(800);

  set_half_digit(1, contract.digit[0].halfs[1]);
  set_half_digit(6, contract.digit[3].halfs[0]);
  _delay(800);

  set_half_digit(2, contract.digit[1].halfs[0]);
  set_half_digit(5, contract.digit[2].halfs[1]);
  _delay(800);

  set_half_digit(3, contract.digit[1].halfs[1]);
  set_half_digit(4, contract.digit[2].halfs[0]);
  _delay(2500);

  // Phase 4: Return to neutral
  set_clock(d_breathe_neutral);
  _delay(3000);

  // Final: Transition to time
  set_speed(400);
  set_acceleration(150);
  set_clock_time(last_hour, last_minute);
}

// ============================================
// RAIN - Vertical falling pattern (row-by-row)
// ============================================
void set_rain()
{
  set_speed(700);
  set_acceleration(350);
  set_direction(CLOCKWISE);

  // Phase 1: All vertical lines pointing up (clouds/source)
  // h=0 (up), m=180 (down) = vertical line |
  t_half_digitl vertical = {0};
  for (int row = 0; row < 3; row++)
  {
    vertical.clocks[row].angle_h = 0;    // up
    vertical.clocks[row].angle_m = 180;  // down
  }
  for (int hd = 0; hd < 8; hd++)
    set_half_digit(hd, vertical);
  _delay(3000);

  // Phase 2: Rain falls - row by row from top to bottom
  // First row falls (top) - becomes horizontal
  for (int hd = 0; hd < 8; hd++)
  {
    t_half_digitl drop = {0};
    drop.clocks[0].angle_h = 270;  // Top row horizontal (left)
    drop.clocks[0].angle_m = 90;   // (right)
    drop.clocks[1].angle_h = 0;    // Middle still vertical
    drop.clocks[1].angle_m = 180;
    drop.clocks[2].angle_h = 0;    // Bottom still vertical
    drop.clocks[2].angle_m = 180;
    set_half_digit(hd, drop);
  }
  _delay(2500);

  // Second row falls (middle)
  for (int hd = 0; hd < 8; hd++)
  {
    t_half_digitl drop = {0};
    drop.clocks[0].angle_h = 270;
    drop.clocks[0].angle_m = 90;
    drop.clocks[1].angle_h = 270;  // Middle row now horizontal
    drop.clocks[1].angle_m = 90;
    drop.clocks[2].angle_h = 0;    // Bottom still vertical
    drop.clocks[2].angle_m = 180;
    set_half_digit(hd, drop);
  }
  _delay(2500);

  // Third row falls (bottom) - all horizontal = splash
  set_clock(d_IIII);
  _delay(3000);

  // Phase 3: Reset to vertical and repeat
  for (int hd = 0; hd < 8; hd++)
    set_half_digit(hd, vertical);
  _delay(2000);

  // Quick falling - all at once to horizontal
  set_speed(800);
  set_clock(d_IIII);
  _delay(3000);

  // Final: Transition to time
  set_speed(400);
  set_acceleration(150);
  set_direction(MIN_DISTANCE);
  set_clock_time(last_hour, last_minute);
}

// ============================================
// HEARTBEAT - Pulsing heart rhythm (realistic timing)
// ============================================
void set_heartbeat()
{
  set_speed(600);
  set_acceleration(300);
  set_direction(MIN_DISTANCE);

  // Phase 1: Start with neutral/relaxed state
  set_clock(d_breathe_neutral);  // Horizontal = resting
  _delay(3000);

  // Beat 1: Systole (contraction) - hands move inward
  set_clock(d_heart_systole);
  _delay(2500);

  // Diastole (relaxation) - hands move outward
  set_clock(d_heart_diastole);
  _delay(3000);

  // Beat 2: Systole (contraction)
  set_clock(d_heart_systole);
  _delay(2500);

  // Diastole (relaxation)
  set_clock(d_heart_diastole);
  _delay(3000);

  // Beat 3: Systole (contraction)
  set_clock(d_heart_systole);
  _delay(2500);

  // Final relaxation
  set_clock(d_heart_diastole);
  _delay(2000);

  // Return to neutral before time
  set_clock(d_breathe_neutral);
  _delay(2000);

  // Final: Transition to time
  set_speed(400);
  set_acceleration(150);
  set_clock_time(last_hour, last_minute);
}

// ============================================
// DANCE MODE - Random shapes chained before time
// ============================================

// Available shape patterns for DANCE mode (only visually distinct patterns)
const t_full_clock* dance_shapes[] = {
  &d_spin_up,
  &d_spin_down,
  &d_squares,
  &d_IIII,
  &d_fun,
  &d_firework,
  &d_obliques_br,
  &d_obliques_tl,
  &d_ripple_out,
  &d_breathe_expand,
  &d_breathe_contract,
  &d_stop
};
const int NUM_DANCE_SHAPES = 12;

void set_dance()
{
  // Seed random with analog noise + time for better randomness
  randomSeed(analogRead(0) + millis());

  // Pick 3 to 5 shapes randomly
  int num_shapes = random(3, 6);

  set_speed(600);
  set_acceleration(300);
  set_direction(CLOCKWISE);

  // Start from a known state
  set_clock(d_IIII);
  _delay(3000);

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

    // Fixed delay between shapes (4 seconds for motor completion)
    _delay(4000);
  }

  // Final: Transition to time display
  set_speed(400);
  set_acceleration(150);
  set_direction(MIN_DISTANCE);
  set_clock_time(last_hour, last_minute);
}