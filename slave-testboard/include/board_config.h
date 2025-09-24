#ifndef CONFIG_H
#define CONFIG_H

#define STEPS 5760 // 360 * 16

#define A_STEP 20 // f(scx)
#define A_DIR 21 // CW/CCW
#define B_STEP 27 // f(scx)
#define B_DIR 28 // CW/CCW

#define C_STEP 15 // f(scx)
#define C_DIR 14 // CW/CCW
#define D_STEP 1 // f(scx)
#define D_DIR 0 // CW/CCW

#define E_STEP 11 // f(scx)
#define E_DIR 10 // CW/CCW
#define F_STEP 3 // f(scx)
#define F_DIR 2 // CW/CCW

// I2C address
#define ADDR_1 13
#define ADDR_2 16
#define ADDR_3 17
#define ADDR_4 18

#define WIRE_SDA 4 // Alreaday defined
#define WIRE_SCL 5

#define RESET 30

#endif