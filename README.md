# LoRaGro 🌾

![Build Status](https://img.shields.io/badge/build-in%20progress-yellow)
![License](https://img.shields.io/badge/license-Apache%202.0-blue)
![Platform](https://img.shields.io/badge/platform-Zephyr%20RTOS-orange)
![Hardware](https://img.shields.io/badge/hardware-nRF52840-blue)

**Modular LoRa-Based Agricultural IoT Platform with Solar-Powered, Energy-Autonomous Nodes**

LoRaGro is an open-source, modular agricultural IoT platform that enables long-term, low-power monitoring of crops, soil, and microclimate conditions. With solar charging, 
nodes can operate indefinitely. On battery alone, expect 6–12 months of autonomous operation from a single 18650 cell.

```
[FiNo Node] ──LoRa──> [GaNo Gateway] ──WiFi/Ethernet──> [Server/App]
     ↓                        ↓                                ↓
  Sensors                Bridge logic                    Data storage
  Solar + battery        Time sync                       Visualization
  Waterproof             Buffering                       Alerts
```

> **🚧 Project Status:** Active firmware development (Phase 1)  
> Hardware PCB design coming in Phase 2 (months 7-8)

---

## 🚀 Quick Start

**Want to try LoRaGro?**

1. **Clone the repository:**
   ```bash
   git clone https://github.com/yourusername/loragro.git
   cd loragro
   ```

2. **Set up development environment:**
   - Install [Zephyr SDK](https://docs.zephyrproject.org/latest/develop/getting_started/)
   - See [firmware setup guide](docs/firmware-setup.md)

3. **Development hardware needed:**
   - nRF52840-DK development board
   - SX1262 LoRa module
   - Sensors (see [hardware requirements](docs/hardware-requirements.md))

4. **Join the community:**
   - GitHub Issues: [Report bugs/suggestions](https://github.com/yourusername/loragro/issues)
   - Discord: [Coming soon]

---

## ✨ Key Features

- ✅ **Energy autonomous** — Solar + battery operation for indefinite runtime
- ✅ **Ultra-low power** — 6-12 months on single 18650 (battery-only mode)
- ✅ **Modular sensors** — Plug-and-play, auto-detected via Devicetree
- ✅ **Single platform** — One firmware codebase for all node types
- ✅ **Open source** — Apache 2.0 firmware, CERN-OHL hardware
- ✅ **LoRa connectivity** — Long range, low power, no cellular needed
- ✅ **Waterproof** — IP67 enclosure (planned) for outdoor deployment
- ✅ **Real-time monitoring** — Mobile app with alerts (planned)

---

## 🌱 Platform Philosophy

**One platform. Multiple roles. Maximum flexibility.**

- **One firmware codebase** — Zephyr RTOS powers all nodes
- **One core hardware design** — nRF52840 + SX1262 foundation
- **Multiple node roles** — Configured via peripherals and settings
- **Sensor auto-detection** — Devicetree-driven, plug-and-play
- **Firmware-first approach** — Iterate quickly, commit hardware later

This architecture enables:
- Rapid prototyping and testing
- Clean, maintainable codebase
- Easy addition of new sensor types
- Cost-effective scaling

---

## 🧩 Node Types

| Node Type | Purpose | Power Source | Typical Location | Status |
|-----------|---------|--------------|------------------|--------|
| **FiNo** | Field sensing | Battery + solar | Outdoor fields, orchards | 🔨 Active dev |
| **GaNo** | Data gateway | Mains powered | Farm buildings | 📅 Planned (Phase 2) |

### FiNo — Field Node

Low-power sensing node for agricultural environments.

**Key specifications:**
- **Power:** Single 18650 Li-ion (3000-6000mAh) + optional 5W solar panel
- **Battery life (battery-only):**
  - BASIC config: ~12 months (15-min readings)
  - ORCHARD config: ~10 months (adds rain gauge)
  - VINEYARD config: ~8 months (adds leaf wetness)
- **Solar operation:** Indefinite runtime with adequate sunlight (>4h/day)
- **Enclosure:** IP67 waterproof (planned)
- **Communication:** LoRa uplink (868MHz EU / 915MHz US)
- **Range:** 2-10 km (depending on terrain)

**Fully modular** — sensors detected automatically at boot.

---

### GaNo — Gateway Node

LoRa-to-IP bridge for data collection and forwarding.

**Responsibilities:**
- Collects data from FiNo nodes
- Bridges LoRa → Ethernet/WiFi/LTE
- Time synchronization
- Data buffering
- Optional edge processing

**Power:** Mains-powered for 24/7 reliability.

---

## 📦 FiNo Sensor Configurations

FiNo hardware is modular. Sensors are auto-detected at boot using Devicetree overlays.

### Configuration Comparison

| Feature | BASIC ⭐⭐⭐⭐⭐ | ORCHARD ⭐⭐⭐⭐ | VINEYARD ⭐⭐⭐⭐ | COASTAL ⭐⭐⭐⭐ |
|---------|---------|----------|-----------|----------|
| **Soil moisture** | ✅ Capacitive | ✅ Capacitive | ✅ Capacitive | ✅ RS485 (pro) |
| **Soil temperature** | ✅ DS18B20 | ✅ DS18B20 | ✅ DS18B20 | ✅ RS485 (pro) |
| **Air temp/humidity** | ✅ BME280 | ✅ BME280 | ✅ BME280 | ✅ BME280 |
| **Air pressure** | ✅ BME280 | ✅ BME280 | ✅ BME280 | ✅ BME280 |
| **Rain gauge** | ❌ | ✅ Tipping bucket | ✅ Tipping bucket | ❌ |
| **Leaf wetness** | ❌ | ❌ | ✅ Resistive | ❌ |
| **Soil EC (salinity)** | ❌ | ❌ | ❌ | ✅ RS485 |
| **Battery life** | 12 months | 10 months | 8 months | 12 months |
| **Est. BOM cost** | $35-45 | $45-55 | $65-80 | $70-90 |
| **Target price** | $60-80 | $75-100 | $90-125 | $100-130 |

---

### BASIC — Universal Configuration ⭐⭐⭐⭐⭐

**Sensors included:**
- Capacitive soil moisture (ADC)
- Soil temperature — DS18B20 (1-Wire)
- Air temperature, humidity, pressure — BME280 (I²C)
- Battery voltage monitoring (built-in ADC)

**What it does:**
- ✅ Tells you when to irrigate (soil moisture)
- ✅ Warns about frost (air temperature drops)
- ✅ Predicts disease risk (humidity + temperature)
- ✅ Monitors soil warmth for planting timing
- ✅ Tracks weather patterns (pressure trends)

**Best for:**
- Field crops (wheat, corn, soybeans)
- Vegetables (tomatoes, peppers, lettuce)
- Small orchards (apples, peaches)
- Vineyards (basic tier)
- Pasture management
- Home gardens

**Target market:** 70-80% of farmers

---

### ORCHARD Configuration ⭐⭐⭐⭐

**Everything in BASIC, plus:**
- Rain gauge — Tipping bucket (GPIO)

**Why farmers need it:**
- Skip irrigation when it just rained (save water + money)
- Track on-site rainfall (more accurate than distant weather stations)
- Disease models require rainfall data
- Insurance claims documentation

**Best for:**
- Apple, peach, citrus, stone fruit orchards
- Vineyards with irrigation
- Vegetable farms with precise water management

**Target market:** 15% of farmers

---

### GREENHOUSE Configuration ⭐⭐⭐⭐

**Purpose:** High-resolution, controlled-environment monitoring for greenhouses, polytunnels, and indoor grow rooms where light, CO₂, and ventilation directly affect yield and quality.

**Everything in BASIC, plus:**
- **CO₂ concentration** — SCD30 / SCD41 (I²C)
- **Ambient light (PAR proxy)** — BH1750 (I²C)
- **Optional soil moisture (multi-point)** — Capacitive probes (ADC, 2–4 channels)
- **Optional relay/IO expansion** — Vent/fan/irrigation status sensing (GPIO, read-only in Phase 1)

**Why growers need it:**
- Optimize **photosynthesis** by keeping CO₂ in the optimal range
- Balance **light vs. temperature** to prevent plant stress
- Detect **ventilation failures** early (CO₂ rising too fast)
- Improve **fertigation timing** with tighter moisture feedback
- Increase yield consistency in intensive production

**Typical metrics collected:**
- Air temperature & humidity
- Barometric pressure
- CO₂ ppm (trend + thresholds)
- Light level (lux as PAR proxy)
- Soil moisture (zone-based, optional)
- Battery voltage (if battery-backed)

**Power profile:**
- **Primary:** Mains or dedicated 5V supply
- **Backup:** Single 18650 (graceful brownout handling)
- **Battery-only (backup mode):** ~3–6 months (higher sensor duty cycle)

**Communication:**
- **Primary:** LoRa uplink to GaNo
- **Optional:** Local BLE for commissioning and diagnostics

**Best for:**
- Greenhouses & polytunnels
- Hydroponics / aquaponics
- Indoor vertical farms
- Seedling nurseries
- Research & trial plots

**Recommended sampling intervals:**
- CO₂ / Air T&H: every 1–5 minutes
- Light level: every 1–5 minutes (daytime adaptive)
- Soil moisture: every 10–15 minutes

**Alerts & automation hooks (planned):**
- CO₂ too high/low → ventilation alert
- Excess heat under high light → shading alert
- Prolonged humidity → disease risk warning
- Optional IO hooks for external controllers (Phase 3)

**Estimated BOM impact:**
- +$20–35 (CO₂ + light sensors)
- **Target node price:** $90–120

**Target market:**  
High-value, intensive growers (≈10–15% of deployments)

> _Note:_ GREENHOUSE configuration prioritizes data density over ultra-low power. When mains power is available, duty cycles can be increased significantly without impacting reliability.

---

### VINEYARD Configuration ⭐⭐⭐⭐

**Everything in ORCHARD, plus:**
- Leaf wetness sensor — Resistive probe (ADC)

**Why farmers need it:**
- Disease prediction (fungal infections need wet leaves)
- Spray timing optimization (don't spray when leaves are wet)
- Dew/frost detection (more accurate than humidity alone)

**Disease prevention for:**
- Powdery mildew, downy mildew (grapes)
- Apple scab, rust
- Tomato late blight
- Cucurbit downy mildew

**Target market:** 3-5% of farmers (high-value crops!)

---

### COASTAL / ARID Configuration ⭐⭐⭐⭐

**Replaces basic soil sensors with:**
- RS485 Modbus 3-in-1 professional probe
  - Soil moisture
  - Soil temperature
  - Electrical conductivity (EC)

**Why farmers need it:**
- Salinity monitoring (coastal salt intrusion, arid soils)
- Professional-grade accuracy
- Lower maintenance (one probe vs. two sensors)
- Pore water EC calculation (moisture-compensated salinity)

**Best for:**
- Coastal farms with salt intrusion issues
- Arid/desert agriculture (saline soils)
- Greenhouse operations (fertigation management)
- Organic farms (soil health monitoring)
- Research stations

**Target market:** 5-8% of farmers

---

## 🏗️ System Architecture

### Hardware Block Diagram

```
┌─────────────────────────────────────────┐
│           FiNo Field Node               │
├─────────────────────────────────────────┤
│  ┌──────────┐        ┌──────────┐       │
│  │ nRF52840 │◄──I²C──┤  BME280  │       │
│  │   MCU    │        │ (air T/H)│       │
│  │          │◄─1Wire─┤ DS18B20  │       │
│  │          │        │(soil T)  │       │
│  │          │◄──ADC──┤ Moisture │       │
│  └────┬─────┘        │  Sensor  │       │
│       │              └──────────┘       │
│       │ SPI                             │
│       ▼                                 │
│  ┌──────────┐        ┌──────────┐       │
│  │  SX1262  │◄───────┤  Solar   │       │
│  │   LoRa   │        │  Panel   │       │
│  └──────────┘        │   5W     │       │
│                      └─────┬────┘       │
│                            │            │
│                      ┌─────▼────┐       │
│                      │  18650   │       │
│                      │ Battery  │       │
│                      └──────────┘       │
└─────────────────────────────────────────┘
           │
           │ LoRa 868/915 MHz
           ▼
┌─────────────────────────────────────────┐
│           GaNo Gateway                  │
├─────────────────────────────────────────┤
│  ┌──────────┐        ┌──────────┐       │
│  │ nRF52840 │        │ Ethernet │       │
│  │   MCU    │───────►│   or     │───────┼───► Internet
│  │          │        │  WiFi    │       │
│  └────┬─────┘        └──────────┘       │
│       │ SPI                             │
│       ▼                                 │
│  ┌──────────┐                           │
│  │  SX1262  │                           │
│  │   LoRa   │                           │
│  └──────────┘                           │
│                                         │
│  [Mains powered 5V]                     │
└─────────────────────────────────────────┘
```

### Software Architecture

```
┌──────────────────────────────────────────────┐
│          Zephyr RTOS Application             │
├──────────────────────────────────────────────┤
│  ┌────────────┐  ┌──────────────┐            │
│  │   Sensor   │  │   LoRa       │            │
│  │ Abstraction│  │ Communication│            │
│  │   Layer    │  │    Layer     │            │
│  └────────────┘  └──────────────┘            │
│                                              │
│  ┌────────────┐  ┌──────────────┐            │
│  │   Power    │  │ Configuration│            │
│  │ Management │  │   Manager    │            │
│  └────────────┘  └──────────────┘            │
├──────────────────────────────────────────────┤
│         Zephyr RTOS Kernel                   │
│  (Scheduling, Power, Drivers, Devicetree)    │
├──────────────────────────────────────────────┤
│         nRF52840 Hardware (BSP)              │
└──────────────────────────────────────────────┘
```

---

## 🔧 Technology Stack

### Firmware
- **RTOS:** Zephyr 3.x
- **SDK:** nRF Connect SDK (Nordic)
- **Language:** C++17
- **Build system:** CMake + west
- **Key subsystems:**
  - Devicetree for hardware abstraction
  - Power management (deep sleep modes)
  - NVS (Non-Volatile Storage)
  - Logging and debugging shell
  - OTA/DFU via BLE or LoRa

### Hardware
- **MCU:** nRF52840 (ARM Cortex-M4F, 64MHz, 1MB Flash, 256KB RAM)
- **Radio:** SX1262 (LoRa transceiver, 868/915 MHz)
- **Power:** 18650 Li-ion (3000-6000mAh) + solar MPPT charging
- **Sensors:**
  - I²C: BME280, BH1750, SCD30
  - 1-Wire: DS18B20
  - ADC: Capacitive moisture, leaf wetness
  - RS485: Professional soil probes
- **Enclosure:** IP67 waterproof (planned)

### Mobile App (Planned)
- **Framework:** Tauri (cross-platform)
- **Backend:** Rust
- **Frontend:** Svelte/React
- **Database:** SQLite (local storage)
- **Features:**
  - Real-time node monitoring
  - Historical data visualization
  - Alert configuration
  - CSV data export

---

## 🛠️ Hardware Status

**Current stage:** ✅ Firmware development and simulation

**Development hardware in use:**
- nRF52840-DK (Nordic development board)
- SX1262 LoRa breakout modules
- Off-the-shelf sensors for prototyping:
  - BME280 modules
  - Capacitive soil moisture sensors
  - DS18B20 temperature probes
- Solar panels + TP4056 charging circuits
- 18650 batteries and holders

**Production PCB:** ❌ Not yet designed

**Timeline:**
- Custom PCB design: Phase 2 (months 7-8)
- First prototype batch: Month 8
- Field testing: Months 11-12

---

## 🗺️ Roadmap

### ✅ Phase 1: Foundation — **IN PROGRESS**
- [x] Project infrastructure (GitHub, documentation)
- [x] Zephyr RTOS setup and "Hello World"
- [ ] LoRa communication layer (SX1262 driver)
- [ ] Sensor abstraction layer (generic interface)
- [ ] Power management (deep sleep, battery monitoring)
- [ ] Configuration system (NVS storage)

### 📅 Phase 2: Product Development
- [ ] Custom PCB design (KiCad schematic and layout)
- [ ] PCB prototyping (5-10 units from JLCPCB)
- [ ] Mobile app development (Rust + Tauri)
- [ ] Field testing with friendly farmers (2-3 locations)
- [ ] Iterate based on real-world feedback

### 📅 Phase 3: Launch & Growth
- [ ] Production PCB design (50-100 units)
- [ ] Enclosure design (3D printable, IP67)
- [ ] User documentation and video tutorials
- [ ] OSHWA certification
- [ ] First commercial sales (20-50 units)

### 📅 Phase 4: Expansion
- [ ] Greenhouse variant (CO₂ and light sensors)
- [ ] Cold chain monitoring variant (high-accuracy temperature)
- [ ] Additional sensor configurations
- [ ] Scale to 200-500 units

---

## 🤝 Contributing

LoRaGro is in active development! We welcome contributions:

- 🐛 **Bug reports** — Found an issue? [Open an issue](https://github.com/yourusername/loragro/issues)
- 💡 **Feature suggestions** — Have an idea? Share it!
- 📖 **Documentation** — Help improve guides and tutorials
- 🔧 **Code contributions** — Pull requests welcome!
- 🧪 **Testing** — Try the firmware and report your findings

**How to contribute:**
1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

See [CONTRIBUTING.md](CONTRIBUTING.md) for detailed guidelines.

**Get in touch:**
- GitHub Issues: [Report bugs/suggestions](https://github.com/yourusername/loragro/issues)
- Email: your.email@example.com
- Discord: [Coming soon]

---

## 📜 License

LoRaGro uses multiple licenses for different components:

| Component | License | File |
|-----------|---------|------|
| **Firmware** (C++/Zephyr code) | Apache 2.0 | [LICENSE](LICENSE) |
| **Hardware** (PCB, schematics, enclosures) | CERN-OHL-W-2.0 | [LICENSE-HARDWARE.md](LICENSE-HARDWARE.md) |
| **Documentation** (guides, manuals, tutorials) | CC-BY-4.0 | [LICENSE-DOCS.md](LICENSE-DOCS.md) |

### Why Multiple Licenses?

- **Apache 2.0 (firmware)**: Patent protection, commercial-friendly, permissive
- **CERN-OHL-W-2.0 (hardware)**: Improvements to the design must be shared, but commercial use is allowed
- **CC-BY-4.0 (docs)**: Allows remixing and translation with attribution

### TLDR: Can I use this commercially?

**Yes!** You can:
- ✅ Manufacture and sell LoRaGro nodes
- ✅ Build products using LoRaGro components
- ✅ Fork and modify for your needs
- ✅ Use in proprietary systems

**You must:**
- ✅ Share improvements to the PCB design (CERN-OHL-W)
- ✅ Provide attribution (all licenses)
- ✅ Include license notices

**You don't have to:**
- ❌ Open-source your entire product (only LoRaGro modifications)
- ❌ Pay royalties or fees

---

## 🙏 Acknowledgments

**Built with:**
- [Zephyr RTOS](https://www.zephyrproject.org/) — Open source RTOS
- [nRF Connect SDK](https://www.nordicsemi.com/Products/Development-software/nrf-connect-sdk) — Nordic's development platform
- [KiCad](https://www.kicad.org/) — Open source PCB design

**Inspired by:**
- Open-source hardware movement
- Agricultural sustainability challenges
- Need for affordable, repairable IoT solutions

**Special thanks to:**
- Open source community

---

## 📞 Contact & Support

- **GitHub Issues:** [Report bugs or request features](github.com/P4V77/Zephyr/LoRaGro)
- **Email:**pavelmich.id@gmail.com
- **Website:** [Coming soon]-
- **Documentation:** [docs/](docs/)

---

## ⭐ Star History

If you find LoRaGro useful, please consider starring the repository!

[![Star History Chart](https://api.star-history.com/svg?repos=yourusername/loragro&type=Date)](https://star-history.com/#yourusername/loragro&Date)

---

**Built with ❤️  for farmers, by makers.**
