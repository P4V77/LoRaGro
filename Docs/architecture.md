# LoRaGro Firmware Architecture

**Version:** 0.1 (Early Development)
**Last Updated:** December 2025
**Status:** 🔨 Phase 1 – Foundation & Prototyping

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
7. [Data Model & Protocol](#7-data-model--protocol)
8. [Power Management](#8-power-management)
9. [Development Approach](#9-development-approach)
10. [Current Status](#10-current-status)
11. [Open Questions](#11-open-questions)

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

```c
const struct device *bme280 = DEVICE_DT_GET_OR_NULL(DT_NODELABEL(bme280));
if (bme280 && device_is_ready(bme280)) {
    register_sensor(bme280);
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
* LoRa transmission
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
* Downlink scheduling

**Power options**

* USB‑C / mains
* Optional solar for remote locations

**Status**
Planned for **Phase 2**

---

## 4. Sensor‑Derived Configuration

LoRaGro does not define configuration profiles as first‑class entities. Instead, **behavior emerges from detected hardware**.

The following groupings are conceptual and describe common sensor combinations seen in practice.

### 4.1 Field‑Oriented Setups

* **BASIC**
  Soil moisture, soil temperature, air T/H/P

* **ORCHARD**
  BASIC + rain gauge

* **VINEYARD**
  ORCHARD + leaf wetness

* **COASTAL**
  RS‑485 soil probe (moisture, temperature, EC)

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
│   ├── app/
│   │   ├── Fino-LoRaGro/     # All sensing nodes
│   │   └── Gano-LoRaGro/     # Gateway application
│   └── modules/
│       └── loragro_module/
│
├── SW-LoRaGro/
│   └── Gapp-LoRaGro/
│
├── Hardware-LoRaGro/
├── Common-LoRaGro/
├── Docs/
├── LICENSE
├── LICENSE-HARDWARE.md
├── LICENSE-DOCS.md
└── README.md
```

---

## 6. Firmware Architecture

### 6.1 Application Structure (FiNo)

```c
void main(void) {
    platform_init();
    sensor_manager_init();
    lora_init();

    while (1) {
        sensor_manager_sample_all();
        measurement_batch_t batch = get_measurements();
        lora_transmit(&batch);

        uint32_t next_wake = config_get_sample_interval();
        power_sleep_until(next_wake);
    }
}
```

This loop is **identical for all deployments**.
Only detected hardware and policy parameters influence behavior.

---

## 7. Data Model & Protocol

### 7.1 Measurement Model

```c
struct lg_measurement {
    uint16_t sensor_id;
    uint8_t  sensor_type;
    int32_t  value;
    uint32_t timestamp;
};
```

Greenhouse‑specific sensors (CO₂, light) are treated identically to field sensors.

---

### 7.2 LoRa Packet Overview

* Compact binary format
* CBOR‑encoded payload
* Batched measurements
* Optimized for low airtime

---

## 8. Power Management

Power behavior is **hardware‑ and policy‑dependent**:

* Field‑oriented setups → aggressive deep sleep
* Greenhouse‑oriented setups → higher duty cycle

**Key idea**
Power management is a **policy layer**, not a node role.

---

## 9. Development Approach

### 9.1 Simulation‑First

Supported targets:

* `native_sim`
* `nrf52_bsim`
* `promicro_nrf52840`

Simulation behavior is identical across all hardware configurations.

---

## 10. Current Status

**Working**

* Build system
* Devicetree overlays
* Sensor discovery
* Simulation targets

**In progress**

* LoRa driver (SX1262)
* Sensor abstraction layer
* Power profiling

**Planned**

* Gateway firmware
* Custom PCB
* Field trials

---

## 11. Open Questions

* Optimal greenhouse sampling policies
* CO₂ sensor calibration strategy
* Downlink configuration updates
* Encryption timing and scope

---

## Document History

| Version | Date          | Changes                       |
| ------: | ------------- | ----------------------------- |
|     0.1 | December 2025 | Initial architecture document |
