#ifndef WIFI_UTILS_H
#define WIFI_UTILS_H

#include <Arduino.h>
#include <WiFi.h> 
#include <ESPmDNS.h>

Adafruit_NeoPixel pixels(NUMPIXELS, PIN_RGB, NEO_GRB + NEO_KHZ800);
const int max_wifi_retries = 50;

/**
 * Connects to a wifi network
 * @param ssid      access point SSID
 * @param password  access point password
 * @param mdns      mdns host name
 * @return true if successful, false otherwise
*/
bool wifi_connect(const char *ssid, const char *password, const char *mdns)
{
  Serial.printf("\nConnecting to %s\n", ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  int i = max_wifi_retries;
  while (WiFi.status() != WL_CONNECTED && i-- > 0)
  {
    delay(200);
    Serial.print(".");
    delay(200);
  }
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.printf("\nWiFi not connected: max wifi retries reached\n");
    return false;
  }
  if (!MDNS.begin(mdns)) 
  { // Start the mDNS responder for clockclok24.local
    Serial.println("Error setting up MDNS responder!");
  } 
  else 
  {
    MDNS.addService("http", "tcp", 80);
  }
  Serial.println("WiFi connected");
  Serial.println("mDNS started: http://clockclock24.local");
  Serial.println("IP address: " + WiFi.localIP().toString());
  return true;
}

/**
 * Creates a wifi network
 * @param ssid    access point SSID
 * @param mdns    mdns host name
 * @return true if successful, false otherwise
*/
bool wifi_create_AP(const char *ssid, const char *mdns)
{
  // Set static IP
  IPAddress AP_LOCAL_IP(192, 168, 1, 10);
  IPAddress AP_GATEWAY_IP(192, 168, 1, 254);
  IPAddress AP_NETWORK_MASK(255, 255, 255, 0);
  Serial.println("\nCreating access point");
  if (!WiFi.softAPConfig(AP_LOCAL_IP, AP_GATEWAY_IP, AP_NETWORK_MASK))
  {
    Serial.println("AP Config Failed");
    return false;
  }
  WiFi.softAP(ssid, NULL);
  IPAddress IP = WiFi.softAPIP();
  if (!MDNS.begin(mdns)) 
  { // Start the mDNS responder
    Serial.println("Error setting up MDNS responder!");
  }
  else 
  {
    MDNS.addService("http", "tcp", 80);
  }
  Serial.printf("mDNS started: http://%s.local\n", mdns);
  Serial.println("IP address: " + AP_LOCAL_IP.toString());
  Serial.println("Gateway IP: " + AP_GATEWAY_IP.toString());
  return true;
}

/**
 * Update MDNS service
*/
void update_MDNS()
{

}

/**
 * Check wifi connection
 * @return true if wifi is connected, false otherwise
*/
bool is_connected()
{
  return WiFi.status() == WL_CONNECTED;
}

/**
 * Set statut LED color to indicate wifi status
*/
void set_wifi_status_led(int mode) {
  if (mode == EXT_CONN) {
    pixels.setPixelColor(0, pixels.Color(255, 255, 255)); // Blanc
  } else if (mode == HOTSPOT) {
    pixels.setPixelColor(0, pixels.Color(0, 0, 255));     // Bleu
  } else {
    pixels.setPixelColor(0, pixels.Color(10, 0, 0));      // Faible rouge = erreur ?
  }
  pixels.show();
}
#endif