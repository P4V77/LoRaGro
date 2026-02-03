# LoRaGro 🌾

![Build Status](https://img.shields.io/badge/build-in%20progress-yellow)
![License](https://img.shields.io/badge/license-Apache%202.0-blue)
![Platform](https://img.shields.io/badge/platform-Zephyr%20RTOS-orange)
![Hardware](https://img.shields.io/badge/hardware-nRF52840-blue)

**Solar-Powered Agricultural IoT Platform with Modular LoRa Nodes**

LoRaGro is an open-source agricultural monitoring platform for long-term, autonomous crop and soil sensing. Solar-powered nodes run indefinitely, or 6–12 months on battery alone.

```
[FiNo Node] ──LoRa──> [GaNo Gateway] ──WiFi/Ethernet──> [Server/App]
     ↓                        ↓                                ↓
  Sensors                Bridge logic                    Data storage
  Solar + battery        Time sync                       Visualization
  Waterproof             Buffering                       Alerts
```

> **🚧 Project Status:** Phase 1 (Firmware Development)  
> Hardware PCB design coming in Phase 2

---

## ✨ Why LoRaGro?

- **Energy autonomous** — Solar + battery for indefinite operation
- **Long battery life** — 6-12 months on single 18650 (battery-only)
- **Fully modular** — Add or remove sensors as needed
- **Plug-and-play sensors** — Auto-detected via Devicetree
- **Single firmware** — One codebase for all node types
- **Open source** — Apache 2.0 firmware, CERN-OHL hardware
- **LoRa connectivity** — Long range (2-10 km), no cellular needed for sensore nodes
- **Weatherproof** — IP67 enclosure (planned)

---

## 🧩 System Overview

### Node Types

| Node     | Purpose       | Power           | Location       | Status       |
| -------- | ------------- | --------------- | -------------- | ------------ |
| **FiNo** | Field sensing | Battery + solar | Outdoor        | 🔨 Active dev |
| **GaNo** | Data gateway  | Mains           | Farm buildings | 📅 Planned    |

### FiNo — Field Node

Autonomous sensing node for agricultural monitoring.

**Power:**
- Primary: Single 18650 Li-ion (3000mAh)
- Optional: 5W solar panel for indefinite runtime
- Battery life: 6-12 months depending on configuration

**Communication:**
- LoRa uplink (868MHz EU / 915MHz US)
- Range: 2-10 km depending on terrain
- Optional: BLE for commissioning

**Enclosure:** IP67 waterproof (planned)

**Key feature:** Fully modular — add or remove sensors based on your needs.

### GaNo — Gateway Node

LoRa-to-IP bridge for data collection.

**Functions:**
- Collects data from FiNo nodes
- Bridges LoRa → WiFi/Ethernet/LTE
- Time synchronization and data buffering
- Optional edge processing

**Power:** Mains-powered for 24/7 operation

---

## 📦 Sensor Configurations

### Modular Design Philosophy

**LoRaGro is fully modular.** Sensors are:
- Auto-detected at boot via Devicetree
- Hot-swappable between configurations
- Optional in any configuration

**All configurations start with capacitive humidity sensors as the foundation.** Where your operation requires it, you can upgrade to a professional RS485 probe, which provides enhanced moisture precision and adds salinity and soil temperature measurement.

---

### FIELD BASIC Configuration ⭐⭐⭐⭐⭐

**Purpose:** Universal foundation for agricultural monitoring

**Core sensors:**
- **Soil moisture** — Capacitive sensor (ADC)
- **Soil temperature** —   (1-Wire)
- **Air temp/humidity/pressure** — BME280 (I²C)
- **Battery monitoring** — Built-in ADC

**What it measures:**
- When to irrigate (soil moisture)
- Frost warnings (air temperature)
- Disease risk (humidity + temperature)
- Planting timing (soil warmth)
- Weather patterns (pressure trends)

**Battery life:** ~12 months  
**Est. BOM cost:** $35-45  
**Target price:** $60-80

**Best for:**
- Field crops (wheat, corn, soybeans)
- Vegetables (tomatoes, peppers, lettuce)
- Small orchards and vineyards
- Pasture management
- Home gardens
- Any application where basic "wet vs dry" is sufficient

**Target market:** 60-70% of all deployments

---

### ORCHARD Configuration ⭐⭐⭐⭐

**Foundation: FIELD BASIC sensors, plus:**
- **Rain gauge** — Tipping bucket (GPIO interrupt)

**Why add rain tracking:**
- Skip irrigation when it rains (save water and money)
- More accurate than distant weather stations
- Disease modeling requires rainfall data
- Insurance documentation

**Optional upgrade to Premium:**
- Replace capacitive moisture + DS18B20 with **RS485 3-in-1 probe** (+$60-100)
- **Recommended for:** Commercial orchards with fertigation systems or salinity concerns
- **Skip if:** Running basic drip irrigation with good soil quality

**Battery life:** ~10 months  
**Est. BOM cost:** 
- Basic: $45-60
- Premium: $100-145  
**Target price:** 
- Basic: $75-110
- Premium: $155-220

**Best for:**
- Apple, peach, citrus, stone fruit orchards
- Vineyards with irrigation systems
- Vegetable farms with precision water management

**Target market:** 15% of farmers

---

### VINEYARD Configuration ⭐⭐⭐⭐

**Foundation: FIELD BASIC sensors, plus:**
- **Rain gauge** — Tipping bucket (GPIO interrupt)
- **Leaf wetness** — Resistive probe (ADC)

**Why track leaf wetness:**
- Disease prediction (fungal infections need wet leaves)
- Spray timing optimization (don't spray when wet)
- Dew/frost detection (more accurate than humidity)

**Prevents diseases:**
- Powdery mildew, downy mildew (grapes)
- Apple scab, rust
- Tomato late blight
- Cucurbit downy mildew

**Optional upgrade to Premium:**
- Replace capacitive moisture + DS18B20 with **RS485 3-in-1 probe** (+$60-100)
- **Recommended for:** Premium wine grapes, organic vineyards tracking soil health
- **Skip if:** Standard table grapes or basic disease monitoring is your primary goal

**Battery life:** ~8 months  
**Est. BOM cost:** 
- Basic: $65-85
- Premium: $120-165  
**Target price:** 
- Basic: $95-135
- Premium: $175-250

**Best for:** High-value crops requiring disease management

**Target market:** 3-5% of farmers

---

### GREENHOUSE Configuration ⭐⭐⭐⭐

**Foundation: FIELD BASIC sensors, plus:**
- **CO₂ concentration** — SCD30 or SCD41 (I²C)
- **Light level (PAR proxy)** — BH1750 (I²C)
- **Optional:** Multi-zone soil moisture (2-4 probes)
- **Optional:** Relay/IO expansion for monitoring vent/fan status

**Why these sensors matter:**
- Optimize photosynthesis (CO₂ in ideal range)
- Balance light vs. temperature (prevent plant stress)
- Detect ventilation failures early (CO₂ rising)
- Improve fertigation timing (moisture feedback)

**Optional upgrade to Premium:**
- Replace capacitive moisture + DS18B20 with **RS485 3-in-1 probe** (+$60-100)
- **Recommended for:** Hydroponic/aquaponic systems with active fertigation
- **Skip if:** Growing in soil/coco with basic irrigation

**Power:**
- Primary: Mains or dedicated 5V supply
- Backup: Single 18650 (graceful brownout handling)
- Battery-only backup: 3-6 months

**Sampling intervals:**
- CO₂ / Air T&H: every 1-5 minutes
- Light: every 1-5 minutes (daytime adaptive)
- Soil moisture: every 10-15 minutes

**Alerts (planned):**
- CO₂ too high/low → ventilation alert
- Excess heat under high light → shading alert
- Prolonged humidity → disease risk

**Battery life:** 3-6 months (backup mode)  
**Est. BOM cost:** 
- Basic: $65-90
- Premium: $120-175  
**Target price:** 
- Basic: $100-150
- Premium: $180-260

**Best for:**
- Greenhouses and polytunnels
- Hydroponics / aquaponics
- Indoor vertical farms
- Seedling nurseries
- Research plots

**Target market:** 10-15% of deployments

---

### COASTAL / ARID Configuration ⭐⭐⭐⭐

**Foundation: FIELD BASIC air sensors, but:**
- **RS485 3-in-1 Modbus probe REQUIRED** (not optional)
  - Soil moisture
  - Soil temperature
  - Electrical conductivity (EC/salinity)
- **Air temp/humidity/pressure** — BME280 (I²C)
- **Battery monitoring** — Built-in ADC

**Why RS485 probe is required (not optional):**
- Salinity monitoring is the primary use case
- EC measurement is essential, not a nice-to-have
- Basic sensors can't measure EC at all
- You're deploying specifically to monitor salt levels

**What it monitors:**
- Salt intrusion from coastal flooding
- Soil salinity in arid environments
- Soil remediation progress
- Safe thresholds for crop tolerance
- Irrigation water quality impact

**Battery life:** ~12 months  
**Est. BOM cost:** $90-130  
**Target price:** $140-200

**Best for:**
- Coastal farms (salt intrusion issues)
- Arid/desert agriculture (saline soils)
- Reclaimed land management
- Soil remediation projects
- Research in marginal lands

**Target market:** 5-8% of farmers

> **Note:** This is the only configuration where the Premium sensor is required by default, because EC monitoring is the reason you'd choose this configuration.

---

## 🔄 Upgrade Path: Basic to Premium

**All configurations start with FIELD BASIC sensors.** Here's when to upgrade:

### The RS485 3-in-1 Probe Premium Upgrade

**What you get:**
- Soil moisture (±3% accuracy vs ±5-10%)
- Soil temperature (±0.5°C vs ±2°C)
- Electrical conductivity / salinity (not available in basic)

**Investment:** +$60-100 per node

**Worth upgrading when:**
- ✅ You **need** EC/salinity data (primary reason to upgrade)
- ✅ Running fertigation systems (nutrient management)
- ✅ High-value crops >$5,000/hectare (precision matters)
- ✅ Commercial operations with 5+ nodes
- ✅ Coastal or arid environments
- ✅ Research-grade data needed

**Stick with basic when:**
- ✅ Field crops (wheat, corn, soybeans)
- ✅ Simple "wet vs dry" irrigation decisions
- ✅ Good soil quality (no salinity issues)
- ✅ Small farms or hobby operations
- ✅ Budget-conscious deployments
- ✅ Testing LoRaGro before scaling up

### Configuration-Specific Recommendations

| Configuration    | Basic Recommended?             | Premium Upgrade Worth It?                        |
| ---------------- | ------------------------------ | ------------------------------------------------ |
| **FIELD BASIC**  | ✅ Perfect as-is                | Only if you need EC monitoring                   |
| **ORCHARD**      | ✅ Great for most               | Upgrade if fertigation or salinity concerns      |
| **VINEYARD**     | ✅ Disease focus, basic is fine | Upgrade for premium wines, organic certification |
| **GREENHOUSE**   | ✅ CO₂/light are priorities     | Upgrade only for hydro/aquaponic fertigation     |
| **COASTAL/ARID** | ❌ Premium required             | EC monitoring is mandatory, not optional         |

---

## 🔄 Mix and Match Examples

**Remember: Sensors are modular!** 

**Budget-conscious builds:**
- FIELD BASIC standalone ($60-80)
- ORCHARD with basic sensors ($75-110)
- VINEYARD with basic sensors ($95-135)
- GREENHOUSE with basic sensors ($100-150)

**Professional builds:**
- FIELD BASIC → Premium upgrade ($140-200)
- ORCHARD → Premium upgrade ($155-220)
- VINEYARD → Premium upgrade ($175-250)
- GREENHOUSE → Premium upgrade ($180-260)

**Custom combinations:**
- FIELD BASIC + rain gauge only (budget orchard, no leaf wetness)
- ORCHARD basic + CO₂ sensor (enclosed orchard with canopy management)
- GREENHOUSE without CO₂ (cost-optimized polytunnel)
- Start basic, add rain gauge later, upgrade to premium probe in year 2

**Deployment strategy for large farms:**
- Use Premium nodes in critical zones (near irrigation heads, problem areas)
- Use Basic nodes for general monitoring across fields
- Example: 20 basic nodes + 5 premium nodes = comprehensive coverage at reasonable cost

Sensors are detected automatically at boot—just plug them in!

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
│  │          │◄─RS485─┤ SEN0601  │       │
│  │          │  (opt) │(soil pro)│       │
│  └────┬─────┘        └──────────┘       │
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
- **SDK:** nRF Connect SDK
- **Language:** C++17
- **Build:** CMake + west
- **Features:**
  - Devicetree hardware abstraction
  - Deep sleep power management
  - NVS storage
  - Logging and debug shell
  - OTA/DFU updates

### Hardware
- **MCU:** nRF52840 (Cortex-M4F, 64MHz, 1MB Flash, 256KB RAM)
- **Radio:** SX1262 LoRa (868/915 MHz)
- **Power:** 18650 Li-ion + solar MPPT charging
- **Sensors:** I²C, 1-Wire, ADC, RS485 Modbus
- **Enclosure:** IP67 waterproof (planned)

### Mobile App (Planned)
- **Framework:** Tauri (cross-platform)
- **Backend:** Rust
- **Frontend:** Svelte/React
- **Database:** SQLite
- **Features:** Real-time monitoring, alerts, data export

---

## 🛠️ Hardware Status

**Current stage:** Firmware development and simulation

**Development hardware:**
- nRF52840-DK development board
- SX1262 LoRa modules
- Off-the-shelf sensors (BME280, DS18B20, capacitive moisture, etc.)
- Solar panels + TP4056 charging circuits
- 18650 batteries

**Production PCB:** Not yet designed (Phase 2, months 7-8)

---

## 🗺️ Roadmap

### ✅ Phase 1: Foundation (IN PROGRESS)
- [x] Project setup (GitHub, docs)
- [x] Zephyr RTOS "Hello World"
- [x] LoRa communication (SX1262 driver)
- [x] Sensor abstraction layer
- [x] Power management (deep sleep)
- [x] Configuration system (NVS)
- [ ] FiNo Native Sim FW Completed
- [ ] Tests for Fino Native Sim
- [ ] FiNo Node Native Sim FW  Tested
- [ ] GaNo Native Sim FW Completed 
- [ ] Tests for GaNo Native Sim
- [ ] FiNo Node Native Sim FW Tested

### 📅 Phase 2: Product Development
- [ ] Mobile app development
- [ ] FiNo and GaNo for Pro Micro NRF52840 development
- [ ] Building HW of Nodes (Mostly from finished modules)
- [ ] Field testing (2-3 locations)
- [ ] Iterate based on feedback

### 📅 Phase 3: Launch & Growth
- [ ] Production PCB with (50-100 units)
- [ ] IP67 enclosure design
- [ ] User documentation and tutorials
- [ ] Certification
- [ ] First commercial sales

### 📅 Phase 4: Expansion
- [ ] Additional sensor variants
- [ ] Cold chain monitoring
- [ ] Scale to 200-500 units

---

## 🤝 Contributing

Contributions welcome! Help us build LoRaGro:

- 🐛 Bug reports
- 💡 Feature suggestions
- 📖 Documentation improvements
- 🔧 Code contributions
- 🧪 Testing and feedback

**How to contribute:**
1. Fork the repository
2. Create feature branch (`git checkout -b feature/amazing-feature`)
3. Commit changes (`git commit -m 'Add amazing feature'`)
4. Push to branch (`git push origin feature/amazing-feature`)
5. Open Pull Request

See [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines.

**Contact:**
- [GitHub Issues](https://github.com/yourusername/loragro/issues)
- Email: pavelmich.id@gmail.com
- Discord: Coming soon

---

## 📜 License

| Component         | License        |
| ----------------- | -------------- |
| **Firmware**      | Apache 2.0     |
| **Hardware**      | CERN-OHL-W-2.0 |
| **Documentation** | CC-BY-4.0      |

### Can I use this commercially?

**Yes!** You can manufacture, sell, and modify LoRaGro.

**You must:**
- Share PCB design improvements (CERN-OHL-W)
- Provide attribution
- Include license notices

**You don't need to:**
- Open-source your entire product
- Pay royalties

See [LICENSE](LICENSE) for details.

---

## 🙏 Acknowledgments

**Built with:**
- [Zephyr RTOS](https://www.zephyrproject.org/)
- [nRF Connect SDK](https://www.nordicsemi.com/Products/Development-software/nrf-connect-sdk)
- [KiCad](https://www.kicad.org/)

**Inspired by:**
- Open-source hardware movement
- Agricultural sustainability
- Affordable, repairable IoT

---

## 📞 Contact

- **GitHub:** [github.com/P4V77/Zephyr/LoRaGro](https://github.com/P4V77/Zephyr/LoRaGro)
- **Email:** pavelmich.id@gmail.com
- **Docs:** [docs/](docs/)

---

**Built with ❤️ for farmers, by makers.**