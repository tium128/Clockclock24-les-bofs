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
 * Handles GET /test (diagnostic page)
*/
void handle_get_test();

/**
 * Handles GET /api/scan
*/
void handle_api_scan();

/**
 * Handles GET /api/status
*/
void handle_api_status();

/**
 * Handles POST /api/settings
*/
void handle_api_settings();

/**
 * Handles POST /api/motor/test
*/
void handle_api_motor_test();

/**
 * Handles POST /api/drivers/enable
*/
void handle_api_drivers_enable();

/**
 * Handles POST /api/drivers/disable
*/
void handle_api_drivers_disable();

/**
 * Handles POST /api/stop
*/
void handle_api_stop();

/**
 * Handles POST /api/motor/position
 * Move a specific hand to a target position (clock convention: 0=12h, 90=3h, 180=6h, 270=9h)
*/
void handle_api_motor_position();

// ===== CHOREOGRAPHY ENDPOINTS =====

/**
 * Handles GET /choreography (designer page)
*/
void handle_get_choreography();

/**
 * Handles GET /api/choreo/list
*/
void handle_api_choreo_list();

/**
 * Handles GET /api/choreo/load?name=xxx
*/
void handle_api_choreo_load();

/**
 * Handles POST /api/choreo/save (body = JSON)
*/
void handle_api_choreo_save();

/**
 * Handles POST /api/choreo/delete?name=xxx
*/
void handle_api_choreo_delete();

/**
 * Handles POST /api/choreo/play
*/
void handle_api_choreo_play();

/**
 * Handles POST /api/choreo/pause
*/
void handle_api_choreo_pause();

/**
 * Handles POST /api/choreo/stop
*/
void handle_api_choreo_stop();

/**
 * Handles POST /api/choreo/next
*/
void handle_api_choreo_next();

/**
 * Handles POST /api/choreo/prev
*/
void handle_api_choreo_prev();

/**
 * Handles GET /api/choreo/status
*/
void handle_api_choreo_status();

/**
 * Handles POST /api/choreo/apply?keyframe=N
*/
void handle_api_choreo_apply();

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

#endif