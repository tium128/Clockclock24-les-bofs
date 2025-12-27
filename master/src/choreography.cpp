#include "choreography.h"
#include "clock_manager.h"
#include <LittleFS.h>
#include <ArduinoJson.h>

// Current choreography
static t_choreography _currentChoreo;
static bool _choreoLoaded = false;

// Player state
static t_choreo_state _state = CHOREO_STOPPED;
static int _currentKeyframe = 0;
static unsigned long _transitionStartTime = 0;
static unsigned long _keyframeStartTime = 0;
static bool _inTransition = false;

// Previous keyframe for interpolation
static t_choreo_clock _prevClocks[8][3];

// Direction enum mapping
static uint8_t dirToMode(uint8_t dir) {
    return (dir == 0) ? CLOCKWISE : COUNTERCLOCKWISE;
}

void choreo_init() {
    if (!LittleFS.begin(true)) {
        Serial.println("LittleFS mount failed, formatting...");
        LittleFS.format();
        LittleFS.begin(true);
    }

    // Create choreographies directory if not exists
    if (!LittleFS.exists("/choreo")) {
        LittleFS.mkdir("/choreo");
    }

    Serial.println("Choreography system initialized");
}

int choreo_list(char names[][CHOREO_NAME_LEN], int maxCount) {
    int count = 0;
    File root = LittleFS.open("/choreo");
    if (!root || !root.isDirectory()) {
        return 0;
    }

    File file = root.openNextFile();
    while (file && count < maxCount) {
        if (!file.isDirectory()) {
            String filename = file.name();
            if (filename.endsWith(".json")) {
                filename = filename.substring(0, filename.length() - 5);
                strncpy(names[count], filename.c_str(), CHOREO_NAME_LEN - 1);
                names[count][CHOREO_NAME_LEN - 1] = '\0';
                count++;
            }
        }
        file = root.openNextFile();
    }

    return count;
}

bool choreo_load(const char* name) {
    String path = "/choreo/" + String(name) + ".json";

    File file = LittleFS.open(path, "r");
    if (!file) {
        Serial.printf("Failed to open choreography: %s\n", path.c_str());
        return false;
    }

    // Parse JSON
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, file);
    file.close();

    if (error) {
        Serial.printf("JSON parse error: %s\n", error.c_str());
        return false;
    }

    // Clear current choreography
    memset(&_currentChoreo, 0, sizeof(_currentChoreo));

    // Load name
    strncpy(_currentChoreo.name, doc["name"] | name, CHOREO_NAME_LEN - 1);
    _currentChoreo.loop = doc["loop"] | false;

    // Load keyframes
    JsonArray keyframes = doc["keyframes"].as<JsonArray>();
    _currentChoreo.keyframeCount = min((int)keyframes.size(), MAX_KEYFRAMES);

    int kfIndex = 0;
    for (JsonObject kf : keyframes) {
        if (kfIndex >= MAX_KEYFRAMES) break;

        t_keyframe* keyframe = &_currentChoreo.keyframes[kfIndex];

        // Comment
        strncpy(keyframe->comment, kf["comment"] | "", CHOREO_COMMENT_LEN - 1);

        // Timing
        keyframe->transitionMs = kf["transitionMs"] | 1000;
        keyframe->delayMs = kf["delayMs"] | 0;

        // Clocks data: clocks[slave][clock]
        JsonArray clocks = kf["clocks"].as<JsonArray>();
        for (int slave = 0; slave < 8 && slave < (int)clocks.size(); slave++) {
            JsonArray slaveClocks = clocks[slave].as<JsonArray>();
            for (int clock = 0; clock < 3 && clock < (int)slaveClocks.size(); clock++) {
                JsonObject c = slaveClocks[clock].as<JsonObject>();
                keyframe->clocks[slave][clock].angleH = c["angleH"] | 180;
                keyframe->clocks[slave][clock].angleM = c["angleM"] | 180;
                keyframe->clocks[slave][clock].dirH = (strcmp(c["dirH"] | "CW", "CCW") == 0) ? 1 : 0;
                keyframe->clocks[slave][clock].dirM = (strcmp(c["dirM"] | "CW", "CCW") == 0) ? 1 : 0;
            }
        }

        kfIndex++;
    }

    _choreoLoaded = true;
    _currentKeyframe = 0;
    _state = CHOREO_STOPPED;

    Serial.printf("Loaded choreography: %s with %d keyframes\n",
                  _currentChoreo.name, _currentChoreo.keyframeCount);

    return true;
}

bool choreo_save(const char* jsonData) {
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, jsonData);

    if (error) {
        Serial.printf("JSON parse error on save: %s\n", error.c_str());
        return false;
    }

    const char* name = doc["name"] | "unnamed";
    String path = "/choreo/" + String(name) + ".json";

    File file = LittleFS.open(path, "w");
    if (!file) {
        Serial.printf("Failed to create file: %s\n", path.c_str());
        return false;
    }

    serializeJson(doc, file);
    file.close();

    Serial.printf("Saved choreography: %s\n", path.c_str());
    return true;
}

bool choreo_delete(const char* name) {
    String path = "/choreo/" + String(name) + ".json";

    if (LittleFS.remove(path)) {
        Serial.printf("Deleted choreography: %s\n", path.c_str());
        return true;
    }

    Serial.printf("Failed to delete: %s\n", path.c_str());
    return false;
}

