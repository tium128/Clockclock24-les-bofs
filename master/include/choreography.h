#ifndef choreography_h
#define choreography_h

#include <Arduino.h>
#include "clock_state.h"

// Maximum choreography limits
#define MAX_KEYFRAMES 32
#define MAX_CHOREOGRAPHIES 16
#define CHOREO_NAME_LEN 32
#define CHOREO_COMMENT_LEN 128

// Cascade modes for keyframe transitions
typedef enum {
    CASCADE_NONE = 0,
    CASCADE_COLUMN = 1,
    CASCADE_ROW = 2,
    CASCADE_DIAGONAL = 3,
    CASCADE_RIPPLE = 4,     // Ripple from center outward (like a stone in water)
    CASCADE_RIPPLE_IN = 5,  // Ripple from edges to center (reverse ripple)
    CASCADE_SNAKE = 6,      // Snake pattern left-to-right, alternating rows
    CASCADE_SPIRAL = 7      // Spiral from center outward
} t_cascade_mode;

// Clock data for a single clock in a keyframe
typedef struct {
    uint16_t angleH;
    uint16_t angleM;
    uint8_t dirH;  // 0=CW, 1=CCW
    uint8_t dirM;
} t_choreo_clock;

// A keyframe: 8 slaves x 3 clocks
typedef struct {
    t_choreo_clock clocks[8][3];
    char comment[CHOREO_COMMENT_LEN];
    // Motion parameters
    uint16_t speed;         // Motor speed (steps/sec), default 400
    uint16_t accel;         // Acceleration (steps/sec²), default 150
    uint16_t delayMs;       // Delay AFTER reaching this keyframe (ms)
    // Cascade effect
    t_cascade_mode cascadeMode;  // How to stagger the movements
    uint16_t cascadeDelayMs;     // Delay between groups (ms)
} t_keyframe;

// A choreography
typedef struct {
    char name[CHOREO_NAME_LEN];
    uint8_t keyframeCount;
    bool loop;
    t_keyframe keyframes[MAX_KEYFRAMES];
} t_choreography;

// Choreography player state
typedef enum {
    CHOREO_STOPPED,
    CHOREO_PLAYING,
    CHOREO_PAUSED
} t_choreo_state;

// Choreography auto-play mode
typedef enum {
    CHOREO_MODE_OFF = 0,      // Disabled - only manual play
    CHOREO_MODE_MANUAL = 1,   // Manual trigger only
    CHOREO_MODE_AUTO = 2,     // Auto-play on hour change
    CHOREO_MODE_RANDOM = 3    // Random pick on hour change
} t_choreo_mode;

// Initialize choreography system (call once at startup)
void choreo_init();

// Load choreography from LittleFS by name
bool choreo_load(const char* name);

// Get list of available choreographies
int choreo_list(char names[][CHOREO_NAME_LEN], int maxCount);

// Save choreography to LittleFS
bool choreo_save(const char* jsonData);

// Delete choreography
bool choreo_delete(const char* name);

// Playback controls
void choreo_play();
void choreo_pause();
void choreo_stop();
void choreo_next_keyframe();
void choreo_prev_keyframe();

// Get current state
t_choreo_state choreo_get_state();
int choreo_get_current_keyframe();
const char* choreo_get_current_name();

// Update function - call from main loop
void choreo_update();

// Apply a specific keyframe immediately
void choreo_apply_keyframe(int index);

// Get current choreography as JSON (for web interface)
String choreo_get_json();

// Auto-play mode management
void choreo_set_mode(t_choreo_mode mode);
t_choreo_mode choreo_get_mode();

// Enable/disable choreographies for random mode
void choreo_set_enabled(const char* name, bool enabled);
bool choreo_is_enabled(const char* name);

// Trigger choreography on hour change (call from main loop when hour changes)
void choreo_on_hour_change();

// Get total choreography count
int choreo_get_count();

#endif
