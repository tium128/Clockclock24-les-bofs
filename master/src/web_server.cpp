#include "web_server.h"
#include <Wire.h>
#include "i2c.h"
#include "digit.h"
#include "choreography.h"

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
  <title>ClockClock24 - Choreography Designer</title>
  <style>
    *{margin:0;padding:0;box-sizing:border-box}
    body{font-family:-apple-system,BlinkMacSystemFont,'Segoe UI',Roboto,sans-serif;background:#1a1a2e;color:#eee;min-height:100vh;padding:15px}
    .container{max-width:1400px;margin:0 auto}
    header{display:flex;justify-content:space-between;align-items:center;margin-bottom:15px;padding-bottom:15px;border-bottom:1px solid #333;flex-wrap:wrap;gap:10px}
    header h1{font-size:1.3rem;color:#4ecdc4}
    .controls{display:flex;gap:8px;align-items:center;flex-wrap:wrap}
    .controls input[type="text"],.controls select{padding:6px 10px;border:1px solid #444;border-radius:4px;background:#2a2a4a;color:#fff;font-size:0.9rem}
    button{padding:6px 12px;border:none;border-radius:4px;background:#4ecdc4;color:#1a1a2e;cursor:pointer;font-weight:500;font-size:0.85rem;transition:all 0.2s}
    button:hover{background:#45b7aa}
    button.danger{background:#e74c3c;color:#fff}
    button.danger:hover{background:#c0392b}
    .btn-back{position:fixed;top:15px;left:15px;background:#3a3a5a;color:#fff;text-decoration:none;padding:8px 12px;border-radius:4px;font-size:0.85rem}
    .section{background:#2a2a4a;border-radius:8px;padding:15px;margin-bottom:15px}
    .section h2{font-size:1rem;color:#888;margin-bottom:10px}
    .timeline{display:flex;gap:8px;overflow-x:auto;padding:8px 0}
    .kf-thumb{min-width:80px;height:50px;background:#3a3a5a;border-radius:6px;cursor:pointer;display:flex;flex-direction:column;align-items:center;justify-content:center;border:2px solid transparent;font-size:0.8rem}
    .kf-thumb:hover{background:#4a4a6a}
    .kf-thumb.active{border-color:#4ecdc4;background:#3a4a5a}
    .kf-thumb span{font-size:0.7rem;color:#888}
    .matrix{display:grid;grid-template-columns:50px repeat(8,1fr);gap:6px}
    .row-label{display:flex;align-items:center;justify-content:center;font-size:0.75rem;color:#888}
    .slave-labels{display:grid;grid-template-columns:50px repeat(8,1fr);gap:6px;margin-bottom:6px;font-size:0.75rem;color:#888;text-align:center}
    .clock-cell{background:#1a1a2e;border-radius:6px;padding:8px;display:flex;flex-direction:column;align-items:center;gap:6px;border:2px solid transparent;cursor:pointer}
    .clock-cell:hover{background:#252545}
    .clock-cell.selected{border-color:#4ecdc4}
    .clock-face{width:50px;height:50px;border-radius:50%;background:#fff;position:relative;border:2px solid #333}
    .clock-face::before{content:'';position:absolute;top:50%;left:50%;width:5px;height:5px;background:#333;border-radius:50%;transform:translate(-50%,-50%);z-index:10}
    .hand{position:absolute;bottom:50%;left:50%;transform-origin:bottom center;border-radius:2px}
    .hand-h{width:3px;height:14px;background:#e74c3c;margin-left:-1.5px}
    .hand-m{width:2px;height:20px;background:#3498db;margin-left:-1px}
    .clock-inputs{display:grid;grid-template-columns:1fr 1fr;gap:3px;width:100%}
    .input-group{display:flex;flex-direction:column;gap:2px}
    .input-group label{font-size:0.6rem;color:#888;text-align:center}
    .input-group input,.input-group select{width:100%;padding:3px;border:1px solid #444;border-radius:3px;background:#2a2a4a;color:#fff;text-align:center;font-size:0.75rem}
    .input-h input,.input-h select{border-color:#e74c3c}
    .input-m input,.input-m select{border-color:#3498db}
    .comment-section{margin-top:15px}
    .comment-section textarea{width:100%;height:60px;padding:8px;border:1px solid #444;border-radius:4px;background:#1a1a2e;color:#fff;resize:vertical;font-family:inherit;font-size:0.85rem}
    .sidebar{display:grid;grid-template-columns:repeat(auto-fit,minmax(200px,1fr));gap:15px}
    .action-group{background:#2a2a4a;border-radius:8px;padding:12px}
    .action-group h4{font-size:0.8rem;color:#4ecdc4;margin-bottom:8px}
    .action-group button{display:block;width:100%;margin-bottom:5px;background:#3a3a5a;color:#fff}
    .action-group button:hover{background:#4a4a6a}
    .player-controls{display:flex;gap:8px;align-items:center;flex-wrap:wrap;margin-bottom:10px}
    .status{padding:4px 8px;border-radius:4px;font-size:0.8rem}
    .status.stopped{background:#666}
    .status.playing{background:#27ae60}
    .status.paused{background:#f39c12}
    #previewCanvas{background:#1a1a2e;border-radius:8px;width:100%;max-width:700px}
    .timing-inputs{display:flex;gap:15px;margin-top:10px;flex-wrap:wrap}
    .timing-inputs label{display:flex;align-items:center;gap:8px;font-size:0.85rem;color:#888}
    .timing-inputs input{width:80px;padding:4px;border:1px solid #444;border-radius:4px;background:#2a2a4a;color:#fff;text-align:center}
    @media(max-width:900px){.matrix{grid-template-columns:40px repeat(8,1fr);gap:4px}.clock-face{width:35px;height:35px}.hand-h{height:10px}.hand-m{height:14px}.clock-inputs{display:none}}
  </style>
</head>
<body>
  <a href="/" class="btn-back">< Back</a>
  <div class="container">
    <header>
      <h1>Choreography Designer</h1>
      <div class="controls">
        <select id="choreoSelect" onchange="loadSelected()"><option value="">-- New --</option></select>
        <input type="text" id="projectName" placeholder="Name" value="new_choreo" style="width:150px">
        <button onclick="saveProject()">Save</button>
        <button onclick="exportJSON()">Export</button>
        <button onclick="document.getElementById('importFile').click()">Import</button>
        <input type="file" id="importFile" accept=".json" style="display:none" onchange="importJSON(event)">
        <button class="danger" onclick="deleteProject()">Delete</button>
      </div>
    </header>

    <div class="section">
      <h2>Keyframes</h2>
      <div style="display:flex;gap:8px;margin-bottom:8px">
        <button onclick="addKeyframe()">+ Add</button>
        <button onclick="duplicateKeyframe()">Duplicate</button>
        <button onclick="deleteKeyframe()" class="danger">Delete</button>
      </div>
      <div class="timeline" id="timeline"></div>
    </div>

    <div class="section">
      <h2>Keyframe <span id="kfNum">1</span></h2>
      <div class="slave-labels">
        <span></span><span>S1</span><span>S2</span><span>S3</span><span>S4</span><span>S5</span><span>S6</span><span>S7</span><span>S8</span>
      </div>
      <div class="matrix" id="matrix"></div>
      <div class="timing-inputs">
        <label>Transition: <input type="number" id="transitionMs" value="1000" min="100" max="10000" step="100" onchange="saveTransition()">ms</label>
        <label>Delay: <input type="number" id="delayMs" value="0" min="0" max="10000" step="100" onchange="saveDelay()">ms</label>
        <label><input type="checkbox" id="loopCheck" onchange="saveLoop()"> Loop</label>
      </div>
      <div class="comment-section">
        <textarea id="comment" placeholder="Instructions (e.g., trigger columns with 500ms offset)" onchange="saveComment()"></textarea>
      </div>
    </div>

    <div class="sidebar">
      <div class="action-group">
        <h4>Presets</h4>
        <button onclick="setAll(0,0)">12h (0/0)</button>
        <button onclick="setAll(90,90)">3h (90/90)</button>
        <button onclick="setAll(180,180)">6h (180/180)</button>
        <button onclick="setAll(270,270)">9h (270/270)</button>
        <button onclick="setAll(135,315)">Diagonal</button>
      </div>
      <div class="action-group">
        <h4>Direction</h4>
        <button onclick="setDir('CW','CW')">All CW</button>
        <button onclick="setDir('CCW','CCW')">All CCW</button>
        <button onclick="setDir('CW','CCW')">H:CW M:CCW</button>
      </div>
      <div class="action-group">
        <h4>Selection</h4>
        <button onclick="selectAll()">Select All</button>
        <button onclick="selectNone()">Deselect</button>
        <button onclick="selectCol()">Select Column</button>
        <button onclick="selectRow()">Select Row</button>
      </div>
      <div class="action-group">
        <h4>Hardware Control</h4>
        <div class="player-controls">
          <button onclick="hwPrev()">Prev</button>
          <button onclick="hwPlay()" id="playBtn">Play</button>
          <button onclick="hwStop()">Stop</button>
          <button onclick="hwNext()">Next</button>
          <span class="status stopped" id="hwStatus">Stopped</span>
        </div>
        <button onclick="hwApply()">Apply Current KF</button>
      </div>
    </div>

    <div class="section">
      <h2>Preview</h2>
      <canvas id="previewCanvas" width="700" height="200"></canvas>
    </div>
  </div>

<script>
let keyframes=[];
let currentKF=0;
let selected=new Set();
let isPlaying=false;

document.addEventListener('DOMContentLoaded',()=>{addKeyframe();loadList();render();});

function createKF(){
  const kf={id:Date.now(),comment:'',transitionMs:1000,delayMs:0,clocks:[]};
  for(let s=0;s<8;s++){kf.clocks[s]=[];for(let c=0;c<3;c++)kf.clocks[s][c]={angleH:180,angleM:180,dirH:'CW',dirM:'CW'};}
  return kf;
}

function addKeyframe(){keyframes.push(createKF());currentKF=keyframes.length-1;render();}
function duplicateKeyframe(){if(!keyframes.length)return;const d=JSON.parse(JSON.stringify(keyframes[currentKF]));d.id=Date.now();keyframes.splice(currentKF+1,0,d);currentKF++;render();}
function deleteKeyframe(){if(keyframes.length<=1){alert('Need at least 1 keyframe');return;}keyframes.splice(currentKF,1);if(currentKF>=keyframes.length)currentKF=keyframes.length-1;render();}
function selectKF(i){currentKF=i;render();}

function render(){
  const tl=document.getElementById('timeline');
  tl.innerHTML='';
  keyframes.forEach((k,i)=>{
    const d=document.createElement('div');
    d.className='kf-thumb'+(i===currentKF?' active':'');
    d.innerHTML='<strong>KF '+(i+1)+'</strong><span>'+(k.comment?k.comment.substring(0,12):'')+'</span>';
    d.onclick=()=>selectKF(i);
    tl.appendChild(d);
  });

  const kf=keyframes[currentKF];
  document.getElementById('kfNum').textContent=currentKF+1;
  document.getElementById('comment').value=kf.comment||'';
  document.getElementById('transitionMs').value=kf.transitionMs||1000;
  document.getElementById('delayMs').value=kf.delayMs||0;

  const m=document.getElementById('matrix');
  m.innerHTML='';
  for(let r=0;r<3;r++){
    const rl=document.createElement('div');rl.className='row-label';rl.textContent='C'+r;m.appendChild(rl);
    for(let s=0;s<8;s++){
      const cl=kf.clocks[s][r];const id=s+'-'+r;const sel=selected.has(id);
      const cell=document.createElement('div');
      cell.className='clock-cell'+(sel?' selected':'');
      cell.innerHTML=`
        <div class="clock-face">
          <div class="hand hand-h" style="transform:rotate(${cl.angleH}deg)"></div>
          <div class="hand hand-m" style="transform:rotate(${cl.angleM}deg)"></div>
        </div>
        <div class="clock-inputs">
          <div class="input-group input-h">
            <label>H</label>
            <input type="number" min="0" max="359" value="${cl.angleH}" onchange="updAngle(${s},${r},'H',this.value)">
            <select onchange="updDir(${s},${r},'H',this.value)">
              <option value="CW"${cl.dirH==='CW'?' selected':''}>CW</option>
              <option value="CCW"${cl.dirH==='CCW'?' selected':''}>CCW</option>
            </select>
          </div>
          <div class="input-group input-m">
            <label>M</label>
            <input type="number" min="0" max="359" value="${cl.angleM}" onchange="updAngle(${s},${r},'M',this.value)">
            <select onchange="updDir(${s},${r},'M',this.value)">
              <option value="CW"${cl.dirM==='CW'?' selected':''}>CW</option>
              <option value="CCW"${cl.dirM==='CCW'?' selected':''}>CCW</option>
            </select>
          </div>
        </div>`;
      cell.onclick=(e)=>{if(e.target.tagName==='INPUT'||e.target.tagName==='SELECT')return;toggleSel(s,r,e.shiftKey);};
      m.appendChild(cell);
    }
  }
  drawPreview();
}

function updAngle(s,c,h,v){const a=((parseInt(v)||0)%360+360)%360;if(h==='H')keyframes[currentKF].clocks[s][c].angleH=a;else keyframes[currentKF].clocks[s][c].angleM=a;render();}
function updDir(s,c,h,v){if(h==='H')keyframes[currentKF].clocks[s][c].dirH=v;else keyframes[currentKF].clocks[s][c].dirM=v;}
function saveComment(){keyframes[currentKF].comment=document.getElementById('comment').value;render();}
function saveTransition(){keyframes[currentKF].transitionMs=parseInt(document.getElementById('transitionMs').value)||1000;}
function saveDelay(){keyframes[currentKF].delayMs=parseInt(document.getElementById('delayMs').value)||0;}
function saveLoop(){/* stored in project */}

function toggleSel(s,c,add){const id=s+'-'+c;if(!add)selected.clear();if(selected.has(id))selected.delete(id);else selected.add(id);render();}
function selectAll(){selected.clear();for(let s=0;s<8;s++)for(let c=0;c<3;c++)selected.add(s+'-'+c);render();}
function selectNone(){selected.clear();render();}
function selectCol(){const col=prompt('Slave (1-8):');const s=parseInt(col)-1;if(isNaN(s)||s<0||s>7)return;selected.clear();for(let c=0;c<3;c++)selected.add(s+'-'+c);render();}
function selectRow(){const row=prompt('Clock (0-2):');const c=parseInt(row);if(isNaN(c)||c<0||c>2)return;selected.clear();for(let s=0;s<8;s++)selected.add(s+'-'+c);render();}

function setAll(aH,aM){const t=selected.size>0?selected:getAllIds();t.forEach(id=>{const[s,c]=id.split('-').map(Number);keyframes[currentKF].clocks[s][c].angleH=aH;keyframes[currentKF].clocks[s][c].angleM=aM;});render();}
function setDir(dH,dM){const t=selected.size>0?selected:getAllIds();t.forEach(id=>{const[s,c]=id.split('-').map(Number);keyframes[currentKF].clocks[s][c].dirH=dH;keyframes[currentKF].clocks[s][c].dirM=dM;});render();}
function getAllIds(){const ids=new Set();for(let s=0;s<8;s++)for(let c=0;c<3;c++)ids.add(s+'-'+c);return ids;}

function drawPreview(){
  const canvas=document.getElementById('previewCanvas');
  const ctx=canvas.getContext('2d');
  const kf=keyframes[currentKF];
  ctx.fillStyle='#1a1a2e';ctx.fillRect(0,0,canvas.width,canvas.height);
  const sz=22,pad=6,sx=35,sy=25;
  for(let s=0;s<8;s++){
    for(let c=0;c<3;c++){
      const x=sx+s*(sz*2+pad),y=sy+c*(sz*2+pad);
      const cl=kf.clocks[s][c];
      ctx.beginPath();ctx.arc(x+sz,y+sz,sz,0,Math.PI*2);ctx.fillStyle='#fff';ctx.fill();ctx.strokeStyle='#333';ctx.lineWidth=2;ctx.stroke();
      const hRad=(cl.angleH-90)*Math.PI/180;
      ctx.beginPath();ctx.moveTo(x+sz,y+sz);ctx.lineTo(x+sz+Math.cos(hRad)*sz*0.5,y+sz+Math.sin(hRad)*sz*0.5);ctx.strokeStyle='#e74c3c';ctx.lineWidth=3;ctx.stroke();
      const mRad=(cl.angleM-90)*Math.PI/180;
      ctx.beginPath();ctx.moveTo(x+sz,y+sz);ctx.lineTo(x+sz+Math.cos(mRad)*sz*0.7,y+sz+Math.sin(mRad)*sz*0.7);ctx.strokeStyle='#3498db';ctx.lineWidth=2;ctx.stroke();
      ctx.beginPath();ctx.arc(x+sz,y+sz,3,0,Math.PI*2);ctx.fillStyle='#333';ctx.fill();
    }
  }
  ctx.fillStyle='#888';ctx.font='10px sans-serif';ctx.textAlign='center';
  for(let s=0;s<8;s++)ctx.fillText('S'+(s+1),sx+s*(sz*2+pad)+sz,15);
  ctx.textAlign='right';
  for(let c=0;c<3;c++)ctx.fillText('C'+c,28,sy+c*(sz*2+pad)+sz+4);
}

async function loadList(){
  try{
    const res=await fetch('/api/choreo/list');
    const data=await res.json();
    const sel=document.getElementById('choreoSelect');
    sel.innerHTML='<option value="">-- New --</option>';
    data.choreographies.forEach(n=>{const o=document.createElement('option');o.value=n;o.textContent=n;sel.appendChild(o);});
  }catch(e){console.error('List error:',e);}
}

async function loadSelected(){
  const name=document.getElementById('choreoSelect').value;
  if(!name){keyframes=[createKF()];currentKF=0;document.getElementById('projectName').value='new_choreo';render();return;}
  try{
    const res=await fetch('/api/choreo/load?name='+encodeURIComponent(name));
    const data=await res.json();
    if(data.success&&data.choreography){
      const ch=data.choreography;
      document.getElementById('projectName').value=ch.name;
      document.getElementById('loopCheck').checked=ch.loop||false;
      keyframes=ch.keyframes||[createKF()];
      currentKF=0;render();
    }
  }catch(e){console.error('Load error:',e);}
}

async function saveProject(){
  const name=document.getElementById('projectName').value||'unnamed';
  const loop=document.getElementById('loopCheck').checked;
  const proj={name,loop,keyframes};
  try{
    const res=await fetch('/api/choreo/save',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify(proj)});
    const data=await res.json();
    alert(data.success?'Saved!':'Error: '+data.message);
    loadList();
  }catch(e){alert('Save error:'+e);}
}

async function deleteProject(){
  const name=document.getElementById('choreoSelect').value;
  if(!name){alert('Select a choreography first');return;}
  if(!confirm('Delete '+name+'?'))return;
  try{
    const res=await fetch('/api/choreo/delete?name='+encodeURIComponent(name),{method:'POST'});
    const data=await res.json();
    alert(data.success?'Deleted':'Error');
    loadList();
    keyframes=[createKF()];currentKF=0;document.getElementById('projectName').value='new_choreo';render();
  }catch(e){alert('Delete error:'+e);}
}

function exportJSON(){
  const name=document.getElementById('projectName').value||'choreo';
  const loop=document.getElementById('loopCheck').checked;
  const proj={name,loop,keyframes,version:'1.0',created:new Date().toISOString()};
  const blob=new Blob([JSON.stringify(proj,null,2)],{type:'application/json'});
  const a=document.createElement('a');a.href=URL.createObjectURL(blob);a.download=name+'.json';a.click();
}

function importJSON(e){
  const file=e.target.files[0];if(!file)return;
  const reader=new FileReader();
  reader.onload=(ev)=>{
    try{
      const data=JSON.parse(ev.target.result);
      if(data.keyframes){
        keyframes=data.keyframes;currentKF=0;
        document.getElementById('projectName').value=data.name||'imported';
        document.getElementById('loopCheck').checked=data.loop||false;
        render();alert('Imported!');
      }
    }catch(err){alert('Import error:'+err);}
  };
  reader.readAsText(file);e.target.value='';
}

async function hwPlay(){
  try{await fetch('/api/choreo/play',{method:'POST'});updateStatus();}catch(e){console.error(e);}
}
async function hwStop(){
  try{await fetch('/api/choreo/stop',{method:'POST'});updateStatus();}catch(e){console.error(e);}
}
async function hwNext(){
  try{await fetch('/api/choreo/next',{method:'POST'});updateStatus();}catch(e){console.error(e);}
}
async function hwPrev(){
  try{await fetch('/api/choreo/prev',{method:'POST'});updateStatus();}catch(e){console.error(e);}
}
async function hwApply(){
  const name=document.getElementById('projectName').value;
  await saveProject();
  try{
    await fetch('/api/choreo/load?name='+encodeURIComponent(name));
    await fetch('/api/choreo/apply?keyframe='+currentKF,{method:'POST'});
    alert('Applied keyframe '+(currentKF+1));
  }catch(e){alert('Error:'+e);}
}

async function updateStatus(){
  try{
    const res=await fetch('/api/choreo/status');
    const data=await res.json();
    const st=document.getElementById('hwStatus');
    st.textContent=data.state;
    st.className='status '+data.state.toLowerCase();
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