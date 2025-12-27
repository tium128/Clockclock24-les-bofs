#ifndef choreography_h
#define choreography_h

#include <Arduino.h>
#include "clock_state.h"

// Maximum choreography limits
#define MAX_KEYFRAMES 32
#define MAX_CHOREOGRAPHIES 16
#define CHOREO_NAME_LEN 32
#define CHOREO_COMMENT_LEN 128

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
    uint16_t transitionMs;  // Duration to reach this keyframe
    uint16_t delayMs;       // Delay before starting next transition
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

#endif
