#include "web_server.h"
#include <Wire.h>
#include "clock_state.h"
#include "i2c.h"
#include "digit.h"

WebServer _server(80);

t_browser_time _browser_time = {0, 0, 0, 0, 0, 0};
bool _time_changed_browser = false;

// Test state tracking
static int _motor_positions[8][3][2]; // [board][clock][hand] = angle
static bool _drivers_enabled = true;
static uint32_t _test_counter = 1;

// Initialize positions to 270 (6h00)
void init_test_positions() {
  for(int b = 0; b < 8; b++)
    for(int c = 0; c < 3; c++)
      for(int h = 0; h < 2; h++)
        _motor_positions[b][c][h] = 270;
  _drivers_enabled = true;
}

void server_start()
{
  // Initialize test positions
  init_test_positions();

  // Setup web server connection
  _server.enableCORS(true);
  _server.begin();
  _server.on("/", HTTP_GET, handle_get);
  _server.on("/test", HTTP_GET, handle_get_test);
  _server.on("/config", HTTP_GET, handle_get_config);
  _server.on("/time", HTTP_POST, handle_post_time);
  _server.on("/adjust", HTTP_POST, handle_post_adjust);
  _server.on("/mode", HTTP_POST, handle_post_mode);
  _server.on("/sleep", HTTP_POST, handle_post_sleep);
  _server.on("/connection", HTTP_POST, handle_post_connection);
  // Test API endpoints
  _server.on("/api/scan", HTTP_GET, handle_api_scan);
  _server.on("/api/status", HTTP_GET, handle_api_status);
  _server.on("/api/motor/test", HTTP_POST, handle_api_motor_test);
  _server.on("/api/drivers/enable", HTTP_POST, handle_api_drivers_enable);
  _server.on("/api/drivers/disable", HTTP_POST, handle_api_drivers_disable);
  _server.on("/api/stop", HTTP_POST, handle_api_stop);
  Serial.println("WebServer setup done (with test API)");
}

void handle_webclient()
{
  _server.handleClient();
}

void server_stop()
{
  _server.close();
}

void handle_get()
{
  Serial.println("Handle GET /");
  _server.send(200, "text/html", WEB_PAGE);
}

void handle_get_config()
{
  Serial.println("Handle GET /config");\
  char payload[1024];
  {
    char s_time[512] = "[";
    for (int i = 0; i < 7; i++)
    {
      strncat(s_time, "[", 2);
      for (int j = 0; j < 24; j++)
      {
        strncat(s_time, get_sleep_time(i, j) ? "1" : "0", 2);
        if(j < 23)
          strncat(s_time,"," , sizeof(2));
      }
      strncat(s_time, "]", sizeof(2));
      if(i < 6)
        strncat(s_time,"," , sizeof(2));
    }
    strncat(s_time, "]", sizeof(2));
    snprintf(payload, sizeof(payload), 
      "{\"clock_mode\":%d,"
      "\"wireless_mode\":%d,"
      "\"ssid\":\"%s\","
      "\"password\":\"%s\","
      "\"sleep_time\":%s}",
      get_clock_mode(), get_connection_mode(), get_ssid(), get_password(), s_time);
  }
  _server.send(200, "application/json", payload);
}

void handle_post_time()
{
  Serial.println("Handle POST /time");
  if (_server.hasArg("h"))
    _browser_time.hour = _server.arg("h").toInt();
  if (_server.hasArg("m"))
    _browser_time.minute = _server.arg("m").toInt();
  if (_server.hasArg("s"))
    _browser_time.second = _server.arg("s").toInt();
  if (_server.hasArg("D"))
    _browser_time.day = _server.arg("D").toInt();
  if (_server.hasArg("M"))
    _browser_time.month = _server.arg("M").toInt();
  if (_server.hasArg("Y"))
    _browser_time.year = _server.arg("Y").toInt();
  if (_server.hasArg("timezone"))
  {
    int _browser_timezone = _server.arg("timezone").toInt();
    set_timezone(_browser_timezone);
  }
  _time_changed_browser = true;
  _server.send(200, "text/plain", "");
  Serial.printf("Time received: %d:%d:%d\n", 
    _browser_time.hour, _browser_time.minute, _browser_time.second);
}

