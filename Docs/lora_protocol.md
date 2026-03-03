# LoRaGro Protocol Specification v1.1

## 1. Frame Architecture
The protocol uses a fixed-size header for efficiency, followed by a variable payload and a mandatory security tag. All 16-bit fields are transmitted in **Big-Endian** (Network Byte Order).

### 1.1 Common Header (4 Bytes)
| Offset | Size | Field             | Description                                                      |
| :----- | :--- | :---------------- | :--------------------------------------------------------------- |
| 0      | 2 B  | **Target ID**     | Destination device ID (Gateway: 5 bits \| Node: 11 bits).        |
| 2      | 1 B  | **Frame Type**    | `0x01` (DATA), `0x02` (CONFIG), `0xA5` (ACK), `0x5A` (RESPONSE). |
| 3      | 1 B  | **Frame Counter** | Least significant 8 bits of the internal 32-bit counter.         |

> **Special Address:** `Target ID = 0xFFFF` is reserved for **Broadcast**. No ACK is expected or sent for broadcast frames.

---

## 2. Counter Management & Auto-Resync

### 2.1 The "Rule of One" (Reset Logic)
After any reset (power-on or 24h timeout), the synchronization is strictly controlled:
* **Initial State:** **RX Counter** = `0`, **TX Counter** = `1`.
* **First Contact:** The first frame received after a reset **MUST** have the `Frame Counter` LSB equal to `1`. Otherwise, it is rejected.

### 2.2 24-Hour Auto-Resync
* **Gateway:** If no valid frame is received from a Node for **> 24 hours**, it resets that Node's `last_rx_counter` to `0`.
* **Node:** If no valid ACK/Response is received for **> 24 hours**, it resets its `TX Counter` to `1`.
* This ensures both sides converge back to a clean state even after long-term signal loss.

---

## 3. Detailed Message Structures

### 3.1 DATA Frame (Uplink: Node → Gateway)
| Field           | Size    | Description                                       |
| :-------------- | :------ | :------------------------------------------------ |
| **Header**      | 4 B     | Type `0x01`                                       |
| **Batch Count** | 1 B     | Number of measurement entries                     |
| **Timestamp**   | 4 B     | Unix Epoch of the first measurement               |
| **Payload**     | n * 5 B | Measurement entries (Sensor ID + Value1 + Value2) |
| **CMAC Tag**    | 4 B     | AES-CMAC signature                                |

### 3.2 CONFIG Frame (Downlink: Gateway → Node)
| Field         | Size | Description                                    |
| :------------ | :--- | :--------------------------------------------- |
| **Header**    | 4 B  | Type `0x02`.                                   |
| **Cmd Count** | 1 B  | Number of commands in the frame.               |
| **Prot. Ver** | 1 B  | Must match `PROTOCOL_VERSION` (1).             |
| **Commands**  | n B  | Variable length sequence (ID + Length + Data). |
| **CMAC Tag**  | 4 B  | AES-CMAC signature.                            |

### 3.3 RESPONSE Frame (Uplink: Node → Gateway)
| Field           | Size | Description                         |
| :-------------- | :--- | :---------------------------------- |
| **Header**      | 4 B  | Type `0x5A`.                        |
| **Result Code** | 1 B  | Execution status (see Section 3.4). |
| **CMAC Tag**    | 4 B  | AES-CMAC signature.                 |

### 3.4 Result Codes (Response Frame)
| Code   | Name                  | Description                        |
| :----- | :-------------------- | :--------------------------------- |
| `0x00` | **OK**                | Command executed successfully.     |
| `0x01` | **INVALID_LENGTH**    | Payload size mismatch.             |
| `0x02` | **DIFFERENT_ID**      | Frame addressed to another device. |
| `0x03` | **PROTOCOL_MISMATCH** | Unsupported protocol version.      |
| `0x04` | **UNKNOWN_COMMAND**   | Unrecognized Command ID.           |
| `0x05` | **FLASH_FAILED**      | Error writing to NVM/Flash.        |
| `0x06` | **OK_REBOOT**         | Success, device will now reboot.   |

---

## 4. CONFIG Commands

Each command inside a CONFIG frame is encoded as: `[CMD_BYTE] [PAYLOAD...]`

The `CMD_BYTE` encodes both the command ID and payload size.

