# Slave Board Pinout - RP2040

## Stepper Motors (TMC2209)

| Motor | Function | STEP Pin | DIR Pin | Notes |
|-------|----------|----------|---------|-------|
| A | Clock 2 - Minute | 1 | 0 | |
| B | Clock 2 - Hour | 11 | 10 | |
| C | Clock 1 - Minute | 3 | 2 | |
| D | Clock 1 - Hour | 14 | 15 | |
| E | Clock 0 - Minute | 28 | 27 | |
| F | Clock 0 - Hour | 20 | 21 | Corrigé: pins 19/20 -> 20/21 |

## Driver Control

| Signal | Pin | Description |
|--------|-----|-------------|
| TMC_ENN | 12 | Enable all drivers (active LOW) |
| RESET | 30 | Reset motor controllers |

## I2C Address DIP Switch

| Bit | Pin | Description |
|-----|-----|-------------|
| ADDR_1 | 19 | LSB |
| ADDR_2 | 18 | |
| ADDR_3 | 17 | |
| ADDR_4 | 16 | MSB |

Address = !ADDR_1 + (!ADDR_2 << 1) + (!ADDR_3 << 2) + (!ADDR_4 << 3)

## I2C Communication

| Signal | Pin |
|--------|-----|
| SDA | 4 |
| SCL | 5 |

## Direction Inversion

| Motor | INVERT_DIR | Notes |
|-------|------------|-------|
| A | 1 | **A corriger: Clock 2 Minute inversé** |
| B | 1 | **A corriger: Clock 2 Hour inversé** |
| C | 1 | **A corriger: Clock 1 Minute inversé** |
| D | 1 | **A corriger: Clock 1 Hour inversé** |
| E | 1 | **A corriger: Clock 0 Minute inversé** |
| F | 1 | **A corriger: Clock 0 Hour inversé** |

## Historique des corrections

| Date | Correction |
|------|------------|
| 2025-12-25 | F_STEP/F_DIR: 19/20 -> 20/21 |
| 2025-12-25 | Tous les moteurs: direction inversée (INVERT_x_DIR=1 pour A,B,C,D,E,F) |
