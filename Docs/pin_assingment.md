# LoRaGro – Pin Assignment

Target board: **SuperMini nRF52840**  
MCU: **Nordic nRF52840**  
RTOS: **Zephyr**

This document defines all pin assignments used by the LoRaGro firmware.
Arduino-style `Dx` labels are included **for reference only** and do **not**
imply pin usage or reservation.

Only pins explicitly referenced in the DeviceTree are considered in use.

---

## 1. SPI0 – SX1262 LoRa Transceiver

SPI0 is used to communicate with the Semtech SX1262 LoRa radio.

### SPI Signals

| Signal | MCU Pin | Port  | Arduino Label | Notes       |
| ------ | ------- | ----- | ------------- | ----------- |
| SCK    | P1.15   | GPIO1 | D18           | SPI clock   |
| MOSI   | P1.13   | GPIO1 | D15           | SPI MOSI    |
| MISO   | P1.11   | GPIO1 | D14           | SPI MISO    |
| CS     | P0.10   | GPIO0 | D16           | Chip select |

### SX1262 Control Pins

| Function | MCU Pin | Port  | Arduino Label | Notes        |
| -------- | ------- | ----- | ------------- | ------------ |
| RESET    | P0.20   | GPIO0 | D3            | Active low   |
| BUSY     | P0.22   | GPIO0 | D4            | Active high  |
| DIO1     | P0.24   | GPIO0 | D5            | IRQ          |
| TX_EN    | P0.11   | GPIO0 | D7            | RF switch TX |
| RX_EN    | P1.00   | GPIO1 | D6            | RF switch RX |

**Status:** ✅ All pins valid and conflict-free

---

## 2. UART1 – Modbus RTU (RS-485)

UART1 is used for Modbus RTU over RS-485.

| Function | MCU Pin | Port  | Arduino Label | Notes        |
| -------- | ------- | ----- | ------------- | ------------ |
| TX       | P0.08   | GPIO0 | D0            | UARTE TX     |
| RX       | P0.06   | GPIO0 | D1            | UARTE RX     |
| RTS      | P0.17   | GPIO0 | D2            | RS-485 DE/RE |
| CTS      | —       | —     | —             | Not used     |

**Status:** ✅ Correct for RS-485 half-duplex

---

## 3. I²C0 Bus

I²C0 is used for environmental and auxiliary sensors.

| Function | MCU Pin | Port  | Arduino Label | Notes     |
| -------- | ------- | ----- | ------------- | --------- |
| SDA      | P1.04   | GPIO1 | D8            | I²C data  |
| SCL      | P1.06   | GPIO1 | D9            | I²C clock |

### I²C Devices

| Device              | Address | Purpose   |
| ------------------- | ------- | --------- |
| Air temp / humidity | `0x76`  | BME280    |
| Ambient light       | `0x44`  | OPT3001   |
| CO₂ sensor          | `0x62`  | SCD4x     |
| GPS (via bridge)    | `0x4D`  | SC16IS740 |

**Status:** ✅ Valid configuration

---

## 4. ADC – Analog Inputs

ADC is used for soil moisture sensing and battery voltage measurement.

| Signal        | ADC Channel | MCU Pin | Arduino Label | Notes        |
| ------------- | ----------- | ------- | ------------- | ------------ |
| Soil moisture | AIN5        | P0.29   | D20           | Analog input |
| Battery sense | AIN7        | P0.31   | D21           | Analog input |

### Notes

- Pins are **used exclusively as analog inputs**
- No GPIO, PWM, or peripheral reuse
- Fully valid on nRF52840

**Status:** ✅ Correct and safe

---

## 5. 1-Wire Bus (Soil Temperature)

1-Wire bus used for soil temperature sensor (e.g. DS18B20).

| Function    | MCU Pin | Port  | Arduino Label | Notes      |
| ----------- | ------- | ----- | ------------- | ---------- |
| 1-Wire data | P0.02   | GPIO0 | D19           | Open-drain |

**Status:** ✅ Correct

---

## 6. Power and Special Pins

| Pin   | Function            | Notes                                |
| ----- | ------------------- | ------------------------------------ |
| P0.13 | External VCC cutoff | Board-supported, not yet used in DTS |
| RAW   | Battery input       | Board-level only                     |
| VCC   | 3.3 V switched      | Controlled by P0.13                  |
| RESET | System reset        | Not GPIO-usable                      |

---

## 7. Pin Usage Summary

- No pin is assigned to more than one peripheral
- No hidden Arduino pin conflicts
- All assignments are valid for SuperMini nRF52840
- ADC pins are used as analog-only, as intended

---

## 8. Overall Status

| Subsystem      | Status |
| -------------- | ------ |
| SPI0 (LoRa)    | ✅ OK   |
| UART1 (RS-485) | ✅ OK   |
| I²C0           | ✅ OK   |
| ADC            | ✅ OK   |
| 1-Wire         | ✅ OK   |

**This pin configuration is clean, correct, and production-ready.**
