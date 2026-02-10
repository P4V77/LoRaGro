# Fino-LoRaGro Firmware 🌾

Basic skeleton for the Fino node firmware, running on Zephyr RTOS with native simulator (BSIM) support. Includes fake sensor drivers for environmental sensor logging and testing without hardware.

## Prerequisites

### 1. Zephyr SDK & Base (Required)

- Zephyr SDK 0.17.3 or later
- Zephyr base (tested with v4.2.0)
- West build system
- Python ≥ 3.10

Follow the [official Zephyr getting started guide](https://docs.zephyrproject.org/latest/develop/getting_started/index.html) to install these prerequisites.

### 2. BSIM (Board Simulator) Installation ⚠️ Mandatory

BSIM is not included by default in the Zephyr SDK and must be installed separately:

```bash
# 1. Clone the BSIM repository
git clone https://github.com/BabbleSim/ext_BSIM.git ~/bsim

# 2. Install BSIM dependencies and compile
cd ~/bsim
source setup_src.sh

# 3. Add BSIM to your PATH (add to ~/.bashrc or ~/.zshrc)
export PATH=~/bsim/bin:$PATH

# 4. Verify installation
which bsim_bt  # Should return a path
```

### 3. Environment Setup

Ensure these environment variables are set (add to your shell profile):

```bash
export ZEPHYR_BASE=/path/to/zephyr-base
export PATH=$HOME/bsim/bin:$PATH
```

## Building for Native Simulator (BSIM)

### Step 1: Build for nRF52 BSIM target

```bash
# Navigate to project directory
cd FW-LoRaGro/Fino-LoRaGro

# Build for BSIM
west build -b nrf52_bsim . -p \
    -- -DBOARD_ROOT="$ZEPHYR_BASE" \
       -DCONFIG_DEBUG_OPTIMIZATIONS=y \
       -DCONFIG_DEBUG_THREAD_INFO=y
```

**Note:**
- `$ZEPHYR_BASE` must point to your Zephyr installation directory
- Warnings about unknown vendor prefixes (like `p4v`) are expected for fake sensors
- Build artifacts will be in `build/nrf52_bsim/`

### Step 2: Run in BSIM

```bash
# zephyr.exe is uploaded to git, so you can run it without building
# Make sure you are in the FW-LoRaGro directory
# Execute the compiled firmware by running this in terminal:

./Fino-LoRaGro/build/nrf52_bsim/zephyr/zephyr.exe
```

**Expected output:**
```bash
?_??: WARNING: Neither simulation id or the device number have been set. I assume you want to run without a BabbleSim phy (-nosim)
?_??: WARNING: If this is not what you wanted, check with --help how to set them
d_00: @00:00:00.000000  [00:00:00.000,000] <inf> regulator_fake: === REGULATOR FAKE INIT START ===
d_00: @00:00:00.007470  [00:00:00.007,446] <inf> regulator_fake: Device: regulator-3v3
d_00: @00:00:00.013950  [00:00:00.013,946] <dbg> regulator_fake: fake_get_voltage: get_voltage: 3300000 uV
d_00: @00:00:00.022230  [00:00:00.022,216] <inf> regulator_fake: regulator_common_init returned: 0
d_00: @00:00:00.029790  [00:00:00.029,785] <inf> regulator_fake: === REGULATOR FAKE INIT END ===
d_00: @00:00:00.037170  [00:00:00.037,139] <inf> emul: Registering 1 emulator(s) for spi-emul@0
d_00: @00:00:00.044460  [00:00:00.044,433] <wrn> emul: Cannot find emulator for 'sx1262@0'
d_00: @00:00:00.051570  [00:00:00.051,544] <inf> sx1262_fake: Fake SX1262 initialized
d_00: @00:00:00.057960  [00:00:00.057,952] <inf> bh1750_fake: Fake BH1750 light sensor initialized
d_00: @00:00:00.065520  [00:00:00.065,490] <inf> bme280_fake: Fake BME280 Enviromental sensor initialized
d_00: @00:00:00.073710  [00:00:00.073,699] <inf> scd41_fake: Fake SCD41 CO2 sensor initialized
d_00: @00:00:00.080910  [00:00:00.080,902] <inf> soil_modbus: 3in1 Soil Modbus Sensor FAKE init (slave=1)
d_00: @00:00:00.089100  *** Booting Zephyr OS build v4.2.0 ***
d_00: @00:00:00.092700  [00:00:00.092,681] <inf> main: Starting application
d_00: @00:00:00.098276  [00:00:00.098,266] <inf> fs_nvs: 128 Sectors of 4096 bytes
d_00: @00:00:00.104309  [00:00:00.104,278] <inf> fs_nvs: alloc wra: 0, fe8
d_00: @00:00:00.109709  [00:00:00.109,680] <inf> fs_nvs: data wra: 0, 0
d_00: @00:00:00.114839  [00:00:00.114,837] <wrn> config_manager: No valid config found, loading defaults
d_00: @00:00:00.123209  [00:00:00.123,199] <inf> config_manager: Loaded default config
d_00: @00:00:00.130463  [00:00:00.130,462] <inf> config_manager: Config saved to NVS
d_00: @00:00:00.136673  [00:00:00.136,657] <inf> app: Device ID: 0
d_00: @00:00:00.141353  [00:00:00.141,326] <inf> app: Assigned Gateway ID: 0
d_00: @00:00:00.146933  [00:00:00.146,911] <inf> app: Registering sensors
d_00: @00:00:00.177582  [00:00:00.177,581] <inf> sx1262_fake: Fake SX1262 configured (SF=7 BW=0 CR=1)
d_00: @00:00:00.185322  [00:00:00.185,302] <inf> sample_manager: Type(Manager) 0 Measurement[0]: ID=0 value=22.500 ts=0
d_00: @00:00:00.194772  [00:00:00.194,763] <inf> sample_manager: Type(Manager) 0 Measurement[1]: ID=1 value=55.000 ts=0
d_00: @00:00:00.204222  [00:00:00.204,193] <inf> sample_manager: Type(Manager) 0 Measurement[2]: ID=2 value=101.325 ts=0
d_00: @00:00:00.213762  [00:00:00.213,745] <inf> sample_manager: Type(Manager) 1 Measurement[0]: ID=16 value=8500.000 ts=0
d_00: @00:00:00.223482  [00:00:00.223,480] <inf> sample_manager: Type(Manager) 2 Measurement[0]: ID=34 value=435.000 ts=0
d_00: @00:00:00.233112  [00:00:00.233,093] <inf> sample_manager: Type(Manager) 2 Measurement[1]: ID=32 value=22.050 ts=0
d_00: @00:00:00.242652  [00:00:00.242,645] <inf> sample_manager: Type(Manager) 2 Measurement[2]: ID=33 value=55.300 ts=0
d_00: @00:00:00.252192  [00:00:00.252,166] <inf> sample_manager: Type(Manager) 3 Measurement[0]: ID=49 value=52.300 ts=0
d_00: @00:00:00.261732  [00:00:00.261,718] <inf> sample_manager: Type(Manager) 3 Measurement[1]: ID=48 value=21.400 ts=0
d_00: @00:00:00.271272  [00:00:00.271,270] <inf> sample_manager: Type(Manager) 3 Measurement[2]: ID=50 value=812.000 ts=0
d_00: @00:00:00.280902  [00:00:00.280,883] <inf> sample_manager: Type(Manager) 4 Measurement[0]: ID=51 value=100.000 ts=0
d_00: @00:00:00.290532  [00:00:00.290,527] <inf> sample_manager: Type(Manager) 5 Measurement[0]: ID=64 value=2504.000 ts=0
d_00: @00:00:00.423279  [00:00:00.423,278] <inf> sx1262_fake: TX frame target=0 source=0 counter=0
d_00: @00:00:00.558777  [00:00:00.558,776] <inf> sx1262_fake: Fake RX returning ACK (consumed)
d_00: @00:00:00.565887  [00:00:00.565,856] <inf> app: Frame 0 ACKed
d_00: @00:00:00.570657  [00:00:00.570,648] <inf> sx1262_fake: Fake RX returning CONFIG frame
d_00: @00:00:00.577677  [00:00:00.577,667] <inf> config_manager: Config unchanged, skipping NVS write
d_00: @12:00:00.585511  [12:00:00.585,510] <wrn> power_manager: Battery low: 2592 mV
d_00: @12:00:00.591991  [12:00:00.591,979] <wrn> power_manager: Deep Sleep for: 12 hours
d_00: @24:00:00.598938  [24:00:00.598,937] <inf> power_manager: Battery recovered: 2636 mV
^C
Stopped at simulation time
```

**Key features shown in output:**

1. **Multiple Sensor Types:**
   - Type 0: Environmental (temperature, humidity, pressure)
   - Type 1: Light sensor (BH1750)
   - Type 2: Soil sensor (3-in-1 Modbus - EC, temperature, moisture)
   - Type 3: CO2 sensor (SCD41 - CO2, temperature, conductivity)
   - Type 4: Additional sensor type
   - Type 5: Battery voltage monitoring

2. **LoRa Communication:**
   - SX1262 radio transceiver simulation
   - Frame transmission with ACK reception
   - Configuration frame handling

3. **Power Management:**
   - Battery voltage monitoring
   - Automatic deep sleep when battery is low (< 2600 mV)
   - Recovery detection when battery voltage increases

4. **Configuration Management:**
   - NVS (Non-Volatile Storage) for persistent config
   - Device ID and Gateway ID assignment
   - Configuration frame handling over LoRa

5. **Time-stamped Measurements:**
   - All sensor readings include timestamps
   - Periodic sampling at configurable intervals

**Notes:**

- BSIM simulates Nordic nRF52 hardware in your terminal
- Multiple fake sensor drivers generate realistic values for testing
- Sensor values gradually change over simulated time
- Press Ctrl+C to stop the simulation
- The simulator runs in accelerated time (notice the @HH:MM:SS format)

## Project Structure

```
Fino-LoRaGro/
├── boards/             # Board-specific overlays and configs
├── build/              # Build directory (auto-generated)
├── configs/            # Zephyr Kconfig fragments
├── src/                # Application source code
│   ├── fake_bme280.c  # Fake BME280 driver (temp, humidity, pressure)
│   ├── fake_bh1750.c  # Fake BH1750 light sensor
│   ├── fake_scd41.c   # Fake SCD41 CO2 sensor
│   ├── fake_sx1262.c  # Fake LoRa transceiver
│   └── soil_modbus.c  # Fake 3-in-1 soil sensor
├── scripts/            # Helper scripts
├── CMakeLists.txt      # Zephyr CMake build configuration
├── prj.conf            # Main Zephyr project configuration
├── README.md           # This file
└── .gitignore
```

## Development Notes

### Fake Sensor Drivers

The firmware includes several fake sensor drivers that simulate realistic agricultural sensors:

- **BME280**: Temperature (°C), humidity (%), and atmospheric pressure (kPa)
- **BH1750**: Light intensity (lux)
- **SCD41**: CO2 concentration (ppm), temperature, conductivity
- **3-in-1 Soil Sensor**: Soil EC, temperature, and moisture via Modbus
- **SX1262**: LoRa transceiver for long-range wireless communication

All sensors:
- Implement standard Zephyr sensor APIs
- Generate simulated readings that change gradually over time
- Allow testing application logic without physical hardware
- Can be easily replaced with real drivers for hardware deployment

### Power Management Features

The simulator includes a power management system that:
- Monitors battery voltage
- Triggers deep sleep mode when voltage drops below threshold
- Simulates voltage recovery during sleep
- Resumes normal operation when battery recovers

### Building for Real Hardware

To build for actual hardware (e.g., nRF52840 DK):

```bash
# Example for nRF52840 DK
west build -b nrf52840dk_nrf52840 .

# Flash to device
west flash
```

## Troubleshooting

1. **BSIM not found:** Ensure `~/bsim/bin` is in your PATH and BSIM is compiled
2. **Zephyr not found:** Verify `ZEPHYR_BASE` environment variable is set correctly
3. **Build failures:** Clean build directory: `rm -rf build/`
4. **Emulator warnings:** Warnings about missing emulators (e.g., `sx1262@0`) are expected for fake drivers

## Quick Start Summary

1. Install Zephyr SDK and dependencies
2. Install BSIM separately (crucial step!)
3. Set environment variables
4. Build: `west build -b nrf52_bsim . -p`
5. Run: `./Fino-LoRaGro/build/nrf52_bsim/zephyr/zephyr.exe`

**Remember:** BSIM must be installed alongside Zephyr, not as part of it. The fake sensor drivers enable immediate testing in simulation before hardware integration.

## License

Apache 2.0

---

*Last updated: February 10, 2026*