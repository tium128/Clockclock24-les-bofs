# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

ClockClock 24 replica - a kinetic sculpture with 24 analog clocks arranged in an 8x3 matrix that display digital time through coordinated hand movements. This is a PlatformIO-based embedded systems project with a master-slave architecture.

## Build Commands

Both projects use PlatformIO. Run commands from within the `master/` or `slave/` directory:

```bash
# Build
pio run

# Upload to device
pio run -t upload

# Serial monitor (115200 baud)
pio device monitor
```

Configure `upload_port` and `monitor_port` in `platformio.ini` for your setup.

## Architecture

### Hardware Architecture
- **Master**: ESP32-S3 (Waveshare ESP32-S3 Zero) - WiFi, NTP sync, web server, I2C master
- **Slaves**: 8x Raspberry Pi Pico (RP2040) - each controls 3 clocks (6 stepper motors via AX1201728SG drivers)
- **Motors**: VID28-05 dual-shaft stepper motors (360° continuous rotation)
- **Communication**: I2C bus at 100kHz, daisy-chained boards

### Master (`master/`)
Runs on ESP32-S3. Key modules:
- `main.cpp` - Animation loop (LAZY, FUN, WAVES modes), time synchronization
- `clock_manager.h/clock_manger.cpp` - Sends clock positions to slaves via I2C
- `digit.h` - Predefined hand angle patterns for digits 0-9
- `web_server.cpp` - Serves web interface at `http://clockclock24.local`
- `ntp.cpp` - NTP time synchronization
- `clock_config.cpp` - EEPROM-based persistent configuration

### Slave (`slave/`)
Runs on RP2040 with dual-core utilization:
- **Core 0**: I2C interrupt handler (`receiveEvent`) - receives target positions
- **Core 1**: Stepper motor control loop with AccelStepper library
- Uses spin locks for thread-safe communication between cores
- I2C address set via 4-position DIP switch (pins 13, 16, 17, 18)

### Data Flow
1. Master calculates target hand angles based on current time and animation mode
2. Master sends `t_half_digit` struct (3 clocks × angle/speed/acceleration/direction) via I2C
3. Slave receives on Core 0, updates target state with spin lock protection
4. Slave Core 1 continuously runs motors toward target positions using AccelStepper

### Key Data Structures
```cpp
// Clock state sent over I2C
typedef struct clock_state {
    uint16_t angle_h, angle_m;    // Target angles (0-360°)
    uint16_t speed_h, speed_m;    // Motor speed
    uint16_t accel_h, accel_m;    // Acceleration
    uint8_t mode_h, mode_m;       // Direction mode (CLOCKWISE, MIN_DISTANCE, etc.)
    signed char adjust_h, adjust_m; // Manual calibration offset
} t_clock;

// Half-digit = 3 clocks (one vertical column)
typedef struct half_digit {
    t_clock clocks[3];
    uint32_t change_counter[3];  // Detects state changes
} t_half_digit;
```

### Motor Configuration
- Steps per revolution: 46080 (360° × 128 microsteps)
- 6 motors per board (A-F), each with STEP and DIR pins defined in `board_config.h`

## Calibration

Before powering on, manually set all clock hands to 6 o'clock position. This must be done after every power cycle.

## Test/Debug Projects

- `master-debug/` - Master debugging variant
- `master-testslaveI2C/` - I2C communication testing
- `slave-testboard/` - Slave board testing
