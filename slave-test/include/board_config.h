#ifndef CONFIG_H
#define CONFIG_H

#define STEPS 5760 // 360 * 16
//#define STEPS 46080 // 360 * 128

#define A_STEP 1// f(scx)
#define A_DIR 0// CW/CCW
#define B_STEP 11// f(scx)
#define B_DIR 10// CW/CCW

#define C_STEP 3 // f(scx)
#define C_DIR 2 // CW/CCW
#define D_STEP 14 // f(scx)
#define D_DIR 15 // CW/CCW

#define E_STEP 28// f(scx)
#define E_DIR 27// CW/CCW
#define F_STEP 20// f(scx)
#define F_DIR 21// CW/CCW

#define TMC_ENN 12 // Enable pin for all drivers (low active)

// Inversion des DIR (0 = normal, 1 = inversé)
// Tous inversés suite aux tests du 2025-12-25
#define INVERT_A_DIR 1
#define INVERT_B_DIR 1
#define INVERT_C_DIR 1
#define INVERT_D_DIR 1
#define INVERT_E_DIR 1
#define INVERT_F_DIR 1


// I2C address
#define ADDR_1 19
#define ADDR_2 18
#define ADDR_3 17
#define ADDR_4 16

#define WIRE_SDA 4 // Alreaday defined
#define WIRE_SCL 5

#define RESET 30

#endif