void handle_post_adjust()
{
  Serial.println("Handle POST /adjust");
  int clock_index = 0;
  int m_amount = 0;
  int h_amount = 0;
  if (_server.hasArg("index"))
    clock_index = _server.arg("index").toInt();
  if (_server.hasArg("m_amount"))
    m_amount = _server.arg("m_amount").toInt();
  if (_server.hasArg("h_amount"))
    h_amount = _server.arg("h_amount").toInt();

  _server.send(200, "text/plain", "");

  Serial.printf("Adjust received, clock: %d, m_amount: %d, h_amount: %d\n", 
    clock_index, m_amount, h_amount);
  adjust_hands(clock_index, m_amount, h_amount);
}

void handle_post_mode()
{
  Serial.println("Handle POST /mode");
  if (_server.hasArg("mode"))
    set_clock_mode(_server.arg("mode").toInt());
  _server.send(200, "text/plain", "");
}

void handle_post_sleep()
{
  Serial.println("Handle POST /sleep");
  if (_server.hasArg("day"))
  {
    int day = _server.arg("day").toInt();
    for(int i = 0; i < 24; i++)
    {
      char arg[8];
      snprintf(arg, sizeof(arg), "h%d", i);
      if (_server.hasArg(arg))
        set_sleep_time(day, i, _server.arg(arg).toInt() == 0 ? false : true);
    }
    save_sleep_time();
  }
  _server.send(200, "text/html", "");
}

void handle_post_connection()
{
  Serial.println("Handle POST /connection");
  if (_server.hasArg("mode"))
    set_connection_mode(_server.arg("mode").toInt());
  if (_server.hasArg("ssid"))
    set_ssid(_server.arg("ssid").c_str());
  if (_server.hasArg("password"))
    set_password(_server.arg("password").c_str());
  _server.send(200, "text/plain", "");
  end_config();
  ESP.restart();
}

bool is_time_changed_browser()
{
  bool tmp = _time_changed_browser;
  _time_changed_browser = false;
  return tmp;
}

t_browser_time get_browser_time()
{
  return _browser_time;
}

// ===== TEST API HANDLERS =====

