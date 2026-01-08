# Fino-LoRaGro Firmware 🌾

Basic skeleton for the Fino node firmware, running on Zephyr RTOS with native simulator (BSIM) support. Includes a fake sensor drivers for environmental sensor logging and testing without hardware.

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

**Expected output:**
?_??: WARNING: Neither simulation id or the device number have been set. I assume you want to run without a BabbleSim phy (-nosim)
?_??: WARNING: If this is not what you wanted, check with --help how to set them
d_00: @00:00:00.000000  [00:00:00.000,000] <inf> regulator_fake: === REGULATOR FAKE INIT START ===
d_00: @00:00:00.007470  [00:00:00.007,446] <inf> regulator_fake: Device: regulator-3v3
d_00: @00:00:00.013950  [00:00:00.013,946] <dbg> regulator_fake: fake_get_voltage: get_voltage: 3300000 uV
d_00: @00:00:00.022230  [00:00:00.022,216] <inf> regulator_fake: regulator_common_init returned: 0
d_00: @00:00:00.029790  [00:00:00.029,785] <inf> regulator_fake: === REGULATOR FAKE INIT END ===
d_00: @00:00:00.037170  [00:00:00.037,139] <inf> bme280_fake: Fake BME280 initialized
d_00: @00:00:00.043560  *** Booting Zephyr OS build v4.2.0 ***
d_00: @00:00:00.047160  LoRaGro FiNo simulator start
d_00: @00:00:00.049860  [00:00:00.049,835] <inf> main: Starting sensor application
d_00: @00:00:00.066010  [00:00:00.066,009] <inf> sample_manager: Sensor 0 measurement[0]: id=1000 type=1 value=25.65 ts=0
d_00: @00:00:00.075550  [00:00:00.075,531] <inf> sample_manager: Sensor 0 measurement[1]: id=1001 type=2 value=60.50 ts=0
d_00: @00:00:00.085180  [00:00:00.085,174] <inf> sample_manager: Sensor 0 measurement[2]: id=1002 type=3 value=101.32 ts=0
d_00: @00:15:00.104950  [00:15:00.104,949] <inf> sample_manager: Sensor 0 measurement[0]: id=1000 type=1 value=25.65 ts=900
d_00: @00:15:00.114670  [00:15:00.114,654] <inf> sample_manager: Sensor 0 measurement[1]: id=1001 type=2 value=60.50 ts=900
d_00: @00:15:00.124480  [00:15:00.124,450] <inf> sample_manager: Sensor 0 measurement[2]: id=1002 type=3 value=101.32 ts=900
d_00: @00:30:00.144440  [00:30:00.144,439] <inf> sample_manager: Sensor 0 measurement[0]: id=1000 type=1 value=25.65 ts=1800
d_00: @00:30:00.154250  [00:30:00.154,235] <inf> sample_manager: Sensor 0 measurement[1]: id=1001 type=2 value=60.50 ts=1800
^Cd_00: @00:30:00.158438  
Stopped at 1800.158s

Notes:

    BSIM simulates Nordic nRF52 hardware in your terminal.

    Fake BME280 sensor logs values as expected for testing.

    Press Ctrl+C to stop the simulation.

```
## Project Structure

```
Fino-LoRaGro/
├── boards/             # Board-specific overlays and configs
├── build/              # Build directory (auto-generated)
├── configs/            # Zephyr Kconfig fragments
├── src/                # Application source code
│   └── fake_bme280.c  # Fake BME280 driver for simulation
├── scripts/            # Helper scripts
├── CMakeLists.txt      # Zephyr CMake build configuration
├── prj.conf            # Main Zephyr project configuration
├── README.md           # This file
└── .gitignore
```

## Development Notes

### Fake BME280 Driver

- Currently implements a fake BME280 driver using Zephyr sensor APIs
- Generates simulated temperature, pressure, and humidity readings
- Easy to replace with real BME280 driver for hardware deployment
- Allows testing application logic without physical sensors

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

## Quick Start Summary

1. Install Zephyr SDK and dependencies
2. Install BSIM separately (crucial step!)
3. Set environment variables
4. Build: `west build -b nrf52_bsim . -p`
5. Run: `west build -t run`

**Remember:** BSIM must be installed alongside Zephyr, not as part of it. The fake BME280 driver enables immediate testing in simulation before hardware integration.

## License

Apache 2.0

---

*Last updated: January 8, 2026*