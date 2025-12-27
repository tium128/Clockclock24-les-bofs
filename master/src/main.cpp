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
bool is_adjusting = false;

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
 * Sets clock time using custom/choreography mode (LAZY style + choreo triggers)
*/
void set_custom();

/**
 * Sets clock to adjust state (hands at 6:00, drivers ON)
*/
void set_adjust();

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

  // Only update time display if no choreography is playing
  if (choreo_get_state() != CHOREO_PLAYING) {
    int mode = get_clock_mode();
    if (mode == OFF)
      stop();
    else if (mode == ADJUST)
      set_adjust();
    else
      set_time();
  }

  // Update choreography player
  choreo_update();

  // Check for 2/min frequency trigger (if mode is CUSTOM with auto/random)
  if (get_clock_mode() == CUSTOM) {
    choreo_check_frequency_trigger(second());
  }

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
    // Re-enable drivers if coming from stopped or adjusting state
    if(is_stopped || is_adjusting)
    {
      set_all_drivers_enabled(true);
      delay(500); // Wait for all drivers to be fully enabled before sending positions
    }
    is_stopped = false;
    is_adjusting = false;

    // Check for hour change to trigger choreography
    bool hourChanged = (hour() != last_hour) && (last_hour != -1);

    last_hour = hour();
    last_minute = minute();

    // Trigger choreography on hour change (if in auto/random mode)
    if (hourChanged) {
      choreo_on_hour_change();
      // If choreography started, don't update time display yet
      if (choreo_get_state() == CHOREO_PLAYING) {
        return;
      }
    }

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
      case CUSTOM:
        set_custom();
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
    is_adjusting = false;
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

void set_custom()
{
  // CUSTOM mode: display time with LAZY style animation
  // Choreography triggers are handled by choreo_update() based on /choreography settings
  set_speed(200);
  set_acceleration(100);
  set_direction(MIN_DISTANCE);
  set_clock_time(last_hour, last_minute);
}

void set_adjust()
{
  // ADJUST mode: hands at 6:00, drivers stay ON for calibration
  if (!is_adjusting)
  {
    is_adjusting = true;
    is_stopped = false;
    last_hour = -1;
    last_minute = -1;
    set_direction(MIN_DISTANCE);
    set_speed(200);
    set_acceleration(100);
    set_clock(d_stop);  // d_stop sets all hands to 6:00 (digit_null)
    // Keep drivers enabled for micro-adjustments via web interface
    set_all_drivers_enabled(true);
  }
}