### 4.1 Command Table
| CMD ID | Name                  | Payload Size | Description                                          |
| :----- | :-------------------- | :----------- | :--------------------------------------------------- |
| `0x00` | **SET_COMBINED_ID**   | 2 B          | Set new device combined ID (Gateway ID + Node ID).   |
| `0x01` | **SAMPLING_INTERVAL** | 1 B          | Set sample interval in minutes.                      |
| `0x02` | **REBOOT**            | 0 B          | Trigger immediate device reboot.                     |
| `0x03` | **SET_UNIX_TIME**     | 8 B          | Sync RTC — Unix timestamp in seconds, Little-Endian. |
| `0x04` | **LORA_CONFIG**       | 10 B         | Reconfigure LoRa radio parameters (see Section 4.2). |

### 4.2 LORA_CONFIG Payload Layout (10 Bytes, Little-Endian)
| Offset | Size | Field             | Description                                 |
| :----- | :--- | :---------------- | :------------------------------------------ |
| 0      | 4 B  | **Frequency**     | Center frequency in Hz (e.g. 868100000).    |
| 4      | 1 B  | **Bandwidth**     | `lora_signal_bandwidth` enum value.         |
| 5      | 1 B  | **Datarate (SF)** | `lora_datarate` enum value (SF7–SF12).      |
| 6      | 1 B  | **Coding Rate**   | `lora_coding_rate` enum value (CR 4/5–4/8). |
| 7      | 1 B  | **Preamble Len**  | LoRa preamble length in symbols.            |
| 8      | 1 B  | **TX Power**      | Transmit power in dBm (signed).             |
| 9      | 1 B  | **Flags**         | Bit 0: TX mode. Bit 1: IQ inverted.         |

> Commands `SET_COMBINED_ID`, `REBOOT`, and `LORA_CONFIG` trigger a device reboot after saving to NVS.

---

## 5. Security (AES-CMAC)
Every frame is authenticated using the following logic:

* **Key:** `device_key` (derived from `MASTER_KEY` and `Device_ID`).
* **Calculation:**
```
CMAC = AES-CMAC(device_key, [32-bit counter || header || payload])
Tag  = first 4 bytes of CMAC result
```
Verification: The tag is verified against a 32-bit reconstructed counter to prevent replay attacks.

---

## 6. Payload Limits by Spreading Factor (SF)

| Spreading Factor | Max Frame Size | Usable Payload (Header + Tag excluded) |
| :--------------- | :------------- | :------------------------------------- |
| **SF7 - SF8**    | 242 B          | **234 B**                              |
| **SF9 - SF10**   | 115 B          | **107 B**                              |
| **SF11 - SF12**  | 51 B           | **43 B**                               |

---

## 7. TX Time Multiplexing

To prevent simultaneous transmissions from multiple nodes sharing the same gateway, each node adds a **sleep offset** derived from its Node ID and the estimated LoRa airtime.

### 7.1 Sleep Offset Calculation

```
sleep_offset_s = airtime_s × air_time_margin_factor × rx_time_window_margin × node_id
```

Where:
* `airtime_s` — estimated on-air time of the **maximum payload** frame for the configured SF (see Section 6).
* `air_time_margin_factor` — configurable safety multiplier (default: `1.3`).
* `rx_time_window_margin` — RX window margin (default: `2.0`, sized for max payload + ACK round-trip).
* `node_id` — lower 11 bits of `combined_id >> 5`.

The total offset is added to the configured `sample_interval_minutes` before the node goes to sleep.

### 7.2 Airtime Estimation

Airtime is calculated using standard LoRa modulation parameters:

```
T_sym     = 2^SF / BW
T_preamble = (preamble_len + 4.25) × T_sym
DE        = 1 if SF ≥ 11, else 0
payload_symbols = 8 + ceil((8×L - 4×SF + 44) / (4×(SF - 2×DE))) × (CR + 4)
T_packet  = T_preamble + payload_symbols × T_sym
```

Where `L` is the maximum payload length for the configured SF (see Section 6).

### 7.3 Battery-Aware Sleep

The sleep interval is further adjusted based on battery voltage:

| Battery State                           | Behaviour                                                  |
| :-------------------------------------- | :--------------------------------------------------------- |
| `>= battery_critical_mv`                | Normal `sample_interval_minutes` + TX offset.              |
| `< battery_critical_mv`                 | Reduced `sample_interval_min_low_battery` + TX offset.     |
| `< battery_cutoff_mv`                   | Deep sleep loop of `critically_low_battery_timeout_hours`. |
| Battery recovers `>= battery_cutoff_mv` | Resumes normal operation.                                  |