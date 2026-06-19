# SX1262 LoRa Module Usage Guide

## Introduction
The SX1262 is a low-power long-range LoRa transceiver module operating in sub-GHz bands. This library provides an interface to configure and operate the SX1262 in LoRa mode.

⚠️ **IMPORTANT:** Ensure that the power supply to your board is **UNDER 3.3V**! Exceeding this voltage will damage the module.

## Hardware Setup
To use the SX1262 module, you'll need to connect the following pins to your STM32 microcontroller:

- **SPI_NSS**: SPI slave select pin (output)
- **RF_NRESET**: Hard reset pin for the RF chip (output)
- **RF_BUSY**: Busy status pin (input)
- **RF_DIO1**: Digital IO pin for interrupt signaling (input)
- **SPI Interface**: Connect the module to your microcontroller's SPI bus

## Library Initialization

### Include the Library
```c
#include "SX1262.h"
```

### Define LoRa Parameters
```c
loRa_Para_t lora_parameters;

// Configure parameters
lora_parameters.rf_freq = 868000000;  // Frequency in Hz (e.g., 868 MHz)
lora_parameters.tx_power = 14;        // Transmit power in dBm (-17 to +22)
lora_parameters.lora_sf = LORA_SF7;   // Spreading factor (SF5-SF12)
lora_parameters.band_width = LORA_BW_125; // Bandwidth (e.g., 125 kHz)
lora_parameters.code_rate = LORA_CR_4_5; // Coding rate
lora_parameters.payload_size = 16;    // Size of the payload
```

### Initialize the SX1262 Object
```c
// Create SX1262 object with pin definitions
SX1262 lora(SPI_CS_Pin, SPI_CS_GPIO_Port,
            RESET_Pin, RESET_GPIO_Port,
            BUSY_Pin, BUSY_GPIO_Port,
            DIO1_Pin, DIO1_GPIO_Port,
            &hspi1);  // Use your SPI handle

// Initialize the module with parameters
lora.Init(&lora_parameters);
```

## Sending Data

### Transmit a Packet
```c
uint8_t tx_buffer[32] = "Hello LoRa World!";
uint8_t buffer_size = 16;  // Size of the data to send

// Transmit data
lora.TxPacket(tx_buffer, buffer_size);

// Wait for transmission to complete
if(lora.WaitForIRQ_TxDone()) {
    // Transmission successful
} else {
    // Transmission failed or timed out
}
```

## Receiving Data

### Initialize Reception
```c
uint8_t rx_buffer[256]; // Buffer to store received data
uint16_t rx_size = 0;   // Variable to store the received data size

// Initialize the receive buffer
lora.RxBufferInit(rx_buffer, &rx_size);

// Start receiving
lora.RxInit();
```

### Check for Received Data
```c
if(lora.WaitForIRQ_RxDone()) {
    // Data received, rx_buffer now contains the data
    // rx_size contains the number of bytes received
    
    // Process received data...
}
```

## Power Management

### Set Module to Sleep Mode
```c
lora.SetSleep();
```

### Set Module to Standby Mode
```c
// 0: STDBY_RC (RC oscillator)
// 1: STDBY_XOSC (XOSC oscillator)
lora.SetStandby(0);
```

### Reset the Module
```c
lora.Reset_SX1262();
```

## LoRa Parameter Definitions

### Spreading Factors
- `LORA_SF5` to `LORA_SF12`: Higher spreading factors increase range but reduce data rate

### Bandwidths
- `LORA_BW_7`: 7.8 kHz
- `LORA_BW_10`: 10.4 kHz
- `LORA_BW_15`: 15.6 kHz
- `LORA_BW_20`: 20.8 kHz
- `LORA_BW_31`: 31.25 kHz
- `LORA_BW_41`: 41.7 kHz
- `LORA_BW_62`: 62.5 kHz
- `LORA_BW_125`: 125 kHz
- `LORA_BW_250`: 250 kHz
- `LORA_BW_500`: 500 kHz

### Coding Rates
- `LORA_CR_4_5`: 4/5 coding rate
- `LORA_CR_4_6`: 4/6 coding rate
- `LORA_CR_4_7`: 4/7 coding rate
- `LORA_CR_4_8`: 4/8 coding rate

## Example: Complete Transceiver

```c
#include "main.h"
#include "SX1262.h"

// LoRa parameters
loRa_Para_t lora_params;
SX1262 lora(NSS_Pin, NSS_GPIO_Port,
           NRESET_Pin, NRESET_GPIO_Port,
           BUSY_Pin, BUSY_GPIO_Port,
           DIO1_Pin, DIO1_GPIO_Port,
           &hspi1);

uint8_t rx_buffer[256];
uint16_t rx_size = 0;

void setup_lora() {
    // Configure parameters
    lora_params.rf_freq = 868000000;
    lora_params.tx_power = 14;
    lora_params.lora_sf = LORA_SF9;
    lora_params.band_width = LORA_BW_125;
    lora_params.code_rate = LORA_CR_4_5;
    lora_params.payload_size = 32;
    
    // Initialize
    lora.Init(&lora_params);
    
    // Setup receiver
    lora.RxBufferInit(rx_buffer, &rx_size);
    lora.RxInit();
}

void send_message(uint8_t *data, uint8_t size) {
    lora.TxPacket(data, size);
    if(lora.WaitForIRQ_TxDone()) {
        // Return to receive mode
        lora.RxInit();
    }
}

void check_received() {
    if(lora.WaitForIRQ_RxDone()) {
        // Data is in rx_buffer with length rx_size
        // Process data...
        
        // Reset for next reception
        rx_size = 0;
    }
}
```

## Performance Optimization Tips

1. **Range vs Data Rate**: Higher spreading factors (SF) increase range but reduce data rate. Lower bandwidths increase sensitivity but reduce data rate.

2. **Power Consumption**: Use sleep mode when not active for extended periods.

3. **Payload Size**: Keep payload size as small as possible for better reliability.

4. **Error Correction**: Higher coding rates provide better error correction but reduce effective data rate.

5. **TCXO**: The library assumes a 32MHz TCXO for frequency stability. If you're using a different oscillator, adjust the `FREQ_STEP` in the library.

## Troubleshooting

- **Module Not Responding**: Check power supply (must be under 3.3V), SPI connections, and NSS pin level.

- **Transmission Issues**: Verify antenna connection and ensure no obstacles between transceivers.

- **Reception Problems**: Check if both transmitter and receiver have identical configuration parameters.

- **Timeouts**: The library includes auto-recovery from timeouts by resetting the chip.

- **SPI Issues**: Ensure proper SPI timing and voltage levels.
