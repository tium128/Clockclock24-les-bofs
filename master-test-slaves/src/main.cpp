#include <Arduino.h>
#include <Wire.h>
#include <TimeLib.h>
#include <Adafruit_NeoPixel.h>
#define PIN_RGB     21
#define NUMPIXELS   1

#include "i2c.h"
#include "clock_state.h"
#include "clock_manager.h"
#include "digit.h"
#include "wifi_utils.h"
#include "web_server.h"
#include "clock_config.h"
#include "ntp.h"

void setup() {
  Serial.begin(115200);
  Serial.println("\n=== ClockClock24 Slave Tester v1.0 ===");
  Serial.println("Web interface mode - access /test for controls");
  delay(2000);

  // Load configuration from EEPROM
  begin_config();

  // I2C master
  Wire.begin(9, 8, 100000);

  // Status LED - Blue for test mode
  pixels.begin();
  pixels.setBrightness(10);
  pixels.setPixelColor(0, pixels.Color(0, 0, 255));
  pixels.show();

  // WiFi setup
  if(get_connection_mode() == HOTSPOT)
    wifi_create_AP("ClockClock24-Test", "clockclock24");
  else if( !wifi_connect(get_ssid(), get_password(), "clockclock24-test") )
  {
    set_connection_mode(HOTSPOT);
    wifi_create_AP("ClockClock24-Test", "clockclock24");
  }

  if(get_connection_mode() == EXT_CONN)
  {
    begin_NTP();
    setSyncProvider(get_NTP_time);
    setSyncInterval(60 * 30);
  }
  set_wifi_status_led(get_connection_mode());

  // Start web server with test API
  server_start();

  Serial.println("\nReady! Access test interface at:");
  Serial.println("  http://clockclock24-test.local/test");
  Serial.println("  or http://<IP>/test");
}

void loop() {
  // Only handle web requests - no automatic clock updates
  update_MDNS();
  handle_webclient();
}
