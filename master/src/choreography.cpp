#include "choreography.h"
#include "clock_manager.h"
#include "clock_config.h"
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

// Auto-play mode
static t_choreo_mode _choreoMode = CHOREO_MODE_OFF;

// Enabled choreographies for random mode (stored as a bitmask, max 16)
static uint16_t _enabledMask = 0xFFFF;  // All enabled by default

// Cascade timing tracking
static unsigned long _cascadeStartTime = 0;
static int _cascadeCurrentGroup = 0;

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

    // Load settings from EEPROM
    _choreoMode = (t_choreo_mode)get_choreo_mode();
    _enabledMask = get_choreo_enabled_mask();

    Serial.printf("Choreography system initialized (mode=%d, mask=0x%04X)\n", _choreoMode, _enabledMask);
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

        // Motion parameters
        keyframe->speed = kf["speed"] | 400;
        keyframe->accel = kf["accel"] | 150;
        keyframe->delayMs = kf["delayMs"] | 0;

        // Cascade effect
        const char* cascadeStr = kf["cascadeMode"] | "none";
        if (strcmp(cascadeStr, "column") == 0) {
            keyframe->cascadeMode = CASCADE_COLUMN;
        } else if (strcmp(cascadeStr, "row") == 0) {
            keyframe->cascadeMode = CASCADE_ROW;
        } else if (strcmp(cascadeStr, "diagonal") == 0) {
            keyframe->cascadeMode = CASCADE_DIAGONAL;
        } else if (strcmp(cascadeStr, "ripple") == 0) {
            keyframe->cascadeMode = CASCADE_RIPPLE;
        } else if (strcmp(cascadeStr, "ripple_in") == 0) {
            keyframe->cascadeMode = CASCADE_RIPPLE_IN;
        } else if (strcmp(cascadeStr, "snake") == 0) {
            keyframe->cascadeMode = CASCADE_SNAKE;
        } else if (strcmp(cascadeStr, "spiral") == 0) {
            keyframe->cascadeMode = CASCADE_SPIRAL;
        } else {
            keyframe->cascadeMode = CASCADE_NONE;
        }
        keyframe->cascadeDelayMs = kf["cascadeDelayMs"] | 100;

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

// Helper to calculate cascade delay for a slave/clock position
static uint16_t getCascadeDelay(t_keyframe* kf, int slave, int clock) {
    switch (kf->cascadeMode) {
        case CASCADE_COLUMN:
            return slave * kf->cascadeDelayMs;
        case CASCADE_ROW:
            return clock * kf->cascadeDelayMs;
        case CASCADE_DIAGONAL:
            return (slave + clock) * kf->cascadeDelayMs;
        case CASCADE_RIPPLE: {
            // Ripple from center outward (like a stone in water)
            // Center of 8x3 matrix: slave 3.5 (between 3 and 4), clock 1
            float centerSlave = 3.5f;
            float centerClock = 1.0f;
            float distSlave = abs(slave - centerSlave);
            float distClock = abs(clock - centerClock);
            // Use Euclidean distance for smooth circular ripple
            float distance = sqrt(distSlave * distSlave + distClock * distClock);
            int ring = (int)(distance + 0.5f);
            return ring * kf->cascadeDelayMs;
        }
        case CASCADE_RIPPLE_IN: {
            // Reverse ripple: from edges to center
            float centerSlave = 3.5f;
            float centerClock = 1.0f;
            float distSlave = abs(slave - centerSlave);
            float distClock = abs(clock - centerClock);
            float distance = sqrt(distSlave * distSlave + distClock * distClock);
            int ring = (int)(distance + 0.5f);
            // Max ring is about 4, invert it
            int maxRing = 4;
            return (maxRing - ring) * kf->cascadeDelayMs;
        }
        case CASCADE_SNAKE: {
            // Snake pattern: left-to-right on even rows, right-to-left on odd rows
            int pos;
            if (clock % 2 == 0) {
                pos = clock * 8 + slave;  // Left to right
            } else {
                pos = clock * 8 + (7 - slave);  // Right to left
            }
            return pos * kf->cascadeDelayMs;
        }
        case CASCADE_SPIRAL: {
            // Spiral from center outward
            // Define spiral order for 8x3 matrix (approximate)
            // Center positions first, then expanding outward in a spiral pattern
            static const int spiralOrder[8][3] = {
                {6, 5, 7},   // Slave 0 (leftmost)
                {4, 3, 8},   // Slave 1
                {2, 1, 9},   // Slave 2
                {1, 0, 10},  // Slave 3 (near center)
                {1, 0, 10},  // Slave 4 (near center)
                {2, 1, 9},   // Slave 5
                {4, 3, 8},   // Slave 6
                {6, 5, 7}    // Slave 7 (rightmost)
            };
            return spiralOrder[slave][clock] * kf->cascadeDelayMs;
        }
        default:
            return 0;
    }
}

