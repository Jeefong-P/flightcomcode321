# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is **CURSR-IV Flight Computer** firmware for a high-power rocket. It runs on an **STM32U575VGTx** microcontroller (ARM Cortex-M33, 160 MHz) using **FreeRTOS** (CMSIS-RTOS v2) and is developed in **STM32CubeIDE** (Eclipse-based). The codebase is mixed C/C++.

## Build & Flash

This project has no CLI build toolchain setup — it is built exclusively through **STM32CubeIDE**:

- Open `.project` in STM32CubeIDE
- Build: **Project → Build Project** (or `Ctrl+B`)
- Flash & debug: **Run → Debug** using `CURSR-IV-Flight-Com Debug.launch`
- The `Debug/` directory contains the generated makefile and build artifacts; the actual GNU ARM toolchain is `GNU Tools for STM32 (13.3.rel1)`
- Peripheral configuration is managed via **STM32CubeMX** through `CURSR-IV-Flight-Com.ioc` — regenerate HAL code from there when changing pin assignments or peripherals
- Linker scripts: `STM32U575VGTX_FLASH.ld` (normal) and `STM32U575VGTX_RAM.ld`

## Architecture

### Thread Model

All application logic runs as FreeRTOS tasks created in [`Core/Src/app_freertos.c`](Core/Src/app_freertos.c). All threads run at `osPriorityLow`. Thread entry points are declared in [`Threads/ThreadInclude.h`](Threads/ThreadInclude.h) and implemented in the corresponding `.h` files (not `.cpp`).

| Task | File | Purpose |
|------|------|---------|
| `FlightLogicTask` | `Threads/FlightLogicThread.h` | State machine: IDLE→BOOST→COAST→APOGEE→DROGUE→MAIN→TOUCHDOWN |
| `BMI088Task` | `Threads/SensorThread.h` | IMU + ADXL375 high-G sampling |
| `MS5607Task` | `Threads/SensorThread.h` | Barometric altitude measurement |
| `GNSSTask` | `Threads/GNSSThread.h` | GPS via UART4 DMA |
| `LoRaTask` | `Threads/LoRaThread.h` | Real-time telemetry via SX1262 |
| `SDCardTask` | `Threads/SDCardThread.h` | Log file sync management |
| `BuzzerTask` | `Threads/SystemThread.h` | Audio status feedback |
| `GreenLedTask` | `Threads/SystemThread.h` | Heartbeat LED |
| `FlightLogicLogTask` | `Threads/FlightLogicThread.h` | Flight state logging |
| `BatLevelTask` | `Threads/SystemThread.h` | IMU temperature logging (battery monitoring stub) |

### Global State

[`Threads/GlobalInclude.h`](Threads/GlobalInclude.h) defines the two key globals shared across all threads — **no mutexes are used**:
- `FlightState flightState` — current phase of flight (enum 0–6)
- `InitStatus initStatus` — per-subsystem ready flags; `FlightLogicThread` blocks in `waitInit()` until all are set

### Flight State Machine

Defined in [`Threads/FlightLogicThread.h`](Threads/FlightLogicThread.h):

```
IDLE → BOOST → COAST → APOGEE → DROGUE → MAIN → TOUCHDOWN
```

- **IDLE→BOOST**: 3 consecutive samples > 100 m/s² (from BMI088)
- **BOOST→COAST**: acceleration < 50 m/s² AND > 3 s since launch
- **COAST→APOGEE**: altitude dropped > 5 m from max AND > 20 s in coast AND altitude ≥ 1000 m
- **APOGEE→DROGUE**: fires Servo2 (drogue) immediately
- **DROGUE→MAIN**: altitude < 1000 m AND ≥ 200 s since apogee → fires Servo1 (main)
- **MAIN→TOUCHDOWN**: altitude stable (< 1 m change) for 1000 consecutive readings

Global `altitude` and `acceleration` are Kalman-filtered values written by sensor threads and read by the flight logic thread.

### Sensor Drivers (`Drivers/`)