// Test page HTML (embedded)
const char TEST_PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>ClockClock24 - Slave Tester</title>
  <style>
    body { font-family: monospace; background: #1a1a2e; color: #eee; padding: 20px; }
    h1 { color: #0f0; }
    .section { background: #16213e; padding: 15px; margin: 10px 0; border-radius: 8px; }
    .section h2 { margin-top: 0; color: #00adb5; }
    button { background: #00adb5; color: #fff; border: none; padding: 10px 20px; margin: 5px; cursor: pointer; border-radius: 4px; }
    button:hover { background: #00d4e0; }
    button.danger { background: #e94560; }
    button.danger:hover { background: #ff6b6b; }
    select, input { padding: 8px; margin: 5px; background: #0f3460; color: #fff; border: 1px solid #00adb5; border-radius: 4px; }
    #logs { background: #0a0a15; padding: 10px; height: 200px; overflow-y: auto; font-size: 12px; border-radius: 4px; }
    .log-entry { margin: 2px 0; }
    .log-ok { color: #0f0; }
    .log-err { color: #f00; }
    .log-info { color: #0af; }
    .board-status { display: inline-block; width: 30px; height: 30px; margin: 3px; text-align: center; line-height: 30px; border-radius: 4px; }
    .board-ok { background: #0f0; color: #000; }
    .board-err { background: #333; color: #666; }
    .inline { display: flex; align-items: center; flex-wrap: wrap; }
  </style>
</head>
<body>
  <h1>ClockClock24 Slave Tester</h1>

  <div class="section">
    <h2>I2C Scanner</h2>
    <button onclick="scanI2C()">Scan I2C Bus</button>
    <div id="boards" class="inline" style="margin-top:10px;"></div>
  </div>

  <div class="section">
    <h2>Motor Test</h2>
    <div class="inline">
      <label>Board: <select id="board">
        <option value="1">1</option><option value="2">2</option><option value="3">3</option><option value="4">4</option>
        <option value="5">5</option><option value="6">6</option><option value="7">7</option><option value="8">8</option>
      </select></label>
      <label>Clock: <select id="clock">
        <option value="0">0</option><option value="1">1</option><option value="2">2</option>
      </select></label>
      <label>Hand: <select id="hand">
        <option value="H">Hour</option><option value="M">Minute</option>
      </select></label>
    </div>
    <div style="margin-top:10px;">
      <button onclick="testMotor('CW')">Rotate CW 360°</button>
      <button onclick="testMotor('CCW')">Rotate CCW 360°</button>
    </div>
  </div>

  <div class="section">
    <h2>Drivers</h2>
    <button onclick="enableDrivers()">Enable All Drivers</button>
    <button onclick="disableDrivers()" class="danger">Disable All Drivers</button>
  </div>

  <div class="section">
    <h2>Position</h2>
    <button onclick="moveToStop()">All to 6h00 (Stop)</button>
    <button onclick="getStatus()">Refresh Status</button>
  </div>

  <div class="section">
    <h2>Logs</h2>
    <div id="logs"></div>
  </div>

  <script>
    function log(msg, type='info') {
      const logs = document.getElementById('logs');
      const entry = document.createElement('div');
      entry.className = 'log-entry log-' + type;
      entry.textContent = new Date().toLocaleTimeString() + ' - ' + msg;
      logs.appendChild(entry);
      logs.scrollTop = logs.scrollHeight;
    }

    async function scanI2C() {
      log('Scanning I2C bus...', 'info');
      try {
        const res = await fetch('/api/scan');
        const data = await res.json();
        const boardsDiv = document.getElementById('boards');
        boardsDiv.innerHTML = '';
        data.boards.forEach(b => {
          const div = document.createElement('div');
          div.className = 'board-status ' + (b.found ? 'board-ok' : 'board-err');
          div.textContent = b.address;
          div.title = b.found ? 'Found' : 'Not found';
          boardsDiv.appendChild(div);
        });
        log('Scan complete: ' + data.count + ' board(s) found', 'ok');
      } catch(e) {
        log('Scan failed: ' + e, 'err');
      }
    }

    async function testMotor(direction) {
      const board = document.getElementById('board').value;
      const clock = document.getElementById('clock').value;
      const hand = document.getElementById('hand').value;
      log('Testing motor: Board ' + board + ', Clock ' + clock + ', Hand ' + hand + ', ' + direction, 'info');
      try {
        const res = await fetch('/api/motor/test', {
          method: 'POST',
          headers: {'Content-Type': 'application/x-www-form-urlencoded'},
          body: 'board=' + board + '&clock=' + clock + '&hand=' + hand + '&direction=' + direction
        });
        const data = await res.json();
        log(data.message, data.success ? 'ok' : 'err');
      } catch(e) {
        log('Motor test failed: ' + e, 'err');
      }
    }

    async function enableDrivers() {
      log('Enabling all drivers...', 'info');
      try {
        const res = await fetch('/api/drivers/enable', {method: 'POST'});
        const data = await res.json();
        log(data.message, 'ok');
      } catch(e) {
        log('Enable failed: ' + e, 'err');
      }
    }

    async function disableDrivers() {
      log('Disabling all drivers...', 'info');
      try {
        const res = await fetch('/api/drivers/disable', {method: 'POST'});
        const data = await res.json();
        log(data.message, 'ok');
      } catch(e) {
        log('Disable failed: ' + e, 'err');
      }
    }

    async function moveToStop() {
      log('Moving all hands to 6h00...', 'info');
      try {
        const res = await fetch('/api/stop', {method: 'POST'});
        const data = await res.json();
        log(data.message, 'ok');
      } catch(e) {
        log('Move failed: ' + e, 'err');
      }
    }

    async function getStatus() {
      log('Getting status...', 'info');
      try {
        const res = await fetch('/api/status');
        const data = await res.json();
        log('Drivers: ' + (data.drivers_enabled ? 'ENABLED' : 'DISABLED'), data.drivers_enabled ? 'ok' : 'err');
      } catch(e) {
        log('Status failed: ' + e, 'err');
      }
    }

    // Initial scan on load
    window.onload = () => {
      log('ClockClock24 Slave Tester ready', 'ok');
      scanI2C();
    };
  </script>
</body>
</html>
)rawliteral";

void handle_get_test()
{
  Serial.println("Handle GET /test");
  _server.send(200, "text/html", TEST_PAGE);
}

void handle_api_scan()
{
  Serial.println("API: Scan I2C");
  String json = "{\"boards\":[";
  int count = 0;

  for(int addr = 1; addr <= 8; addr++) {
    Wire.beginTransmission(addr);
    int error = Wire.endTransmission();
    bool found = (error == 0);
    if(found) count++;

    if(addr > 1) json += ",";
    json += "{\"address\":" + String(addr) + ",\"found\":" + (found ? "true" : "false") + "}";
  }

  json += "],\"count\":" + String(count) + "}";
  _server.send(200, "application/json", json);
}

void handle_api_status()
{
  Serial.println("API: Get status");
  String json = "{\"drivers_enabled\":" + String(_drivers_enabled ? "true" : "false");
  json += ",\"positions\":[";

  for(int b = 0; b < 8; b++) {
    for(int c = 0; c < 3; c++) {
      if(b > 0 || c > 0) json += ",";
      json += "{\"board\":" + String(b+1);
      json += ",\"clock\":" + String(c);
      json += ",\"angle_h\":" + String(_motor_positions[b][c][0]);
      json += ",\"angle_m\":" + String(_motor_positions[b][c][1]);
      json += "}";
    }
  }

  json += "]}";
  _server.send(200, "application/json", json);
}

void handle_api_motor_test()
{
  Serial.println("API: Motor test");

  if(!_server.hasArg("board") || !_server.hasArg("clock") ||
     !_server.hasArg("hand") || !_server.hasArg("direction")) {
    _server.send(400, "application/json", "{\"success\":false,\"message\":\"Missing parameters\"}");
    return;
  }

  int board = _server.arg("board").toInt();
  int clock_idx = _server.arg("clock").toInt();
  String hand_str = _server.arg("hand");
  String dir_str = _server.arg("direction");

  if(board < 1 || board > 8 || clock_idx < 0 || clock_idx > 2) {
    _server.send(400, "application/json", "{\"success\":false,\"message\":\"Invalid board or clock\"}");
    return;
  }

  int hand = (hand_str == "H" || hand_str == "h") ? 0 : 1;
  int direction = (dir_str == "CW" || dir_str == "cw") ? CLOCKWISE : COUNTERCLOCKWISE;

  test_single_motor(board, clock_idx, hand, direction);

  String msg = "Motor test: Board " + String(board) + ", Clock " + String(clock_idx);
  msg += ", " + String(hand == 0 ? "Hour" : "Minute") + " hand, " + dir_str;
  _server.send(200, "application/json", "{\"success\":true,\"message\":\"" + msg + "\"}");
}

void handle_api_drivers_enable()
{
  Serial.println("API: Enable drivers");
  set_all_drivers_enabled(true);
  _drivers_enabled = true;
  _server.send(200, "application/json", "{\"success\":true,\"message\":\"All drivers enabled\"}");
}

void handle_api_drivers_disable()
{
  Serial.println("API: Disable drivers");
  set_all_drivers_enabled(false);
  _drivers_enabled = false;
  _server.send(200, "application/json", "{\"success\":true,\"message\":\"Drivers disable requested (will disable when motors stop)\"}");
}

void handle_api_stop()
{
  Serial.println("API: Move to stop position");
  set_direction(MIN_DISTANCE);
  set_speed(200);
  set_acceleration(100);
  set_clock(d_stop);

  // Update tracked positions
  for(int b = 0; b < 8; b++)
    for(int c = 0; c < 3; c++)
      for(int h = 0; h < 2; h++)
        _motor_positions[b][c][h] = 270;

  _server.send(200, "application/json", "{\"success\":true,\"message\":\"Moving all hands to 6h00\"}");
}

void test_single_motor(int board, int clock_idx, int hand, int direction)
{
  t_half_digit hd = {0};

  // Set all clocks to current position (no move)
  for(int i = 0; i < 3; i++) {
    hd.clocks[i].angle_h = _motor_positions[board-1][i][0];
    hd.clocks[i].angle_m = _motor_positions[board-1][i][1];
    hd.clocks[i].speed_h = 200;
    hd.clocks[i].speed_m = 200;
    hd.clocks[i].accel_h = 100;
    hd.clocks[i].accel_m = 100;
    hd.clocks[i].mode_h = MIN_DISTANCE;
    hd.clocks[i].mode_m = MIN_DISTANCE;
    hd.change_counter[i] = _test_counter;
  }

  // Set target motor to rotate 360 degrees
  if(hand == 0) {
    hd.clocks[clock_idx].mode_h = direction;
    // Position stays same but direction mode makes it do full rotation
  } else {
    hd.clocks[clock_idx].mode_m = direction;
  }

  hd.change_counter[clock_idx] = ++_test_counter;

  // Send to board
  Wire.beginTransmission(board);
  I2C_writeAnything(hd);
  Wire.endTransmission();

  Serial.printf("Test motor: board=%d, clock=%d, hand=%d, dir=%d\n", board, clock_idx, hand, direction);
}