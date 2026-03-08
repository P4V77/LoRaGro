# LoRaGro Firmware Architecture

**Version:** 0.2
**Last Updated:** March 2026
**Status:** 🔨 Phase 1 – FiNo simulation complete, GaNo planned

---

## Overview

This document describes the firmware and system architecture of the **LoRaGro** platform.
LoRaGro is an open‑source, modular LoRa‑based agricultural IoT system designed for long‑term, low‑power monitoring of crops, soil, and microclimate conditions.

**Target audience**

* Contributors and firmware developers
* Hardware designers planning sensor integrations
* Anyone wanting to understand how LoRaGro works internally

LoRaGro follows a **firmware‑first, hardware‑driven architecture** built on Zephyr RTOS.
All sensing deployments—field, orchard, vineyard, greenhouse, and coastal—use the **same FiNo firmware**.
Node behavior is not selected by predefined roles or explicit profiles, but **emerges from the set of sensors detected at startup**.

---

## Table of Contents

1. [System Overview](#1-system-overview)
2. [Architectural Principles](#2-architectural-principles)
3. [Node Roles](#3-node-roles)
4. [Sensor‑Derived Configuration](#4-sensor-derived-configuration)
5. [Repository Structure](#5-repository-structure)
6. [Firmware Architecture](#6-firmware-architecture)
7. [Security Architecture](#7-security-architecture)
8. [Data Model & Protocol](#8-data-model--protocol)
9. [Power Management](#9-power-management)
10. [Configuration & Persistence](#10-configuration--persistence)
11. [Development Approach](#11-development-approach)
12. [Current Status](#12-current-status)
13. [Open Questions](#13-open-questions)

---

## 1. System Overview

### 1.1 High‑Level Topology

```
┌─────────────┐
│ FiNo        │
│ Sensing     │
│ Node        │
│             │
│ Sensors     │
│ LoRa TX     │
└──────┬──────┘
       │
       │ LoRa 868 / 915 MHz
       │
┌──────▼──────┐
│ GaNo        │
│ Gateway     │
│             │
│ LoRa RX/TX  │
│ IP Network  │
│ (Eth/WiFi/  │
│ LTE)        │
└──────┬──────┘
       │
       ▼
  Backend / App
```

All sensing nodes run the same **FiNo firmware binary**. Differences in behavior—sampling rate, enabled measurements, power policy, and operational assumptions—are a direct consequence of **which sensors are present**.

At boot, FiNo automatically discovers available hardware using **Devicetree**, initializes only the detected sensors, and enables the corresponding functionality. There are no firmware forks, role switches, or deployment‑specific binaries.

---

## 2. Architectural Principles

### 2.1 One Firmware, Many Deployments

LoRaGro intentionally avoids artificial node variants.

* One sensing firmware: **FiNo**
* One gateway firmware: **GaNo**
* Many real‑world deployments derived from hardware

This approach keeps:

* Code size minimal
* Execution paths deterministic
* Power behavior predictable
* Long‑term maintenance manageable

---

### 2.2 Hardware‑Driven Configuration

Behavioral differences between deployments are expressed through:

* Devicetree overlays (hardware presence)
* Kconfig / `prj.conf` (capabilities and limits)
* Policy parameters stored in NVS (sampling, power)

There is **no runtime role switching** and no hard‑coded deployment modes.

**Benefits**

* No dead code
* Smaller binaries
* Easier verification
* Better power optimization

---

### 2.3 Devicetree‑Driven Modularity

All sensors are declared in Devicetree and discovered at boot.

```dts
&i2c0 {
    bme280: bme280@76 {
        compatible = "bosch,bme280";
        reg = <0x76>;
    };
};
```

#### Runtime Discovery

```cpp
const struct device *bme280 = DEVICE_DT_GET_OR_NULL(DT_NODELABEL(bme280));
if (bme280 && device_is_ready(bme280)) {
    sample_mgr_.add_sensor(&env_sensor_);
}
```

This enables:

* Plug‑and‑play sensors
* Greenhouse deployments without firmware forks
* Clean separation of hardware description and application logic

---

## 3. Node Roles

### 3.1 FiNo — Sensing Node

**Primary mission**
Low‑power environmental data acquisition and LoRa uplink.

**Responsibilities**

* Periodic sensor sampling
* Measurement batching
* LoRa transmission with CMAC authentication
* Downlink CONFIG frame processing
* Deep sleep between cycles

**Target characteristics**

* Battery life: up to **12 months** (battery‑only deployments)
* Average current: **<10 µA** (field‑oriented setups)
* Flash usage: **<256 KB**
* RAM usage: **<32 KB**

**Typical deployment environments**

* Open fields
* Orchards
* Vineyards
* Greenhouses
* Coastal / saline areas

---

### 3.2 GaNo — Gateway Node

**Primary mission**
Bridge the LoRa network to IP‑based infrastructure.

**Responsibilities**

* Receive uplinks from FiNo nodes
* Forward data to backend services
* Buffer data during outages
* Time synchronization
* Downlink scheduling (CONFIG frames)

**Power options**

* USB‑C / mains for permanent installation
* Battery + solar for remote locations

**Status:** Planned for **Phase 2**

---

## 4. Sensor‑Derived Configuration

LoRaGro does not define configuration profiles as first‑class entities. Instead, **behavior emerges from detected hardware**.

The following groupings are conceptual and describe common sensor combinations seen in practice.

### 4.1 Field‑Oriented Setups

* **BASIC** — Soil moisture, soil temperature, air T/H/P
* **ORCHARD** — BASIC + rain gauge
* **VINEYARD** — ORCHARD + leaf wetness
* **COASTAL** — RS‑485 soil probe (moisture, temperature, EC)

These setups prioritize **ultra‑low power operation**.

---

### 4.2 Greenhouse‑Oriented Setup

A greenhouse deployment is a FiNo node with additional sensors and different power assumptions.

**Typical additions**

* CO₂ sensor (SCD30 / SCD4x)
* Ambient light sensor (BH1750 / OPT3001)
* Higher sampling rate (1–5 minutes)

**Key characteristics**

* Power efficiency is secondary to data resolution
* Mains power is common (battery as backup)
* Same firmware loop, different policy

No architectural changes are required to support greenhouse deployments.

---

## 5. Repository Structure

### 5.1 Top‑Level Layout

```
LoRaGro/
├── FW-LoRaGro/
│   ├── Fino-LoRaGro/         # FiNo node firmware
│   │   └── src/
│   │       └── app.cpp       # Main application loop
│   └── common/               # Shared library
│       ├── include/lora/     # LoRa stack headers
│       ├── src/              # LoRa stack implementation
│       └── drivers/lora/     # Fake SX1262 driver (simulation)
│
├── docs/                     # Documentation
├── hardware/                 # PCB designs (future)
└── README.md
```

---

## 6. Firmware Architecture

### 6.1 Core Components

| Component         | File                               | Responsibility                             |
| :---------------- | :--------------------------------- | :----------------------------------------- |
| `App`             | `src/app.cpp`                      | Main orchestration loop                    |
| `SampleManager`   | `common/src/sample_manager`        | Aggregates sensor measurements             |
| Sensor Adapters   | `common/src/sensors/`              | Hardware abstraction per sensor type       |
| `Auth`            | `common/src/lora_auth.cpp`         | CMAC signing, verification, replay protect |
| `FrameCodec`      | `common/src/lora_frame_codec`      | TX frame building and encoding             |
| `ProtocolHandler` | `common/src/lora_protocol_handler` | RX CONFIG decoding and command execution   |
| `Interface`       | `common/src/lora_interface`        | LoRa send/receive, ACK handling            |
| `ConfigManager`   | `common/src/config_manager`        | NVS persistence, singleton                 |
| `PowerManagement` | `common/src/power_management`      | Battery-aware sleep decisions              |

### 6.2 Run Cycle (FiNo)

The entire node behavior is driven by a single `run_cycle()` function called in a `while(true)` loop:

```
cfg.load()                         ← reload NVS config (picks up ID changes etc.)
powerOn()
  auth.init_key()                  ← derive device key from combined_id
  sample_all()                     ← read all registered sensors
  for each frame:
    build_frame()
    sign_frame()                   ← CMAC with tx_counter
    send_confirmed()               ← TX + wait for ACK
      verify_ack()                 ← CMAC check only, no replay protection
  receive()                        ← optional CONFIG downlink
    verify_frame()                 ← CMAC + replay protection
    decode()                       ← execute commands, save to cfg
    build_response() + sign()
    send_response()
powerOff()
cfg.save()                         ← persist counters and config changes
handle_sleep()                     ← battery-aware sleep duration
```

### 6.3 Software Architecture Diagram

```
Application (App)
   │
   ├── SampleManager
   │     └── Sensor Adapters (BME280, BH1750, SCD41, Soil, ADC...)
   │
   ├── LoRa Stack
   │     ├── Interface        (send_confirmed / receive)
   │     ├── Auth             (sign_frame / verify_ack / verify_frame)
   │     ├── FrameCodec       (build_frame TX)
   │     └── ProtocolHandler  (decode CONFIG RX)
   │
   ├── ConfigManager          (NVS singleton)
   └── PowerManagement        (battery-aware sleep)
```

---

## 7. Security Architecture

### 7.1 Key Hierarchy

```
MASTER_KEY (hardcoded, 128-bit AES)
    └── derive(combined_id)
        └── device_key (per-node, 128-bit AES, cached in RAM)
```

The device key is re-derived whenever `combined_id` changes (e.g. after `SET_COMBINED_ID` CONFIG command).

### 7.2 Frame Authentication

Every frame (DATA, CONFIG, ACK, RESPONSE) carries a 4-byte AES-CMAC tag:

```
tag = AES-CMAC(device_key, [32-bit_counter || header || payload])[0:4]
```

### 7.3 Replay Protection

| Frame Type | Protection           | Notes                                     |
| :--------- | :------------------- | :---------------------------------------- |
| DATA (TX)  | tx_counter monotonic | Increments every frame, persisted to NVS  |
| ACK        | CMAC only            | No replay protection — ephemeral response |
| CONFIG     | rx_counter + CMAC    | Full replay protection                    |
| RESPONSE   | tx_counter           | Part of normal TX signing                 |

### 7.4 Counter Persistence

Counters are written to NVS with thresholds to balance security and flash wear:
- `tx_counter`: written every 64 frames (~1×/day at 15 min interval)
- `rx_counter`: written every 16 frames

After a cold boot, the node resumes from the saved threshold. The maximum gap is bounded by the threshold value, which is well within replay protection tolerance.

---

## 8. Data Model & Protocol

### 8.1 Measurement Model

```cpp
struct Measurement {
    SensorID sensor_id;
    int32_t  value1;      // ÷1000 for fixed-point
    int32_t  value2;      // optional second value
    uint32_t timestamp;   // Unix epoch
};
```

All sensors — field, greenhouse, or otherwise — produce `Measurement` objects. The protocol treats them identically regardless of sensor type.

### 8.2 LoRa Protocol

Custom lightweight binary protocol. See **LoRaGro Protocol Specification v1.3** for full details.

Key properties:
* Fixed 4-byte header (Target ID + Frame Type + Frame Counter)
* AES-CMAC authentication on every frame
* 8-bit wire counter with 32-bit reconstruction
* CONFIG downlink for remote management

---

## 9. Power Management

Power behavior is **hardware‑ and policy‑dependent**:

* Field‑oriented setups → aggressive deep sleep (15–240 min intervals)
* Greenhouse‑oriented setups → higher duty cycle (1–5 min)

### 9.1 Sleep State Machine

```
Battery >= critical_mv  → sample_interval_minutes + TDMA offset
Battery < critical_mv   → sample_interval_min_low_battery + TDMA offset
Battery < cutoff_mv     → deep sleep (critically_low_battery_timeout_hours)
  └── wake → re-check battery only (no full sample)
  └── if recovered → resume normal operation
```

### 9.2 Key Power Parameters (Defaults)

| Parameter                              | Default  | Description                    |
| :------------------------------------- | :------- | :----------------------------- |
| `sample_interval_minutes`              | 15 min   | Normal sampling interval       |
| `sample_interval_min_low_battery`      | 240 min  | Low battery interval           |
| `critically_low_battery_timeout_hours` | 12 hours | Deep sleep duration at cutoff  |
| `battery_critical_mv`                  | 3000 mV  | Threshold for reduced interval |
| `battery_cutoff_mv`                    | 2600 mV  | Threshold for deep sleep       |

**No unnecessary sensor sampling during recovery loop** — only the battery ADC is read to check for recovery.

---

## 10. Configuration & Persistence

### 10.1 ConfigManager

Singleton NVS-backed store for `DeviceConfig`. Key properties:

* Loaded at start of every `run_cycle()` to pick up changes from previous cycle
* Write-on-change only (NVS read-compare before write)
* `Auth` holds a direct reference to `ConfigManager::config_` — no copy/sync needed

### 10.2 DeviceConfig Fields

| Field                     | Description                            |
| :------------------------ | :------------------------------------- |
| `combined_id`             | Gateway ID (5b) + Node ID (11b)        |
| `tx_security_counter`     | Monotonic TX counter, persisted to NVS |
| `rx_security_counter`     | Last verified RX counter, persisted    |
| `lora`                    | LoRa radio parameters                  |
| `sample_interval_minutes` | Normal sleep interval                  |
| `battery_critical_mv`     | Low battery threshold                  |
| `battery_cutoff_mv`       | Deep sleep threshold                   |
| `max_tx_frames_per_cycle` | Hard limit on TX frames (TDMA bound)   |
| `protocol_version`        | Protocol compatibility check           |
| `config_version`          | NVS schema version                     |

### 10.3 NVS Flash Wear

With default thresholds and 15-minute sampling on nRF52840 (1MB flash):

| Counter    | Writes/day | Flash life (2 sectors) | Flash life (128 sectors) |
| :--------- | :--------- | :--------------------- | :----------------------- |
| tx_counter | ~1.5       | ~36 years              | >>100 years              |
| rx_counter | ~1/week    | ~385 years             | >>100 years              |

Flash endurance is not a practical concern for this application.

---

## 11. Development Approach

### 11.1 Simulation‑First

All development is done in simulation before touching hardware.

Supported targets:

* `nrf52_bsim` (BabbleSim) — primary simulation target
* `promicro_nrf52840` — real hardware target

**Fake drivers implemented for:**
* SX1262 LoRa transceiver (with realistic airtime simulation, ACK signing, CONFIG injection)
* BME280 environmental sensor
* BH1750 light sensor
* SCD41 CO₂ sensor
* RS485 soil probe (Modbus)
* ADC (battery + soil analog)
* Voltage regulator

The fake SX1262 driver simulates a full gateway interaction including:
* Realistic airtime delays (SX126x formula)
* Signed ACK responses
* Periodic CONFIG frame injection for testing downlink

### 11.2 Long-Run Simulation

The firmware has been validated in extended BSIM runs:
* 62+ hours simulated runtime
* Frame counter 8-bit overflow (255→0→1) verified
* Battery low/recovery cycle verified
* CONFIG SET_COMBINED_ID end-to-end verified (ID change persists across reboot)
* NVS counter persistence across power cycles verified

---

## 12. Current Status

### Implemented ✅

* Full run cycle (sample → TX → RX → sleep)
* CMAC frame signing and verification
* TX counter with NVS persistence
* RX counter with replay protection and NVS persistence
* ACK verification (CMAC only, no replay)
* CONFIG frame reception and decoding
* `SET_COMBINED_ID` command — saves to NVS, triggers reboot
* Battery-aware sleep state machine
* TDMA sleep offset
* Fake SX1262 driver with CONFIG injection
* 16-hour RX counter auto-reset
* Config reload at start of each cycle

### Pending ❌

* `SAMPLING_INTERVAL` command handler
* `REBOOT` command handler
* `SET_UNIX_TIME` command handler
* `LORA_CONFIG` command handler
* Broadcast `0xFFFF` frame support
* Node TX counter reset after 16h without ACK
* GaNo gateway firmware
* Zephyr test suite (unit + integration)

---

## 13. Open Questions

* Optimal greenhouse sampling policies
* CO₂ sensor calibration strategy
* OTA/DFU update mechanism
* GaNo backend protocol (MQTT vs HTTP vs custom)

---

## Document History

| Version | Date          | Changes                                                                                                                 |
| ------: | ------------- | :---------------------------------------------------------------------------------------------------------------------- |
|     0.1 | December 2025 | Initial architecture document                                                                                           |
|     0.2 | March 2026    | Full update: security arch, run cycle detail, NVS wear analysis, current status, fake driver capabilities, pending work |