All drivers are C++ classes communicating over SPI:
- **MS5607** (`hspi2`) — barometric pressure/temperature; altitude derived from pressure using reference level taken at startup
- **BMI088** (`hspi3`) — 6-axis IMU; configured at 24G / 1600 Hz accel, 500 dps / 2000 Hz gyro; includes `applySensorOffsetCorrection()`
- **ADXL375** (`hspi3`) — ±200G accelerometer; 3200 Hz sample rate
- **SX1262** (`hspi1`) — LoRa radio at 915 MHz; two parameter sets: low-datarate (SF8) for IDLE/TOUCHDOWN, high-datarate (SF5) during flight
- **GNSS** (`huart4` + GPDMA1) — u-blox module configured via UBX binary protocol for Airborne 4G dynamic model at 10 Hz / 115200 baud

### Data Logging

Each sensor has its own [`FileLogger`](Drivers/SDCard/FileLogger.h) instance (binary structs written directly). All loggers are declared in [`Threads/SDCardThread.h`](Threads/SDCardThread.h). `SDCardThread` periodically calls `sync()` on each logger. Log files: `ms5607.log`, `bmi088Acc.log`, `bmi088Gyro.log`, `adxl375.log`, `gnss.log`, `flightLogic.log`, `batLevel.log`. Struct definitions are in [`Threads/FileLogStruct.h`](Threads/FileLogStruct.h).

All sensors log at **1 Hz during IDLE/TOUCHDOWN** and at full rate (up to 500 Hz via `osDelay(1)`) during active flight.

### Pyrotechnic / Recovery (`Drivers/PYRO/PYRO.h`)

No actual pyro channels are used for recovery — instead two servos driven by TIM3 PWM:
- **Servo1** (TIM3 CH3) = Rotor Leash → fires at main deployment (80° for 5 s, returns to 58°)
- **Servo2** (TIM3 CH4) = Dodo → fires at apogee (sweeps 40°→180°, returns to 68°)

`firePyroChannel1()` / `firePyroChannel2()` (GPIO-based pyro channels) exist but are not called in the current flight logic.

### Telemetry Packet

Defined in [`Drivers/LoRaCodec/LoRaCodec.h`](Drivers/LoRaCodec/LoRaCodec.h) — 13-byte struct `{altitude, latitude, longitude, flightState}` memcpy-encoded (no serialization library).

## Incomplete / In-Progress Areas

- **`Drivers/BMI088/ICM.c` / `ICM.h`** — ICM-45686 IMU driver replaces the old BMI088. Uses `ICM45686_Init(&hspi3)` / `ICM45686_ReadData(&hspi3, &icmData)` with a flat `ICM45686_Data_t` struct (`accel_x/y/z` in g, `gyro_x/y/z` in dps). CS pin is hardcoded as `ICM45686_CS_GPIO_Port`/`ICM45686_CS_Pin` in the driver.
- **`BatLevelThread`** — battery voltage monitoring is commented out; currently logs IMU temperature instead (and has a bug: `TemperatureData data` is constructed but never logged).
- **`IMULogThread`** — declared and spawned but body is empty (`osDelay(1000)` loop only).
- **`ADXL375` thread** declared in `ThreadInclude.h` but never spawned in `app_freertos.c`; ADXL375 is sampled inside `IMUThread` instead.
- Reference pressure auto-recalibration in `MS5607Thread` is commented out.
- GPIO interrupt-based sensor data-ready (`HAL_GPIO_EXTI_Callback`) is commented out; polling is used.

## Key Conventions

- Thread logic lives in `.h` files (not `.cpp`) — this allows the single `ThreadInclude.cpp` translation unit to pull everything in via includes.
- Global variables in header files are defined (not just declared) — acceptable because each header is included exactly once via `ThreadInclude.cpp`.
- `/* USER CODE BEGIN / END */` blocks are STM32CubeMX-managed zones; code outside these blocks will be overwritten when regenerating from the `.ioc` file.
- Adaptive logging rate: all sensor threads check `flightState` and throttle to 1 Hz when `IDLE` or `TOUCHDOWN`.
