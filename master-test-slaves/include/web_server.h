#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <Arduino.h>
#include <WebServer.h>

#include "web_page.h"
#include "clock_manager.h"
#include "clock_config.h"

typedef struct browser_time
{
  int hour;
  int minute;
  int second;
  int day;
  int month;
  int year;
} t_browser_time;

/**
 * Starts and configures the server
*/
void server_start();

/**
 * Client handling, needs to be called on the main loop
*/
void handle_webclient();

/**
 * Stops the server and free resources
*/
void server_stop();

/**
 * Handles GET /
*/
void handle_get();

/**
 * Handles GET /config
*/
void handle_get_config();

/**
 * Handles POST /time
*/
void handle_post_time();

/**
 * Handles POST /adjust
*/
void handle_post_adjust();

/**
 * Handles POST /mode
*/
void handle_post_mode();

/**
 * Handles POST /sleep
*/
void handle_post_sleep();

/**
 * Handles POST /connection
*/
void handle_post_connection();

/**
 * Check if the client changed the time
 * @return true if the client makes a request, false otherwise
*/
bool is_time_changed_browser();

/**
 * Return the client's browser time
 * @return time
*/
t_browser_time get_browser_time();

// ===== TEST API =====

/**
 * Handles GET /test - Test page
*/
void handle_get_test();

/**
 * Handles GET /api/scan - I2C bus scan
*/
void handle_api_scan();

/**
 * Handles GET /api/status - Current status (positions, drivers)
*/
void handle_api_status();

/**
 * Handles POST /api/motor/test - Test single motor
*/
void handle_api_motor_test();

/**
 * Handles POST /api/drivers/enable - Enable all drivers
*/
void handle_api_drivers_enable();

/**
 * Handles POST /api/drivers/disable - Disable all drivers
*/
void handle_api_drivers_disable();

/**
 * Handles POST /api/stop - Move all to stop position
*/
void handle_api_stop();

/**
 * Test a single motor rotation
 * @param board Board address (1-8)
 * @param clock_idx Clock index (0-2)
 * @param hand 0=hour, 1=minute
 * @param direction CLOCKWISE or COUNTERCLOCKWISE
*/
void test_single_motor(int board, int clock_idx, int hand, int direction);

#endif