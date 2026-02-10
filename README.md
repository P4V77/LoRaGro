# LoRaGro 🌾

![Build Status](https://img.shields.io/badge/build-in%20progress-yellow)
![License](https://img.shields.io/badge/license-Apache%202.0-blue)
![Platform](https://img.shields.io/badge/platform-Zephyr%20RTOS-orange)
![Hardware](https://img.shields.io/badge/hardware-nRF52840-blue)

**Solar-Powered Agricultural IoT Platform with Modular LoRa Nodes**

LoRaGro is an open-source agricultural monitoring platform designed for long-term autonomous field deployment using ultra-low-power LoRa nodes. The firmware is built on Zephyr RTOS, designed simulation-first (BSIM), and structured to run unchanged on real nRF52 hardware.

```
[FiNo Node] ──LoRa──> [GaNo Gateway] ──WiFi/Ethernet──> [Server/App]
     ↓                        ↓                                ↓
  Sensors                Bridge logic                    Data storage
  Solar + battery        Time sync                       Visualization
  Deep sleep             Buffering                       Alerts
```

> **🚧 Current Status:** Phase 1 – Firmware architecture & simulation  
> Hardware PCB design planned for Phase 2

---

## ✨ What Makes LoRaGro Different?

- 🔋 **Energy autonomous** — Solar + battery for indefinite operation
- 💤 **Battery-aware deep sleep** — Intelligent power management
- 🧠 **Modular sensor managers** — Devicetree-driven architecture
- 🧪 **Full native simulation** — nRF52 BSIM for development without hardware
- 🧩 **Single firmware** — One codebase for all node variants
- 📡 **Raw LoRa protocol** — No LoRaWAN dependency
- 💾 **Persistent configuration** — NVS storage
- 🔁 **Production-ready architecture** — Not a prototype sketch

**This is structured as deployable embedded firmware from day one.**

---

## 🧩 System Architecture

### Node Types

| Node     | Purpose             | Power           | Location       | Status       |
| -------- | ------------------- | --------------- | -------------- | ------------ |
| **FiNo** | Field sensing       | Battery + solar | Outdoor        | 🔨 Active dev |
| **GaNo** | Data gateway/bridge | Mains           | Farm buildings | 📅 Planned    |

### FiNo — Field Node

Autonomous agricultural sensing unit for long-term deployment.

**Power System:**
- Single 18650 Li-ion (3000mAh)
- Optional 5W solar panel for indefinite runtime
- Battery-aware sleep intervals
- Critical deep-sleep recovery mode
- Graceful brownout behavior
- Battery life: 6-12 months depending on configuration

**Communication:**
- LoRa uplink (868MHz EU / 915MHz US)
- Range: 2-10 km depending on terrain

**Enclosure:** IP67 waterproof (planned)

**Key feature:** Fully modular — sensors auto-detected via Devicetree

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

**All configurations start with capacitive sensors as the foundation.** Where your operation requires it, you can upgrade to a professional RS485 probe for enhanced precision and additional measurements.

---

### FIELD BASIC Configuration ⭐⭐⭐⭐⭐

**Purpose:** Universal foundation for agricultural monitoring

**Core sensors:**
- **Soil moisture** — Capacitive sensor (ADC)
- **Soil temperature** — DS18B20 (1-Wire)
- **Air temp/humidity/pressure** — BME280 (I²C)
- **Battery monitoring** — Built-in ADC

**What it measures:**
- When to irrigate (soil moisture)
- Frost warnings (air temperature)
- Disease risk (humidity + temperature)
- Planting timing (soil warmth)
- Weather patterns (pressure trends)

**Battery life:** ~12 months

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

**Battery life:** 3-6 months (backup mode)

**Best for:**

**Sampling intervals:**
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

---

## 🏗️ Technical Architecture

### Software Architecture (Actual Implementation)

```
Application
   ↓
SampleManager
   ↓
Sensor Managers (Devicetree-registered)
   ↓
Measurements
   ↓
LoRa TX (SX1262)
   ↓
PowerManagement (battery-aware sleep decision)
```

### Core Components

| Component       | Responsibility                        |
| --------------- | ------------------------------------- |
| Application     | Main orchestration loop               |
| SampleManager   | Aggregates sensor managers            |
| Sensor Adapters | Hardware abstraction                  |
| PowerManagement | Battery-aware sleep logic             |
| ConfigManager   | Persistent configuration (NVS)        |
| LoRaAuth        | Frame signing & counters              |
| SX1262          | LoRa driver (fake in sim, real on hw) |

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

---

## 🧪 Simulation-First Development

LoRaGro firmware runs in:

- 🖥 **nRF52 BSIM** (BabbleSim) — Full system simulation
- 🔌 **Real nRF52840 hardware** — Same firmware, no changes

**Fake drivers implemented for:**
- SX1262 LoRa transceiver
- BME280 environmental sensor
- BH1750 light sensor
- SCD41 CO₂ sensor
- RS485 soil probe (Modbus)
- ADC (battery + soil analog)

**This allows:**
- Full application testing without hardware
- Power logic verification
- Config persistence testing
- LoRa frame handling validation
- Rapid development iteration

---

## 🔋 Power Management

**Implemented logic:**
- Normal sampling interval
- Low battery interval
- Critical cutoff loop
- Recovery re-sampling after long sleep
- Single battery re-check instead of full batch (energy optimized)

**Example behavior:**

```
Battery OK → normal interval
Battery low → shorter interval
Battery critical → sleep N hours → re-sample battery only
Recovered → resume normal operation
```

**No unnecessary sensor sampling during recovery loop.**

**Battery monitoring:**
- Via ADC with manual millivolt conversion
- Zephyr `adc_raw_to_millivolts()` proved unreliable in emulation
- Manual conversion ensures deterministic behavior

---

## 📡 Communication

**SX1262 LoRa:**
- Custom lightweight protocol
- Frame counters
- ACK handling
- CMAC frame signing
- Security counter persistence

**Architecture prepared for:**
- Gateway time sync
- Downlink config updates
- OTA extension (future)

---

## 🔧 Technology Stack

### Firmware
- **RTOS:** Zephyr 4.x
- **SDK:** nRF Connect SDK
- **Language:** C++17
- **Build:** CMake + west
- **Features:**
  - Devicetree hardware abstraction
  - Deep sleep power management
  - NVS storage
  - Logging and debug shell
  - BSIM native simulation
  - OTA/DFU updates (planned)

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

**Production PCB:** Not yet designed (Phase 2)

---

## 🗺️ Roadmap

### ✅ Phase 1: Foundation (IN PROGRESS)
- [x] Project setup (GitHub, docs)
- [x] Zephyr RTOS integration
- [x] LoRa communication (SX1262 driver)
- [x] Modular sensor abstraction
- [x] Power management (deep sleep)
- [x] Configuration system (NVS)
- [x] CMAC frame signing
- [x] Fake driver layer for simulation
- [x] Devicetree-based sensor registration
- [ ] FiNo Native Sim FW completed
- [ ] Tests for FiNo Native Sim
- [ ] GaNo Native Sim FW completed 
- [ ] Tests for GaNo Native Sim
- [ ] Integration test scenarios

### 📅 Phase 2: Hardware Integration
- [ ] Pro Micro NRF52840 board integration
- [ ] Real sensor integration
- [ ] Solar validation
- [ ] Power profiling
- [ ] Mobile app development
- [ ] Building HW nodes from modules
- [ ] Field testing (2-3 locations)

### 📅 Phase 3: Production
- [ ] Production PCB design (50-100 units)
- [ ] IP67 enclosure design
- [ ] User documentation and tutorials
- [ ] Certification
- [ ] First commercial sales

### 📅 Phase 4: Expansion
- [ ] Additional sensor variants
- [ ] Cold chain monitoring
- [ ] Scale to 200-500 units

---

## 📁 Repository Structure

```
LoRaGro/
├── FW-LoRaGro/
│   ├── Fino-LoRaGro/      # FiNo node firmware
│   └── Gano-LoRaGro/      # GaNo gateway firmware (planned)
├── docs/                  # Documentation
├── hardware/              # PCB designs (future)
└── README.md              # This file
```

**Detailed firmware README available inside `FW-LoRaGro/Fino-LoRaGro/`**

---

## 🤝 Contributing

Contributions welcome! Help us build LoRaGro:

- 🐛 Bug reports
- 💡 Feature suggestions
- 📖 Documentation improvements
- 🔧 Code contributions
- 🧪 Testing and feedback
- 🏗️ Architecture discussion
- ⚡ Power optimization ideas

**How to contribute:**
1. Fork the repository
2. Create feature branch (`git checkout -b feature/amazing-feature`)
3. Commit changes (`git commit -m 'Add amazing feature'`)
4. Push to branch (`git push origin feature/amazing-feature`)
5. Open Pull Request

See [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines.

**Contact:**
- [GitHub Issues](https://github.com/P4V77/Zephyr/LoRaGro/issues)
- Email: pavelmich.id@gmail.com

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

## 💰 Estimated Pricing

**Note:** These are estimated costs based on component pricing and small-scale production (50-100 units). Final pricing will be confirmed during Phase 3.

### Configuration Pricing

| Configuration    | BOM Cost (Basic) | Target Price (Basic) | BOM Cost (Premium)  | Target Price (Premium) |
| ---------------- | ---------------- | -------------------- | ------------------- | ---------------------- |
| **FIELD BASIC**  | $35-45           | $60-80               | —                   | —                      |
| **ORCHARD**      | $45-60           | $75-110              | $100-145            | $155-220               |
| **VINEYARD**     | $65-85           | $95-135              | $120-165            | $175-250               |
| **GREENHOUSE**   | $65-90           | $100-150             | $120-175            | $180-260               |
| **COASTAL/ARID** | $90-130          | $140-200             | Required by default | Required by default    |

**Premium upgrade:** +$60-100 per node (RS485 3-in-1 soil probe)

**What affects pricing:**
- Component selection (basic vs premium sensors)
- Production volume (economies of scale)
- Enclosure choice (IP67 housing)
- Solar panel inclusion (optional)
- Bulk ordering discounts

**Deployment cost optimization:**
- Mix basic and premium nodes strategically
- Use premium sensors in critical monitoring zones
- Use basic sensors for general field coverage
- Example: 20 basic + 5 premium nodes = comprehensive coverage at lower total cost

---

## 📞 Contact

- **GitHub:** [github.com/P4V77/Zephyr/LoRaGro](https://github.com/P4V77/Zephyr/LoRaGro)
- **Email:** pavelmich.id@gmail.com
- **Docs:** [docs/](docs/)

---

**LoRaGro — modular, simulation-first, production-ready agricultural IoT. 🌾**

*Built with ❤️ for farmers, by makers.*