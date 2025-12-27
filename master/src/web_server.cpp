#include "web_server.h"
#include <Wire.h>
#include "i2c.h"
#include "digit.h"
#include "choreography.h"

// Forward declarations for choreography mode API
void handle_api_choreo_mode_get();
void handle_api_choreo_mode_set();
void handle_api_choreo_enable();
void handle_api_choreo_list_full();
void handle_api_choreo_frequency_get();
void handle_api_choreo_frequency_set();

WebServer _server(80);

t_browser_time _browser_time = {0, 0, 0, 0, 0, 0};
bool _time_changed_browser = false;

// Test state tracking
static int _motor_positions[8][3][2]; // [board][clock][hand] = angle
static bool _drivers_enabled = true;
static uint32_t _test_counter = 1;
static int _test_speed = 1000;
static int _test_accel = 500;

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
  _server.on("/config", HTTP_GET, handle_get_config);
  _server.on("/time", HTTP_POST, handle_post_time);
  _server.on("/adjust", HTTP_POST, handle_post_adjust);
  _server.on("/mode", HTTP_POST, handle_post_mode);
  _server.on("/sleep", HTTP_POST, handle_post_sleep);
  _server.on("/connection", HTTP_POST, handle_post_connection);
  // Diagnostic API endpoints
  _server.on("/test", HTTP_GET, handle_get_test);
  _server.on("/api/scan", HTTP_GET, handle_api_scan);
  _server.on("/api/status", HTTP_GET, handle_api_status);
  _server.on("/api/motor/test", HTTP_POST, handle_api_motor_test);
  _server.on("/api/drivers/enable", HTTP_POST, handle_api_drivers_enable);
  _server.on("/api/drivers/disable", HTTP_POST, handle_api_drivers_disable);
  _server.on("/api/stop", HTTP_POST, handle_api_stop);
  _server.on("/api/settings", HTTP_POST, handle_api_settings);
  _server.on("/api/motor/position", HTTP_POST, handle_api_motor_position);
  // Choreography API endpoints
  _server.on("/choreography", HTTP_GET, handle_get_choreography);
  _server.on("/api/choreo/list", HTTP_GET, handle_api_choreo_list);
  _server.on("/api/choreo/load", HTTP_GET, handle_api_choreo_load);
  _server.on("/api/choreo/save", HTTP_POST, handle_api_choreo_save);
  _server.on("/api/choreo/delete", HTTP_POST, handle_api_choreo_delete);
  _server.on("/api/choreo/play", HTTP_POST, handle_api_choreo_play);
  _server.on("/api/choreo/pause", HTTP_POST, handle_api_choreo_pause);
  _server.on("/api/choreo/stop", HTTP_POST, handle_api_choreo_stop);
  _server.on("/api/choreo/next", HTTP_POST, handle_api_choreo_next);
  _server.on("/api/choreo/prev", HTTP_POST, handle_api_choreo_prev);
  _server.on("/api/choreo/status", HTTP_GET, handle_api_choreo_status);
  _server.on("/api/choreo/apply", HTTP_POST, handle_api_choreo_apply);
  _server.on("/api/choreo/mode", HTTP_GET, handle_api_choreo_mode_get);
  _server.on("/api/choreo/mode", HTTP_POST, handle_api_choreo_mode_set);
  _server.on("/api/choreo/enable", HTTP_POST, handle_api_choreo_enable);
  _server.on("/api/choreo/listfull", HTTP_GET, handle_api_choreo_list_full);
  _server.on("/api/choreo/frequency", HTTP_GET, handle_api_choreo_frequency_get);
  _server.on("/api/choreo/frequency", HTTP_POST, handle_api_choreo_frequency_set);
  Serial.println("WebServer setup done");
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
  Serial.println("Handle GET /config");
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
  adjust_hands(clock_index, h_amount, m_amount);
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

// ===== DIAGNOSTIC API HANDLERS =====

