#include "clock_config.h"

// Non volatile preferences
Preferences prefs;

// Internal config state
int _clock_mode;
bool _sleep_time[7 * 24];
int _clock_timezone;

int _wireless_mode;
char _ssid[64];
char _password[64];

// Choreography settings
int _choreo_mode;
uint16_t _choreo_enabled_mask;

void begin_config()
{
  prefs.begin("clockclock24");
  _clock_mode = prefs.getInt("clock_mode", LAZY);
  _wireless_mode = prefs.getInt("wireless_mode", HOTSPOT);
  _clock_timezone = prefs.getInt("clock_timezone", 0);
  strncpy(_ssid, prefs.getString("ssid", "").c_str(), sizeof(_ssid));
  strncpy(_password, prefs.getString("password", "").c_str(), sizeof(_password));
  if(prefs.isKey("sleep_time"))
    prefs.getBytes("sleep_time", _sleep_time, sizeof(_sleep_time));
  else
    memset(_sleep_time, 0, sizeof(_sleep_time));

  // Choreography settings
  _choreo_mode = prefs.getInt("choreo_mode", 0);  // Default: OFF
  _choreo_enabled_mask = prefs.getUShort("choreo_mask", 0xFFFF);  // Default: all enabled
}

void end_config()
{
  prefs.end();
}

void clear_config()
{
  prefs.clear();
  _clock_mode = LAZY;
  _wireless_mode = HOTSPOT;
  strncpy(_ssid, "", sizeof(_ssid));
  strncpy(_password, "", sizeof(_password));
  memset(_sleep_time, 0, sizeof(_sleep_time));
  _choreo_mode = 0;
  _choreo_enabled_mask = 0xFFFF;
}

int get_clock_mode()
{
  return _clock_mode;
}

bool get_sleep_time(int day, int hour)
{
  return _sleep_time[(day * 24) + (hour % 24)];
}

int get_connection_mode()
{
  return _wireless_mode;
}

int get_timezone()
{
  return _clock_timezone;
}

char *get_ssid()
{
  return _ssid;
}

char *get_password()
{
  return _password;
}

void set_clock_mode(int value)
{
  _clock_mode = value;
  prefs.putInt("clock_mode", value);
}

void set_sleep_time(int day, int hour, bool value)
{
  _sleep_time[(day * 24) + (hour % 24)] = value;
}

void save_sleep_time()
{
  prefs.putBytes("sleep_time", _sleep_time, sizeof(_sleep_time));
}

void set_connection_mode(int value)
{
  _wireless_mode = value;
  prefs.putInt("wireless_mode", value);
}

void set_timezone(int value)
{
  _clock_timezone = value;
  prefs.putInt("clock_timezone", value);
}

void set_ssid(const char *value)
{
  strncpy(_ssid, value, sizeof(_ssid));
  prefs.putString("ssid", value);
}

void set_password(const char *value)
{
  strncpy(_password, value, sizeof(_password));
  prefs.putString("password", value);
}

int get_choreo_mode()
{
  return _choreo_mode;
}

void set_choreo_mode(int value)
{
  _choreo_mode = value;
  prefs.putInt("choreo_mode", value);
}

uint16_t get_choreo_enabled_mask()
{
  return _choreo_enabled_mask;
}

void set_choreo_enabled_mask(uint16_t value)
{
  _choreo_enabled_mask = value;
  prefs.putUShort("choreo_mask", value);
}