void choreo_apply_keyframe(int index) {
    if (!_choreoLoaded || index < 0 || index >= _currentChoreo.keyframeCount) {
        return;
    }

    t_keyframe* kf = &_currentChoreo.keyframes[index];

    Serial.printf("Applying keyframe %d: %s\n", index, kf->comment);

    // Send to each slave board
    for (int slave = 0; slave < 8; slave++) {
        t_half_digit hd = {0};

        for (int clock = 0; clock < 3; clock++) {
            t_choreo_clock* cc = &kf->clocks[slave][clock];

            hd.clocks[clock].angle_h = cc->angleH;
            hd.clocks[clock].angle_m = cc->angleM;
            hd.clocks[clock].speed_h = get_speed();
            hd.clocks[clock].speed_m = get_speed();
            hd.clocks[clock].accel_h = get_acceleration();
            hd.clocks[clock].accel_m = get_acceleration();
            hd.clocks[clock].mode_h = dirToMode(cc->dirH);
            hd.clocks[clock].mode_m = dirToMode(cc->dirM);
            hd.clocks[clock].adjust_h = 0;
            hd.clocks[clock].adjust_m = 0;
        }

        // Use change counter to trigger movement
        static uint32_t counter = 1;
        hd.change_counter[0] = ++counter;
        hd.change_counter[1] = ++counter;
        hd.change_counter[2] = ++counter;

        send_half_digit(slave, hd);
    }

    // Store for next transition
    for (int slave = 0; slave < 8; slave++) {
        for (int clock = 0; clock < 3; clock++) {
            _prevClocks[slave][clock] = kf->clocks[slave][clock];
        }
    }
}

void choreo_play() {
    if (!_choreoLoaded || _currentChoreo.keyframeCount == 0) {
        Serial.println("No choreography loaded");
        return;
    }

    if (_state == CHOREO_PAUSED) {
        // Resume from pause
        _state = CHOREO_PLAYING;
        _keyframeStartTime = millis();
        Serial.println("Choreography resumed");
    } else {
        // Start from beginning
        _currentKeyframe = 0;
        _state = CHOREO_PLAYING;
        _inTransition = false;
        _keyframeStartTime = millis();

        // Apply first keyframe immediately
        choreo_apply_keyframe(0);

        Serial.printf("Playing choreography: %s\n", _currentChoreo.name);
    }
}

void choreo_pause() {
    if (_state == CHOREO_PLAYING) {
        _state = CHOREO_PAUSED;
        Serial.println("Choreography paused");
    }
}

void choreo_stop() {
    _state = CHOREO_STOPPED;
    _currentKeyframe = 0;
    _inTransition = false;
    Serial.println("Choreography stopped");
}

void choreo_next_keyframe() {
    if (!_choreoLoaded) return;

    _currentKeyframe++;
    if (_currentKeyframe >= _currentChoreo.keyframeCount) {
        _currentKeyframe = 0;
    }
    choreo_apply_keyframe(_currentKeyframe);
}

void choreo_prev_keyframe() {
    if (!_choreoLoaded) return;

    _currentKeyframe--;
    if (_currentKeyframe < 0) {
        _currentKeyframe = _currentChoreo.keyframeCount - 1;
    }
    choreo_apply_keyframe(_currentKeyframe);
}

void choreo_update() {
    if (_state != CHOREO_PLAYING || !_choreoLoaded) {
        return;
    }

    unsigned long now = millis();
    t_keyframe* currentKf = &_currentChoreo.keyframes[_currentKeyframe];

    // Calculate total time for this keyframe (transition + delay)
    unsigned long totalTime = currentKf->transitionMs + currentKf->delayMs;

    // Check if we need to move to next keyframe
    if (now - _keyframeStartTime >= totalTime) {
        _currentKeyframe++;

        if (_currentKeyframe >= _currentChoreo.keyframeCount) {
            if (_currentChoreo.loop) {
                _currentKeyframe = 0;
            } else {
                _state = CHOREO_STOPPED;
                _currentKeyframe = _currentChoreo.keyframeCount - 1;
                Serial.println("Choreography finished");
                return;
            }
        }

        _keyframeStartTime = now;
        choreo_apply_keyframe(_currentKeyframe);
    }
}

t_choreo_state choreo_get_state() {
    return _state;
}

int choreo_get_current_keyframe() {
    return _currentKeyframe;
}

const char* choreo_get_current_name() {
    if (_choreoLoaded) {
        return _currentChoreo.name;
    }
    return "";
}

String choreo_get_json() {
    if (!_choreoLoaded) {
        return "{}";
    }

    JsonDocument doc;
    doc["name"] = _currentChoreo.name;
    doc["loop"] = _currentChoreo.loop;
    doc["keyframeCount"] = _currentChoreo.keyframeCount;

    JsonArray keyframes = doc["keyframes"].to<JsonArray>();

    for (int k = 0; k < _currentChoreo.keyframeCount; k++) {
        t_keyframe* kf = &_currentChoreo.keyframes[k];
        JsonObject kfObj = keyframes.add<JsonObject>();

        kfObj["comment"] = kf->comment;
        kfObj["transitionMs"] = kf->transitionMs;
        kfObj["delayMs"] = kf->delayMs;

        JsonArray clocks = kfObj["clocks"].to<JsonArray>();
        for (int slave = 0; slave < 8; slave++) {
            JsonArray slaveClocks = clocks.add<JsonArray>();
            for (int clock = 0; clock < 3; clock++) {
                JsonObject c = slaveClocks.add<JsonObject>();
                c["angleH"] = kf->clocks[slave][clock].angleH;
                c["angleM"] = kf->clocks[slave][clock].angleM;
                c["dirH"] = (kf->clocks[slave][clock].dirH == 0) ? "CW" : "CCW";
                c["dirM"] = (kf->clocks[slave][clock].dirM == 0) ? "CW" : "CCW";
            }
        }
    }

    String output;
    serializeJson(doc, output);
    return output;
}