// Test page HTML with same styling as main page
const char TEST_PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>ClockClock24 - Diagnostics</title>
  <style>
    html{width:100%;height:100%}
    body{display:flex;flex-direction:column;align-items:center;font-family:Tahoma,Helvetica,sans-serif;color:#fff;background-color:#212121;cursor:default;user-select:none;font-size:14px;padding:20px}
    h1{margin-bottom:8px}
    .title{font-size:16px;font-weight:700;margin-bottom:8px;text-align:center;margin-top:24px}
    .section{width:100%;max-width:600px;margin:10px 0}
    .btn{min-width:72px;height:48px;margin:4px;font-size:14px;border-width:0;padding:0 16px;background-color:transparent;color:#fff;box-shadow:inset 0 0 2px #dfdfdf;cursor:pointer;display:inline-flex;align-items:center;justify-content:center}
    .btn:hover{background-color:#b4b4b44b}
    .btn.danger{box-shadow:inset 0 0 2px #ff6b6b}
    .btn.danger:hover{background-color:#ff6b6b33}
    .btn-back{position:absolute;top:20px;left:20px}
    select{padding:8px 16px;margin:4px;background-color:#212121;color:#fff;border:1px solid #dfdfdf;font-size:14px}
    .inline{display:flex;align-items:center;flex-wrap:wrap;gap:8px;margin:8px 0}
    #logs{background:#1a1a1a;padding:10px;height:150px;overflow-y:auto;font-size:12px;font-family:monospace;margin-top:8px;box-shadow:inset 0 0 2px #dfdfdf}
    .log-entry{margin:2px 0}
    .log-ok{color:#4f4}
    .log-err{color:#f44}
    .log-info{color:#4af}
    .board-status{display:inline-block;width:30px;height:30px;margin:3px;text-align:center;line-height:30px;font-size:12px}
    .board-ok{background:#4f4;color:#000}
    .board-err{background:#333;color:#666}
    .slider-container{display:flex;align-items:center;gap:10px;margin:8px 0}
    .slider-container label{min-width:100px}
    .slider-container input[type=range]{flex:1;max-width:300px}
    .slider-container span{min-width:60px;text-align:right}
    input[type=range]{-webkit-appearance:none;background:#333;height:8px;border-radius:4px}
    input[type=range]::-webkit-slider-thumb{-webkit-appearance:none;width:20px;height:20px;background:#dfdfdf;border-radius:50%;cursor:pointer}
    a{color:#8a8a8a;text-decoration:none}
  </style>
</head>
<body>
  <a href="/" class="btn btn-back">← Back</a>
  <h1>Diagnostics</h1>

  <div class="section">
    <div class="title">I2C Scanner</div>
    <button class="btn" onclick="scanI2C()">Scan I2C Bus</button>
    <div id="boards" class="inline" style="margin-top:10px;"></div>
  </div>

  <div class="section">
    <div class="title">Motor Test</div>
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
      <button class="btn" onclick="testMotor('CW')">Rotate CW 180°</button>
      <button class="btn" onclick="testMotor('CCW')">Rotate CCW 180°</button>
    </div>
  </div>

  <div class="section">
    <div class="title">Speed Settings</div>
    <div class="slider-container">
      <label>Speed:</label>
      <input type="range" id="speed" min="200" max="5000" value="1000" oninput="updateSpeedLabel()">
      <span id="speedVal">1000</span>
    </div>
    <div class="slider-container">
      <label>Acceleration:</label>
      <input type="range" id="accel" min="100" max="2000" value="500" oninput="updateAccelLabel()">
      <span id="accelVal">500</span>
    </div>
    <button class="btn" onclick="applySettings()">Apply</button>
  </div>

  <div class="section">
    <div class="title">Drivers</div>
    <button class="btn" onclick="enableDrivers()">Enable All</button>
    <button class="btn danger" onclick="disableDrivers()">Disable All</button>
  </div>

  <div class="section">
    <div class="title">Position</div>
    <button class="btn" onclick="moveToStop()">All to 6h00</button>
  </div>

  <div class="section">
    <div class="title">Move to Position (Clock Convention)</div>
    <div class="inline">
      <label>Board: <select id="pos_board">
        <option value="1">1</option><option value="2">2</option><option value="3">3</option><option value="4">4</option>
        <option value="5">5</option><option value="6">6</option><option value="7">7</option><option value="8">8</option>
      </select></label>
      <label>Clock: <select id="pos_clock">
        <option value="0">0</option><option value="1">1</option><option value="2">2</option>
      </select></label>
      <label>Hand: <select id="pos_hand">
        <option value="H">Hour</option><option value="M">Minute</option>
      </select></label>
    </div>
    <div class="inline">
      <label>Target: <select id="pos_target">
        <option value="0">12h (up)</option>
        <option value="90">3h (right)</option>
        <option value="180">6h (down)</option>
        <option value="270">9h (left)</option>
      </select></label>
      <label>Direction: <select id="pos_dir">
        <option value="CW">Clockwise</option>
        <option value="CCW">Counter-clockwise</option>
        <option value="MIN">Shortest path</option>
      </select></label>
    </div>
    <div style="margin-top:10px;">
      <button class="btn" onclick="moveToPosition()">Move</button>
    </div>
  </div>

  <div class="section">
    <div class="title">Logs</div>
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

    function updateSpeedLabel() {
      document.getElementById('speedVal').textContent = document.getElementById('speed').value;
    }
    function updateAccelLabel() {
      document.getElementById('accelVal').textContent = document.getElementById('accel').value;
    }

    async function applySettings() {
      const speed = document.getElementById('speed').value;
      const accel = document.getElementById('accel').value;
      log('Applying speed=' + speed + ', accel=' + accel, 'info');
      try {
        const res = await fetch('/api/settings', {
          method: 'POST',
          headers: {'Content-Type': 'application/x-www-form-urlencoded'},
          body: 'speed=' + speed + '&accel=' + accel
        });
        const data = await res.json();
        log(data.message, 'ok');
      } catch(e) {
        log('Settings failed: ' + e, 'err');
      }
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
      log('Testing: Board ' + board + ', Clock ' + clock + ', ' + hand + ', ' + direction, 'info');
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
      log('Enabling drivers...', 'info');
      try {
        const res = await fetch('/api/drivers/enable', {method: 'POST'});
        const data = await res.json();
        log(data.message, 'ok');
      } catch(e) {
        log('Enable failed: ' + e, 'err');
      }
    }

    async function disableDrivers() {
      log('Disabling drivers...', 'info');
      try {
        const res = await fetch('/api/drivers/disable', {method: 'POST'});
        const data = await res.json();
        log(data.message, 'ok');
      } catch(e) {
        log('Disable failed: ' + e, 'err');
      }
    }

    async function moveToStop() {
      log('Moving all to 6h00...', 'info');
      try {
        const res = await fetch('/api/stop', {method: 'POST'});
        const data = await res.json();
        log(data.message, 'ok');
      } catch(e) {
        log('Move failed: ' + e, 'err');
      }
    }

    async function moveToPosition() {
      const board = document.getElementById('pos_board').value;
      const clock = document.getElementById('pos_clock').value;
      const hand = document.getElementById('pos_hand').value;
      const target = document.getElementById('pos_target').value;
      const dir = document.getElementById('pos_dir').value;
      const targetLabel = document.getElementById('pos_target').selectedOptions[0].text;
      log('Moving: Board ' + board + ', Clock ' + clock + ', ' + hand + ' to ' + targetLabel + ' (' + dir + ')', 'info');
      try {
        const res = await fetch('/api/motor/position', {
          method: 'POST',
          headers: {'Content-Type': 'application/x-www-form-urlencoded'},
          body: 'board=' + board + '&clock=' + clock + '&hand=' + hand + '&angle=' + target + '&direction=' + dir
        });
        const data = await res.json();
        log(data.message, data.success ? 'ok' : 'err');
      } catch(e) {
        log('Move failed: ' + e, 'err');
      }
    }

    window.onload = () => {
      log('Diagnostics ready', 'ok');
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
  json += ",\"speed\":" + String(_test_speed);
  json += ",\"accel\":" + String(_test_accel);
  json += "}";
  _server.send(200, "application/json", json);
}

void handle_api_settings()
{
  Serial.println("API: Update settings");
  if(_server.hasArg("speed")) {
    _test_speed = _server.arg("speed").toInt();
    if(_test_speed < 200) _test_speed = 200;
    if(_test_speed > 5000) _test_speed = 5000;
  }
  if(_server.hasArg("accel")) {
    _test_accel = _server.arg("accel").toInt();
    if(_test_accel < 100) _test_accel = 100;
    if(_test_accel > 2000) _test_accel = 2000;
  }
  String msg = "Speed=" + String(_test_speed) + ", Accel=" + String(_test_accel);
  _server.send(200, "application/json", "{\"success\":true,\"message\":\"" + msg + "\"}");
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

  // Build half digit command
  t_half_digit hd = {0};

  int current_angle = _motor_positions[board-1][clock_idx][hand];
  int target_angle = (current_angle + 180) % 360;

  for(int i = 0; i < 3; i++) {
    hd.clocks[i].angle_h = _motor_positions[board-1][i][0];
    hd.clocks[i].angle_m = _motor_positions[board-1][i][1];
    hd.clocks[i].speed_h = _test_speed;
    hd.clocks[i].speed_m = _test_speed;
    hd.clocks[i].accel_h = _test_accel;
    hd.clocks[i].accel_m = _test_accel;
    hd.clocks[i].mode_h = MIN_DISTANCE;
    hd.clocks[i].mode_m = MIN_DISTANCE;
    hd.change_counter[i] = _test_counter;
  }

  if(hand == 0) {
    hd.clocks[clock_idx].angle_h = target_angle;
    hd.clocks[clock_idx].mode_h = direction;
    _motor_positions[board-1][clock_idx][0] = target_angle;
  } else {
    hd.clocks[clock_idx].angle_m = target_angle;
    hd.clocks[clock_idx].mode_m = direction;
    _motor_positions[board-1][clock_idx][1] = target_angle;
  }

  hd.change_counter[clock_idx] = ++_test_counter;

  Wire.beginTransmission(board);
  I2C_writeAnything(hd);
  Wire.endTransmission();

  String msg = "Board " + String(board) + ", Clock " + String(clock_idx);
  msg += ", " + String(hand == 0 ? "Hour" : "Minute") + ", " + dir_str;
  _server.send(200, "application/json", "{\"success\":true,\"message\":\"" + msg + "\"}");
}

void handle_api_drivers_enable()
{
  Serial.println("API: Enable drivers");
  set_all_drivers_enabled(true);
  _drivers_enabled = true;
  _server.send(200, "application/json", "{\"success\":true,\"message\":\"Drivers enabled\"}");
}

void handle_api_drivers_disable()
{
  Serial.println("API: Disable drivers");
  set_all_drivers_enabled(false);
  _drivers_enabled = false;
  _server.send(200, "application/json", "{\"success\":true,\"message\":\"Drivers disabled\"}");
}

void handle_api_stop()
{
  Serial.println("API: Move to stop position");
  set_direction(MIN_DISTANCE);
  set_speed(_test_speed);
  set_acceleration(_test_accel);
  set_clock(d_stop);

  for(int b = 0; b < 8; b++)
    for(int c = 0; c < 3; c++)
      for(int h = 0; h < 2; h++)
        _motor_positions[b][c][h] = 270;

  _server.send(200, "application/json", "{\"success\":true,\"message\":\"Moving to 6h00\"}");
}

void handle_api_motor_position()
{
  Serial.println("API: Motor position");

  if(!_server.hasArg("board") || !_server.hasArg("clock") ||
     !_server.hasArg("hand") || !_server.hasArg("angle") || !_server.hasArg("direction")) {
    _server.send(400, "application/json", "{\"success\":false,\"message\":\"Missing parameters\"}");
    return;
  }

  int board = _server.arg("board").toInt();
  int clock_idx = _server.arg("clock").toInt();
  String hand_str = _server.arg("hand");
  int target_angle = _server.arg("angle").toInt();
  String dir_str = _server.arg("direction");

  if(board < 1 || board > 8 || clock_idx < 0 || clock_idx > 2) {
    _server.send(400, "application/json", "{\"success\":false,\"message\":\"Invalid board or clock\"}");
    return;
  }

  int hand = (hand_str == "H" || hand_str == "h") ? 0 : 1;
  int direction;
  if(dir_str == "CW" || dir_str == "cw") {
    direction = CLOCKWISE;
  } else if(dir_str == "CCW" || dir_str == "ccw") {
    direction = COUNTERCLOCKWISE;
  } else {
    direction = MIN_DISTANCE;
  }

  // Build half digit command
  t_half_digit hd = {0};

  for(int i = 0; i < 3; i++) {
    hd.clocks[i].angle_h = _motor_positions[board-1][i][0];
    hd.clocks[i].angle_m = _motor_positions[board-1][i][1];
    hd.clocks[i].speed_h = _test_speed;
    hd.clocks[i].speed_m = _test_speed;
    hd.clocks[i].accel_h = _test_accel;
    hd.clocks[i].accel_m = _test_accel;
    hd.clocks[i].mode_h = MIN_DISTANCE;
    hd.clocks[i].mode_m = MIN_DISTANCE;
    hd.change_counter[i] = _test_counter;
  }

  if(hand == 0) {
    hd.clocks[clock_idx].angle_h = target_angle;
    hd.clocks[clock_idx].mode_h = direction;
    _motor_positions[board-1][clock_idx][0] = target_angle;
  } else {
    hd.clocks[clock_idx].angle_m = target_angle;
    hd.clocks[clock_idx].mode_m = direction;
    _motor_positions[board-1][clock_idx][1] = target_angle;
  }

  hd.change_counter[clock_idx] = ++_test_counter;

  Wire.beginTransmission(board);
  I2C_writeAnything(hd);
  Wire.endTransmission();

  // Map angle to clock position for message
  const char* pos_name;
  switch(target_angle) {
    case 0: pos_name = "12h"; break;
    case 90: pos_name = "3h"; break;
    case 180: pos_name = "6h"; break;
    case 270: pos_name = "9h"; break;
    default: pos_name = "?"; break;
  }

  String msg = "Board " + String(board) + ", Clock " + String(clock_idx);
  msg += ", " + String(hand == 0 ? "Hour" : "Minute") + " to " + String(pos_name);
  _server.send(200, "application/json", "{\"success\":true,\"message\":\"" + msg + "\"}");
}

// ===== CHOREOGRAPHY PAGE AND API HANDLERS =====

const char CHOREO_PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="fr">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>ClockClock 24 - Choreographies</title>
  <style>
    *{margin:0;padding:0;box-sizing:border-box}
    body{font-family:Tahoma,Helvetica,sans-serif;background:#212121;color:#fff;min-height:100vh;padding:20px;user-select:none}
    .container{max-width:800px;margin:0 auto}
    header{display:flex;justify-content:space-between;align-items:center;margin-bottom:24px;flex-wrap:wrap;gap:10px}
    header h1{font-size:1.4rem;font-weight:400}
    .btn-back{color:#8a8a8a;text-decoration:none;font-size:14px;padding:8px 16px;box-shadow:inset 0 0 2px #dfdfdf}
    .btn-back:hover{background:#b4b4b44b}
    button{padding:8px 16px;border:none;background:transparent;color:#fff;cursor:pointer;font-size:14px;box-shadow:inset 0 0 2px #dfdfdf;transition:background 0.2s}
    button:hover{background:#b4b4b44b}
    button.active{background:#dfdfdf;color:#000;box-shadow:inset 0 0 0 2px #dfdfdf}
    button.danger{box-shadow:inset 0 0 2px #c60a0a;color:#ff6b6b}
    button.danger:hover{background:#c60a0a33}
    .section{margin-bottom:24px}
    .section h2{font-size:16px;font-weight:700;margin-bottom:12px;text-align:center}
    .mode-selector{display:flex;gap:8px;flex-wrap:wrap;justify-content:center;margin-bottom:16px}
    .mode-btn{padding:10px 18px;box-shadow:inset 0 0 2px #dfdfdf;background:transparent;color:#fff;cursor:pointer;font-size:14px;border:none}
    .mode-btn:hover{background:#b4b4b44b}
    .mode-btn.active{background:#dfdfdf;color:#000;box-shadow:inset 0 0 0 2px #dfdfdf}
    .mode-desc{font-size:12px;color:#8a8a8a;text-align:center;margin-top:8px}
    .choreo-list{margin-top:12px}
    .choreo-item{display:flex;align-items:center;gap:12px;padding:12px;margin-bottom:6px;box-shadow:inset 0 0 1px #555}
    .choreo-item:hover{background:#2a2a2a}
    .choreo-item input[type="checkbox"]{width:16px;height:16px;cursor:pointer;accent-color:#dfdfdf}
    .choreo-name{flex:1;font-size:14px}
    .choreo-actions{display:flex;gap:6px}
    .choreo-actions button{padding:6px 12px;font-size:12px}
    .upload-zone{border:2px dashed #555;padding:30px;text-align:center;cursor:pointer;transition:all 0.2s}
    .upload-zone:hover{border-color:#8a8a8a;background:#2a2a2a}
    .upload-zone input{display:none}
    .player-section{display:flex;align-items:center;gap:12px;flex-wrap:wrap;padding:16px;justify-content:center;box-shadow:inset 0 0 1px #555}
    .player-btn{padding:10px 20px;font-size:14px}
    .player-status{display:flex;flex-direction:column;gap:4px;align-items:center;min-width:100px}
    .status-badge{padding:4px 12px;font-size:12px;font-weight:500;box-shadow:inset 0 0 2px #555}
    .status-badge.stopped{color:#8a8a8a}
    .status-badge.playing{background:#2e7d32;color:#fff;box-shadow:none}
    .status-badge.paused{background:#f57c00;color:#fff;box-shadow:none}
    .status-name{font-size:12px;color:#8a8a8a}
    .empty-msg{text-align:center;padding:24px;color:#666;font-style:italic;font-size:13px}
    .tabs{display:flex;gap:4px;margin-bottom:20px;justify-content:center}
    .tab{padding:10px 20px;background:transparent;border:none;color:#8a8a8a;cursor:pointer;font-size:14px;box-shadow:inset 0 0 2px #555}
    .tab:hover{color:#fff;background:#b4b4b44b}
    .tab.active{background:#dfdfdf;color:#000;box-shadow:inset 0 0 0 2px #dfdfdf}
    .tab-content{display:none}
    .tab-content.active{display:block}
  </style>
</head>
<body>
  <div class="container">
    <header>
      <h1>Choreographies</h1>
      <a href="/" class="btn-back">Retour</a>
    </header>

    <div class="tabs">
      <button class="tab active" onclick="showTab('manage')">Gestion</button>
      <button class="tab" onclick="showTab('upload')">Upload</button>
    </div>

    <div id="tab-manage" class="tab-content active">
      <div class="section">
        <h2>Mode de lecture automatique</h2>
        <div class="mode-selector">
          <button class="mode-btn" data-mode="off" onclick="setMode('off')">Desactive</button>
          <button class="mode-btn" data-mode="manual" onclick="setMode('manual')">Manuel</button>
          <button class="mode-btn" data-mode="auto" onclick="setMode('auto')">Auto</button>
          <button class="mode-btn" data-mode="random" onclick="setMode('random')">Aleatoire</button>
        </div>
        <div class="mode-desc" id="modeDesc">Selectionnez un mode pour voir sa description.</div>
      </div>

      <div class="section" id="frequencySection" style="display:none;">
        <h2>Frequence (modes Auto/Aleatoire)</h2>
        <div class="mode-selector">
          <button class="mode-btn" data-freq="0" onclick="setFrequency(0)">Horaire</button>
          <button class="mode-btn" data-freq="1" onclick="setFrequency(1)">2/min</button>
        </div>
        <div class="mode-desc" id="freqDesc">Horaire: la choreographie se joue a chaque changement d'heure.</div>
      </div>

      <div class="section">
        <h2>Choreographies disponibles</h2>
        <div id="choreoList" class="choreo-list">
          <div class="empty-msg">Aucune choreographie. Uploadez un fichier JSON.</div>
        </div>
      </div>

      <div class="section">
        <h2>Lecture</h2>
        <div class="player-section">
          <button class="player-btn secondary" onclick="hwPrev()">Precedent</button>
          <button class="player-btn" onclick="hwPlay()" id="playBtn">Lecture</button>
          <button class="player-btn secondary" onclick="hwPause()">Pause</button>
          <button class="player-btn danger" onclick="hwStop()">Stop</button>
          <button class="player-btn secondary" onclick="hwNext()">Suivant</button>
          <div class="player-status">
            <span class="status-badge stopped" id="statusBadge">Arrete</span>
            <span class="status-name" id="statusName">-</span>
          </div>
        </div>
      </div>
    </div>

    <div id="tab-upload" class="tab-content">
      <div class="section">
        <h2>Uploader une choreographie</h2>
        <p style="color:#888;margin-bottom:15px;font-size:0.9rem">
          Creez vos choreographies avec le designer local (ChoregraphieDesigner/index.html) puis uploadez le fichier JSON ici.
        </p>
        <div class="upload-zone" onclick="document.getElementById('uploadFile').click()">
          <input type="file" id="uploadFile" accept=".json" onchange="uploadFile(event)">
          <p style="font-size:1.1rem;margin-bottom:8px">Cliquez ou glissez un fichier JSON</p>
          <p style="color:#666;font-size:0.85rem">Format: export du Choreographie Designer</p>
        </div>
      </div>
    </div>
  </div>

<script>
let currentMode='off';
let currentFrequency=0;
let choreographies=[];

const modeDescriptions={
  off:'Desactive: Les choreographies ne se declenchent pas automatiquement.',
  manual:'Manuel: Declenchez les choreographies manuellement via les boutons de lecture.',
  auto:'Auto: La premiere choreographie activee se joue selon la frequence choisie.',
  random:'Aleatoire: Une choreographie au hasard parmi celles activees se joue selon la frequence choisie.'
};

const freqDescriptions={
  0:'Horaire: la choreographie se joue a chaque changement d\'heure.',
  1:'2/min: la choreographie se joue toutes les 30 secondes (2 fois par minute).'
};

document.addEventListener('DOMContentLoaded',()=>{loadData();updateStatus();});

function showTab(tab){
  document.querySelectorAll('.tab').forEach(t=>t.classList.remove('active'));
  document.querySelectorAll('.tab-content').forEach(c=>c.classList.remove('active'));
  document.querySelector(`.tab[onclick="showTab('${tab}')"]`).classList.add('active');
  document.getElementById('tab-'+tab).classList.add('active');
}

async function loadData(){
  try{
    const res=await fetch('/api/choreo/listfull');
    const data=await res.json();
    currentMode=data.mode||'off';
    currentFrequency=data.frequency||0;
    choreographies=data.choreographies||[];
    renderMode();
    renderFrequency();
    renderList();
  }catch(e){console.error('Load error:',e);}
}

function renderMode(){
  document.querySelectorAll('.mode-btn[data-mode]').forEach(btn=>{
    btn.classList.toggle('active',btn.dataset.mode===currentMode);
  });
  document.getElementById('modeDesc').textContent=modeDescriptions[currentMode]||'';
  // Show frequency section only for auto/random modes
  const freqSection=document.getElementById('frequencySection');
  freqSection.style.display=(currentMode==='auto'||currentMode==='random')?'block':'none';
}

function renderFrequency(){
  document.querySelectorAll('.mode-btn[data-freq]').forEach(btn=>{
    btn.classList.toggle('active',parseInt(btn.dataset.freq)===currentFrequency);
  });
  document.getElementById('freqDesc').textContent=freqDescriptions[currentFrequency]||'';
}

function renderList(){
  const list=document.getElementById('choreoList');
  if(choreographies.length===0){
    list.innerHTML='<div class="empty-msg">Aucune choreographie. Uploadez un fichier JSON.</div>';
    return;
  }
  list.innerHTML='';
  choreographies.forEach(ch=>{
    const item=document.createElement('div');
    item.className='choreo-item';
    item.innerHTML=`
      <input type="checkbox" ${ch.enabled?'checked':''} onchange="toggleEnabled('${ch.name}',this.checked)" title="Activer pour mode auto/random">
      <span class="choreo-name">${ch.name}</span>
      <div class="choreo-actions">
        <button class="secondary" onclick="loadAndPlay('${ch.name}')">Jouer</button>
        <button class="danger" onclick="deleteChoreography('${ch.name}')">Suppr</button>
      </div>`;
    list.appendChild(item);
  });
}

async function setMode(mode){
  try{
    await fetch('/api/choreo/mode',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:'mode='+mode});
    currentMode=mode;
    renderMode();
  }catch(e){alert('Erreur: '+e);}
}

async function setFrequency(freq){
  try{
    await fetch('/api/choreo/frequency',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:'frequency='+freq});
    currentFrequency=freq;
    renderFrequency();
  }catch(e){alert('Erreur: '+e);}
}

async function toggleEnabled(name,enabled){
  try{
    await fetch('/api/choreo/enable',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:'name='+encodeURIComponent(name)+'&enabled='+(enabled?'true':'false')});
    const ch=choreographies.find(c=>c.name===name);
    if(ch)ch.enabled=enabled;
  }catch(e){alert('Erreur: '+e);}
}

async function loadAndPlay(name){
  try{
    await fetch('/api/choreo/load?name='+encodeURIComponent(name));
    await fetch('/api/choreo/play',{method:'POST'});
    updateStatus();
  }catch(e){alert('Erreur: '+e);}
}

async function deleteChoreography(name){
  if(!confirm('Supprimer "'+name+'" ?'))return;
  try{
    await fetch('/api/choreo/delete?name='+encodeURIComponent(name),{method:'POST'});
    loadData();
  }catch(e){alert('Erreur: '+e);}
}

async function uploadFile(e){
  const file=e.target.files[0];
  if(!file)return;
  const reader=new FileReader();
  reader.onload=async(ev)=>{
    try{
      const data=JSON.parse(ev.target.result);
      if(!data.keyframes||!data.name){alert('Format invalide');return;}
      const res=await fetch('/api/choreo/save',{method:'POST',headers:{'Content-Type':'application/json'},body:ev.target.result});
      const result=await res.json();
      if(result.success){alert('Upload reussi: '+data.name);loadData();showTab('manage');}
      else alert('Erreur: '+result.message);
    }catch(err){alert('Erreur: '+err);}
  };
  reader.readAsText(file);
  e.target.value='';
}

async function hwPlay(){try{await fetch('/api/choreo/play',{method:'POST'});updateStatus();}catch(e){}}
async function hwPause(){try{await fetch('/api/choreo/pause',{method:'POST'});updateStatus();}catch(e){}}
async function hwStop(){try{await fetch('/api/choreo/stop',{method:'POST'});updateStatus();}catch(e){}}
async function hwNext(){try{await fetch('/api/choreo/next',{method:'POST'});updateStatus();}catch(e){}}
async function hwPrev(){try{await fetch('/api/choreo/prev',{method:'POST'});updateStatus();}catch(e){}}

async function updateStatus(){
  try{
    const res=await fetch('/api/choreo/status');
    const data=await res.json();
    const badge=document.getElementById('statusBadge');
    const name=document.getElementById('statusName');
    const stateMap={Playing:'En cours',Paused:'Pause',Stopped:'Arrete'};
    badge.textContent=stateMap[data.state]||data.state;
    badge.className='status-badge '+data.state.toLowerCase();
    name.textContent=data.name||'-';
  }catch(e){}
}
setInterval(updateStatus,2000);
</script>
</body>
</html>
)rawliteral";

void handle_get_choreography() {
  Serial.println("Handle GET /choreography");
  _server.send(200, "text/html", CHOREO_PAGE);
}

void handle_api_choreo_list() {
  Serial.println("API: List choreographies");
  char names[MAX_CHOREOGRAPHIES][CHOREO_NAME_LEN];
  int count = choreo_list(names, MAX_CHOREOGRAPHIES);

  String json = "{\"choreographies\":[";
  for (int i = 0; i < count; i++) {
    if (i > 0) json += ",";
    json += "\"" + String(names[i]) + "\"";
  }
  json += "]}";

  _server.send(200, "application/json", json);
}

void handle_api_choreo_load() {
  Serial.println("API: Load choreography");

  if (!_server.hasArg("name")) {
    _server.send(400, "application/json", "{\"success\":false,\"message\":\"Missing name\"}");
    return;
  }

  String name = _server.arg("name");
  if (choreo_load(name.c_str())) {
    String json = "{\"success\":true,\"choreography\":" + choreo_get_json() + "}";
    _server.send(200, "application/json", json);
  } else {
    _server.send(404, "application/json", "{\"success\":false,\"message\":\"Not found\"}");
  }
}

void handle_api_choreo_save() {
  Serial.println("API: Save choreography");

  if (_server.hasArg("plain")) {
    String body = _server.arg("plain");
    if (choreo_save(body.c_str())) {
      _server.send(200, "application/json", "{\"success\":true}");
    } else {
      _server.send(500, "application/json", "{\"success\":false,\"message\":\"Save failed\"}");
    }
  } else {
    _server.send(400, "application/json", "{\"success\":false,\"message\":\"No data\"}");
  }
}

void handle_api_choreo_delete() {
  Serial.println("API: Delete choreography");

  if (!_server.hasArg("name")) {
    _server.send(400, "application/json", "{\"success\":false,\"message\":\"Missing name\"}");
    return;
  }

  String name = _server.arg("name");
  if (choreo_delete(name.c_str())) {
    _server.send(200, "application/json", "{\"success\":true}");
  } else {
    _server.send(500, "application/json", "{\"success\":false,\"message\":\"Delete failed\"}");
  }
}

void handle_api_choreo_play() {
  Serial.println("API: Play choreography");
  choreo_play();
  _server.send(200, "application/json", "{\"success\":true}");
}

void handle_api_choreo_pause() {
  Serial.println("API: Pause choreography");
  choreo_pause();
  _server.send(200, "application/json", "{\"success\":true}");
}

void handle_api_choreo_stop() {
  Serial.println("API: Stop choreography");
  choreo_stop();
  _server.send(200, "application/json", "{\"success\":true}");
}

void handle_api_choreo_next() {
  Serial.println("API: Next keyframe");
  choreo_next_keyframe();
  _server.send(200, "application/json", "{\"success\":true,\"keyframe\":" + String(choreo_get_current_keyframe()) + "}");
}

void handle_api_choreo_prev() {
  Serial.println("API: Previous keyframe");
  choreo_prev_keyframe();
  _server.send(200, "application/json", "{\"success\":true,\"keyframe\":" + String(choreo_get_current_keyframe()) + "}");
}

void handle_api_choreo_status() {
  t_choreo_state state = choreo_get_state();
  const char* stateStr;
  switch (state) {
    case CHOREO_PLAYING: stateStr = "Playing"; break;
    case CHOREO_PAUSED: stateStr = "Paused"; break;
    default: stateStr = "Stopped"; break;
  }

  String json = "{\"state\":\"" + String(stateStr) + "\",";
  json += "\"keyframe\":" + String(choreo_get_current_keyframe()) + ",";
  json += "\"name\":\"" + String(choreo_get_current_name()) + "\"}";

  _server.send(200, "application/json", json);
}

void handle_api_choreo_apply() {
  Serial.println("API: Apply keyframe");

  int kf = 0;
  if (_server.hasArg("keyframe")) {
    kf = _server.arg("keyframe").toInt();
  }

  choreo_apply_keyframe(kf);
  _server.send(200, "application/json", "{\"success\":true}");
}

// ===== CHOREOGRAPHY MODE API =====

void handle_api_choreo_mode_get() {
  t_choreo_mode mode = choreo_get_mode();
  const char* modeStr;
  switch (mode) {
    case CHOREO_MODE_MANUAL: modeStr = "manual"; break;
    case CHOREO_MODE_AUTO: modeStr = "auto"; break;
    case CHOREO_MODE_RANDOM: modeStr = "random"; break;
    default: modeStr = "off"; break;
  }

  String json = "{\"mode\":\"" + String(modeStr) + "\"}";
  _server.send(200, "application/json", json);
}

void handle_api_choreo_mode_set() {
  Serial.println("API: Set choreography mode");

  if (!_server.hasArg("mode")) {
    _server.send(400, "application/json", "{\"success\":false,\"message\":\"Missing mode\"}");
    return;
  }

  String mode = _server.arg("mode");
  t_choreo_mode m = CHOREO_MODE_OFF;

  if (mode == "manual") m = CHOREO_MODE_MANUAL;
  else if (mode == "auto") m = CHOREO_MODE_AUTO;
  else if (mode == "random") m = CHOREO_MODE_RANDOM;

  choreo_set_mode(m);
  _server.send(200, "application/json", "{\"success\":true}");
}

void handle_api_choreo_enable() {
  Serial.println("API: Enable/disable choreography");

  if (!_server.hasArg("name") || !_server.hasArg("enabled")) {
    _server.send(400, "application/json", "{\"success\":false,\"message\":\"Missing params\"}");
    return;
  }

  String name = _server.arg("name");
  bool enabled = (_server.arg("enabled") == "true" || _server.arg("enabled") == "1");

  choreo_set_enabled(name.c_str(), enabled);
  _server.send(200, "application/json", "{\"success\":true}");
}

void handle_api_choreo_list_full() {
  Serial.println("API: List choreographies with enabled status");
  char names[MAX_CHOREOGRAPHIES][CHOREO_NAME_LEN];
  int count = choreo_list(names, MAX_CHOREOGRAPHIES);

  String json = "{\"choreographies\":[";
  for (int i = 0; i < count; i++) {
    if (i > 0) json += ",";
    bool enabled = choreo_is_enabled(names[i]);
    json += "{\"name\":\"" + String(names[i]) + "\",\"enabled\":" + (enabled ? "true" : "false") + "}";
  }
  json += "],\"mode\":\"";

  t_choreo_mode mode = choreo_get_mode();
  switch (mode) {
    case CHOREO_MODE_MANUAL: json += "manual"; break;
    case CHOREO_MODE_AUTO: json += "auto"; break;
    case CHOREO_MODE_RANDOM: json += "random"; break;
    default: json += "off"; break;
  }
  json += "\",\"frequency\":" + String(get_choreo_frequency());
  json += ",\"count\":" + String(count) + "}";

  _server.send(200, "application/json", json);
}

void handle_api_choreo_frequency_get() {
  int freq = get_choreo_frequency();
  String json = "{\"frequency\":" + String(freq) + "}";
  _server.send(200, "application/json", json);
}

void handle_api_choreo_frequency_set() {
  Serial.println("API: Set choreography frequency");

  if (!_server.hasArg("frequency")) {
    _server.send(400, "application/json", "{\"success\":false,\"message\":\"Missing frequency\"}");
    return;
  }

  int freq = _server.arg("frequency").toInt();
  if (freq < 0 || freq > 1) {
    _server.send(400, "application/json", "{\"success\":false,\"message\":\"Invalid frequency (0=hourly, 1=2/min)\"}");
    return;
  }

  set_choreo_frequency(freq);
  Serial.printf("Choreography frequency set to: %d\n", freq);
  _server.send(200, "application/json", "{\"success\":true}");
}