// Apply a single slave's clocks (used for cascade timing)
static void applySlaveClocksWithDelay(t_keyframe* kf, int slave) {
    static uint32_t counter = 1;
    t_half_digit hd = {0};

    for (int clock = 0; clock < 3; clock++) {
        t_choreo_clock* cc = &kf->clocks[slave][clock];

        hd.clocks[clock].angle_h = cc->angleH;
        hd.clocks[clock].angle_m = cc->angleM;
        hd.clocks[clock].speed_h = kf->speed;
        hd.clocks[clock].speed_m = kf->speed;
        hd.clocks[clock].accel_h = kf->accel;
        hd.clocks[clock].accel_m = kf->accel;
        hd.clocks[clock].mode_h = dirToMode(cc->dirH);
        hd.clocks[clock].mode_m = dirToMode(cc->dirM);
        hd.clocks[clock].adjust_h = 0;
        hd.clocks[clock].adjust_m = 0;
    }

    hd.change_counter[0] = ++counter;
    hd.change_counter[1] = ++counter;
    hd.change_counter[2] = ++counter;

    send_half_digit(slave, hd);
}

void choreo_apply_keyframe(int index) {
    if (!_choreoLoaded || index < 0 || index >= _currentChoreo.keyframeCount) {
        return;
    }

    t_keyframe* kf = &_currentChoreo.keyframes[index];

    Serial.printf("Applying keyframe %d: %s (speed=%d, accel=%d, cascade=%d)\n",
                  index, kf->comment, kf->speed, kf->accel, kf->cascadeMode);

    if (kf->cascadeMode == CASCADE_NONE) {
        // No cascade - apply all slaves immediately
        for (int slave = 0; slave < 8; slave++) {
            applySlaveClocksWithDelay(kf, slave);
        }
    } else {
        // Cascade mode - stagger the commands
        // For simplicity, we apply slaves with delays using millis-based timing
        // This is a blocking approach suitable for choreography playback
        unsigned long startTime = millis();
        bool slaveSent[8] = {false};

        while (true) {
            unsigned long elapsed = millis() - startTime;
            bool allSent = true;

            for (int slave = 0; slave < 8; slave++) {
                if (!slaveSent[slave]) {
                    uint16_t delay = getCascadeDelay(kf, slave, 0); // Use clock 0 for column/diagonal
                    if (elapsed >= delay) {
                        applySlaveClocksWithDelay(kf, slave);
                        slaveSent[slave] = true;
                    } else {
                        allSent = false;
                    }
                }
            }

            if (allSent) break;
            delay(10); // Small delay to avoid busy-waiting
        }
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

// Estimate transition time based on speed (rough approximation)
// Assumes average 90° movement at given speed
static uint16_t estimateTransitionTime(t_keyframe* kf) {
    // steps for 90° = 46080 * 90 / 360 = 11520 steps
    // time = steps / speed (roughly, ignoring acceleration)
    // Add some buffer for acceleration ramp-up/down
    uint16_t baseTime = (11520 / kf->speed) * 1000;
    return baseTime + 500; // Add 500ms buffer
}

void choreo_update() {
    if (_state != CHOREO_PLAYING || !_choreoLoaded) {
        return;
    }

    unsigned long now = millis();
    t_keyframe* currentKf = &_currentChoreo.keyframes[_currentKeyframe];

    // Calculate total time for this keyframe:
    // - Estimated transition time based on speed
    // - Plus cascade total time if cascade mode
    // - Plus explicit delay after keyframe
    uint16_t transitionTime = estimateTransitionTime(currentKf);
    uint16_t cascadeTime = 0;
    if (currentKf->cascadeMode != CASCADE_NONE) {
        // Max cascade delay = 7 slaves * cascadeDelayMs (for column mode)
        cascadeTime = 7 * currentKf->cascadeDelayMs;
    }
    unsigned long totalTime = transitionTime + cascadeTime + currentKf->delayMs;

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
        kfObj["speed"] = kf->speed;
        kfObj["accel"] = kf->accel;
        kfObj["delayMs"] = kf->delayMs;

        // Cascade mode
        const char* cascadeStr = "none";
        switch (kf->cascadeMode) {
            case CASCADE_COLUMN: cascadeStr = "column"; break;
            case CASCADE_ROW: cascadeStr = "row"; break;
            case CASCADE_DIAGONAL: cascadeStr = "diagonal"; break;
            case CASCADE_RIPPLE: cascadeStr = "ripple"; break;
            case CASCADE_RIPPLE_IN: cascadeStr = "ripple_in"; break;
            case CASCADE_SNAKE: cascadeStr = "snake"; break;
            case CASCADE_SPIRAL: cascadeStr = "spiral"; break;
            default: cascadeStr = "none"; break;
        }
        kfObj["cascadeMode"] = cascadeStr;
        kfObj["cascadeDelayMs"] = kf->cascadeDelayMs;

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

// ============================================
// AUTO-PLAY MODE FUNCTIONS
// ============================================

void choreo_set_mode(t_choreo_mode mode) {
    _choreoMode = mode;
    set_choreo_mode((int)mode);  // Persist to EEPROM
    Serial.printf("Choreography mode set to: %d\n", mode);
}

t_choreo_mode choreo_get_mode() {
    return _choreoMode;
}

void choreo_set_enabled(const char* name, bool enabled) {
    // Find the index of this choreography
    char names[MAX_CHOREOGRAPHIES][CHOREO_NAME_LEN];
    int count = choreo_list(names, MAX_CHOREOGRAPHIES);

    for (int i = 0; i < count && i < 16; i++) {
        if (strcmp(names[i], name) == 0) {
            if (enabled) {
                _enabledMask |= (1 << i);
            } else {
                _enabledMask &= ~(1 << i);
            }
            set_choreo_enabled_mask(_enabledMask);  // Persist to EEPROM
            Serial.printf("Choreography '%s' %s\n", name, enabled ? "enabled" : "disabled");
            return;
        }
    }
}

bool choreo_is_enabled(const char* name) {
    char names[MAX_CHOREOGRAPHIES][CHOREO_NAME_LEN];
    int count = choreo_list(names, MAX_CHOREOGRAPHIES);

    for (int i = 0; i < count && i < 16; i++) {
        if (strcmp(names[i], name) == 0) {
            return (_enabledMask & (1 << i)) != 0;
        }
    }
    return true; // Default to enabled if not found
}

int choreo_get_count() {
    char names[MAX_CHOREOGRAPHIES][CHOREO_NAME_LEN];
    return choreo_list(names, MAX_CHOREOGRAPHIES);
}

void choreo_on_hour_change() {
    // Don't trigger if already playing or if mode is off/manual
    if (_state == CHOREO_PLAYING) {
        return;
    }

    if (_choreoMode != CHOREO_MODE_AUTO && _choreoMode != CHOREO_MODE_RANDOM) {
        return;
    }

    // Get list of choreographies
    char names[MAX_CHOREOGRAPHIES][CHOREO_NAME_LEN];
    int count = choreo_list(names, MAX_CHOREOGRAPHIES);

    if (count == 0) {
        Serial.println("No choreographies available for auto-play");
        return;
    }

    if (_choreoMode == CHOREO_MODE_AUTO) {
        // Auto mode: play the first enabled choreography
        for (int i = 0; i < count && i < 16; i++) {
            if (_enabledMask & (1 << i)) {
                if (choreo_load(names[i])) {
                    choreo_play();
                    Serial.printf("Auto-playing choreography: %s\n", names[i]);
                }
                return;
            }
        }
    } else if (_choreoMode == CHOREO_MODE_RANDOM) {
        // Random mode: pick a random enabled choreography
        // First, count enabled choreographies
        int enabledIndices[MAX_CHOREOGRAPHIES];
        int enabledCount = 0;

        for (int i = 0; i < count && i < 16; i++) {
            if (_enabledMask & (1 << i)) {
                enabledIndices[enabledCount++] = i;
            }
        }

        if (enabledCount == 0) {
            Serial.println("No enabled choreographies for random mode");
            return;
        }

        // Pick a random one
        int randomIdx = random(0, enabledCount);
        int chosenIdx = enabledIndices[randomIdx];

        if (choreo_load(names[chosenIdx])) {
            choreo_play();
            Serial.printf("Random-playing choreography: %s\n", names[chosenIdx]);
        }
    }
}
