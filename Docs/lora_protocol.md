# LoRaGro Protocol Specification v1.3

**Last Updated:** March 2026

---

## 1. Frame Architecture

The protocol uses a fixed-size header for efficiency, followed by a variable payload and a mandatory security tag. All multi-byte fields are transmitted in **Little-Endian** byte order unless stated otherwise.

### 1.1 Common Header (4 Bytes)
| Offset | Size | Field             | Description                                                      |
| :----- | :--- | :---------------- | :--------------------------------------------------------------- |
| 0      | 2 B  | **Target ID**     | Destination device ID (Gateway: 5 bits \| Node: 11 bits). **LE** |
| 2      | 1 B  | **Frame Type**    | `0x01` (DATA), `0x02` (CONFIG), `0xA5` (ACK), `0x5A` (RESPONSE). |
| 3      | 1 B  | **Frame Counter** | Least significant 8 bits of the internal 32-bit counter.         |

> **Special Address:** `Target ID = 0xFFFF` is reserved for **Broadcast**. No ACK is expected or sent for broadcast frames.

---

## 2. Counter Management & Auto-Resync

### 2.1 The "Rule of One" (Reset Logic)
After any reset (power-on or 16h timeout), the synchronization is strictly controlled:
* **Initial State:** **RX Counter** = `0`, **TX Counter** = `1`.
* **First Contact:** The first frame received after a reset **MUST** have the `Frame Counter` LSB equal to `1`. Otherwise, it is rejected.

### 2.2 16-Hour Auto-Resync
* **Gateway:** If no valid frame is received from a Node for **> 16 hours**, it resets that Node's `last_rx_counter` to `0`.
* **Node:** If no valid ACK/Response is received for **> 16 hours**, it resets its `TX Counter` to `1`.

> **Note:** The 16-hour timeout is chosen to be safely above the maximum low-battery deep sleep duration (12 hours), preventing false counter resets after long battery recovery sleeps.

### 2.3 Counter Persistence (NVS)
To survive power loss, counters are persisted to NVS with a write threshold to minimize flash wear:

| Counter      | NVS Write Threshold | Rationale                             |
| :----------- | :------------------ | :------------------------------------ |
| `tx_counter` | Every 64 frames     | ~1 write/day at 15 min interval       |
| `rx_counter` | Every 16 frames     | CONFIG frames are rare; low wear risk |

After a power loss, the node resumes from the last saved threshold value. The maximum counter gap after recovery is bounded by the threshold, which is well within replay protection tolerance.

---

## 3. Detailed Message Structures

### 3.1 DATA Frame (Uplink: Node → Gateway)
| Field           | Size    | Byte Order | Description                         |
| :-------------- | :------ | :--------- | :---------------------------------- |
| **Header**      | 4 B     | —          | Type `0x01`                         |
| **Batch Count** | 1 B     | —          | Number of measurement entries       |
| **Timestamp**   | 4 B     | **LE**     | Unix Epoch of the first measurement |
| **Payload**     | n * 5 B | **LE**     | Measurement entries (see 3.1.1)     |
| **CMAC Tag**    | 4 B     | —          | AES-CMAC signature                  |

#### 3.1.1 Measurement Entry (5 Bytes)
| Offset | Size | Byte Order | Description            |
| :----- | :--- | :--------- | :--------------------- |
| 0      | 1 B  | —          | Sensor ID              |
| 1      | 2 B  | **LE**     | Value 1 (int16, ÷1000) |
| 3      | 2 B  | **LE**     | Value 2 (int16, ÷1000) |

### 3.2 CONFIG Frame (Downlink: Gateway → Node)
| Field         | Size | Byte Order | Description                                 |
| :------------ | :--- | :--------- | :------------------------------------------ |
| **Header**    | 4 B  | —          | Type `0x02`.                                |
| **Cmd Count** | 1 B  | —          | Number of commands in the frame.            |
| **Prot. Ver** | 1 B  | —          | Must match `PROTOCOL_VERSION` (1).          |
| **Commands**  | n B  | **LE**     | Variable length sequence (CMD_BYTE + Data). |
| **CMAC Tag**  | 4 B  | —          | AES-CMAC signature.                         |

### 3.3 ACK Frame (Downlink: Gateway → Node)
| Field        | Size | Byte Order | Description                                       |
| :----------- | :--- | :--------- | :------------------------------------------------ |
| **Header**   | 4 B  | —          | Type `0xA5`. Frame Counter echoes the TX counter. |
| **CMAC Tag** | 4 B  | —          | AES-CMAC signature (no replay protection on ACK). |

> ACK frames do **not** use replay protection — only CMAC authenticity is verified.

### 3.4 RESPONSE Frame (Uplink: Node → Gateway)
| Field           | Size | Byte Order | Description                         |
| :-------------- | :--- | :--------- | :---------------------------------- |
| **Header**      | 4 B  | —          | Type `0x5A`.                        |
| **Result Code** | 1 B  | —          | Execution status (see Section 3.5). |
| **CMAC Tag**    | 4 B  | —          | AES-CMAC signature.                 |

### 3.5 Result Codes (Response Frame)

These values match the `DecodeResult` enum in firmware.

| Code   | Enum Name              | Description                           |
| :----- | :--------------------- | :------------------------------------ |
| `0x00` | **OK**                 | Command executed successfully.        |
| `0x01` | **OK_AND_REBOOT_NEED** | Success, device will reboot after TX. |
| `0x02` | **PROTOCOL_MISMATCH**  | Unsupported protocol version.         |
| `0x03` | **INVALID_LENGTH**     | Payload size mismatch.                |
| `0x04` | **UNKNOWN_COMMAND**    | Unrecognized Command ID.              |
| `0x05` | **EXECUTED_REBOOT**    | Reboot command executed.              |
| `0x06` | **DIFFERENT_ID**       | Frame addressed to another device.    |
| `0x07` | **FLASH_FAILED**       | Error writing to NVM/Flash.           |
| `0x08` | **AUTH_FAILED**        | CMAC verification failed.             |

---

## 4. CONFIG Commands

Each command inside a CONFIG frame is encoded as: `[CMD_BYTE] [PAYLOAD...]`

The `CMD_BYTE` encodes both the command ID and payload size in a single byte:

```
CMD_BYTE = (cmd_id << 2) | encoded_size

encoded_size encoding:
  0x00 → 1 byte payload
  0x01 → 2 bytes payload
  0x02 → 4 bytes payload
  0x03 → reserved (invalid)
```

> **Example:** `SET_COMBINED_ID` (id=0x00, payload=2B) → `CMD_BYTE = (0x00 << 2) | 0x01 = 0x01`

### 4.1 Command Table
| CMD ID | Name                  | CMD_BYTE | Payload Size | Byte Order | Triggers Reboot | Description                                        |
| :----- | :-------------------- | :------- | :----------- | :--------- | :-------------- | :------------------------------------------------- |
| `0x00` | **SET_COMBINED_ID**   | `0x01`   | 2 B          | **LE**     | ✅ Yes           | Set new device combined ID (Gateway ID + Node ID). |
| `0x01` | **SAMPLING_INTERVAL** | `0x05`   | 2 B          | **LE**     | ❌ No            | Set sample interval in minutes (uint16).           |
| `0x02` | **REBOOT**            | `0x08`   | 0 B          | —          | ✅ Yes           | Trigger immediate device reboot.                   |
| `0x03` | **SET_UNIX_TIME**     | `0x0F`   | 8 B          | **LE**     | ❌ No            | Sync RTC — Unix timestamp in seconds (uint64).     |
| `0x04` | **LORA_CONFIG**       | `0x12`   | 10 B         | **LE**     | ✅ Yes           | Reconfigure LoRa radio parameters (see 4.2).       |

> **Note on REBOOT:** `REBOOT` has 0-byte payload — `encoded_size` field is unused. Gateway must send `CMD_BYTE = 0x08` (cmd_id=2, size bits=0 — interpreted as no payload by decoder).

### 4.2 LORA_CONFIG Payload Layout (10 Bytes, Little-Endian)
| Offset | Size | Field             | Description                                 |
| :----- | :--- | :---------------- | :------------------------------------------ |
| 0      | 4 B  | **Frequency**     | Center frequency in Hz (e.g. `868100000`).  |
| 4      | 1 B  | **Bandwidth**     | `lora_signal_bandwidth` enum value.         |
| 5      | 1 B  | **Datarate (SF)** | `lora_datarate` enum value (SF7–SF12).      |
| 6      | 1 B  | **Coding Rate**   | `lora_coding_rate` enum value (CR 4/5–4/8). |
| 7      | 1 B  | **Preamble Len**  | LoRa preamble length in symbols.            |
| 8      | 1 B  | **TX Power**      | Transmit power in dBm (signed).             |
| 9      | 1 B  | **Flags**         | Bit 0: TX mode. Bit 1: IQ inverted.         |

---

## 5. Security (AES-CMAC)

Every frame is authenticated using the following logic:

### 5.1 Key Derivation
```
device_key = AES-128(MASTER_KEY, combined_id || padding)
```
The device key is derived once per boot (or after ID change) and cached in RAM. It is never transmitted.

### 5.2 CMAC Calculation
```
CMAC = AES-CMAC(device_key, [32-bit counter || header || payload])
Tag  = first 4 bytes of CMAC result
```

### 5.3 Replay Protection
* **DATA / CONFIG frames:** Full replay protection using a 32-bit reconstructed counter. Frames with counter ≤ `last_rx_counter` are rejected.
* **ACK frames:** CMAC authenticity check only — no replay protection (ACKs are ephemeral responses).

### 5.4 Counter Reconstruction
The wire carries only the lower 8 bits of the counter. The receiver reconstructs the full 32-bit value:
```
if (lower_8 > (last_rx & 0xFF)):
    reconstructed = (last_rx & 0xFFFFFF00) | lower_8
else:
    reconstructed = (last_rx & 0xFFFFFF00) + 0x100 | lower_8
```
This handles 8-bit overflow transparently.

---

## 6. Payload Limits by Spreading Factor (SF)

| Spreading Factor | Max Frame Size | Usable Payload (Header + Tag excluded) |
| :--------------- | :------------- | :------------------------------------- |
| **SF7 - SF8**    | 242 B          | **234 B**                              |
| **SF9 - SF10**   | 115 B          | **107 B**                              |
| **SF11 - SF12**  | 51 B           | **43 B**                               |

---

## 7. Node Cycle & TDMA Time Multiplexing

### 7.1 Node Run Cycle

Each node executes the following sequence once per sleep cycle:

```
cfg.load()
powerOn()
  → sample_all()
  → TX: up to 3 DATA frames (confirmed, with ACK each)
  → RX: 1 CONFIG frame (optional, max payload)
      → TX: RESPONSE frame (if CONFIG received)
powerOff()
  → save config to NVS
  → handle_sleep()
```

**TX is hard-limited to 3 DATA frames per cycle.** If more measurements are pending they are dropped with an error log. This bound is required by the TDMA window definition (see Section 7.2).

**Config is reloaded at the start of every cycle** to pick up any changes saved in the previous cycle (e.g. after SET_COMBINED_ID).

### 7.2 TDMA Window & Sleep Offset

To prevent collisions each node is assigned a fixed time slot based on its Node ID.

#### Window Composition

```
tx_window  = 3 × airtime(max_payload)    // 3 TX DATA frames
rx_window  = 1 × airtime(max_payload)    // 1 RX CONFIG frame
ack_window = 3 × airtime(ACK_FRAME_SIZE) // 1 ACK per TX frame

node_tdma_window = (tx_window + rx_window + ack_window) × air_time_margin_factor
```

#### Sleep Offset

```
node_id             = combined_id & 0x07FF   (lower 11 bits)
sleep_time_offset_s = node_tdma_window × node_id
```

The offset is added to the configured sleep interval before the node goes to sleep.

#### Default Parameters

| Parameter                 | Default | Description                           |
| :------------------------ | :------ | :------------------------------------ |
| `air_time_margin_factor`  | `1.3`   | Safety multiplier on window size.     |
| `sample_interval_minutes` | `15`    | Base sleep interval (normal battery). |

### 7.3 Network Capacity by Spreading Factor

The maximum number of nodes is bounded by:

```
max_nodes = sample_interval_s / node_tdma_window
```

Calculated for BW = 125 kHz, CR = 4/5, preamble = 8, payload = 51 B, margin = 1.3, 3 TX + 1 RX + 3 ACK per cycle:

| SF   | TDMA window / node | Range (est.) | Max nodes 15 min | Max nodes 30 min | Max nodes 60 min |
| :--- | :----------------- | :----------- | ---------------: | ---------------: | ---------------: |
| SF7  | ≈ 0.38 s           | ~2 km        |       **≈ 2368** |       **≈ 4736** |       **≈ 9473** |
| SF8  | ≈ 0.70 s           | ~4 km        |       **≈ 1286** |       **≈ 2571** |       **≈ 5143** |
| SF9  | ≈ 1.27 s           | ~6 km        |        **≈ 709** |       **≈ 1417** |       **≈ 2835** |
| SF10 | ≈ 2.46 s           | ~8 km        |        **≈ 366** |        **≈ 732** |       **≈ 1463** |
| SF11 | ≈ 4.92 s           | ~11-15 km    |        **≈ 183** |        **≈ 366** |        **≈ 732** |
| SF12 | ≈ 9.75 s           | <20 km       |         **≈ 92** |        **≈ 185** |        **≈ 369** |

> Range estimates are indicative values for open terrain with a typical gateway antenna. Actual range depends on environment, antenna gain, and obstacles.

> ⚠️ **SF12 capacity warning:** For deployments exceeding **91 nodes on SF12 with a 15-minute sample interval**, TDMA slots will overlap. Use the formula below to calculate the required interval:
>
> ```
> required_interval_min = ceil(node_count × node_tdma_window / 60)
> ```
>
> Example: 150 nodes on SF12 → `ceil(150 × 9.75 / 60)` = **25 minutes minimum sample interval**.

### 7.4 Airtime Estimation

```
T_sym      = 2^SF / BW
T_preamble = (preamble_len + 4.25) × T_sym
DE         = 1 if SF ≥ 11, else 0
payload_symbols = 8 + ceil((8×L - 4×SF + 44) / (4×(SF - 2×DE))) × (CR + 4)
T_packet   = T_preamble + payload_symbols × T_sym        [seconds]
```

Where `L` is the payload length passed to `calculate_airtime_s()`.

### 7.5 Battery-Aware Sleep

| Battery State                           | Behaviour                                                  |
| :-------------------------------------- | :--------------------------------------------------------- |
| `>= battery_critical_mv`                | Normal `sample_interval_minutes` + TDMA offset.            |
| `< battery_critical_mv`                 | Reduced `sample_interval_min_low_battery` + TDMA offset.   |
| `< battery_cutoff_mv`                   | Deep sleep loop of `critically_low_battery_timeout_hours`. |
| Battery recovers `>= battery_cutoff_mv` | Resumes normal operation.                                  |

If no battery sensor is available or measurement fails, the node falls back to `sample_interval_minutes` without TDMA offset.

---

## 8. Combined ID Format

The `combined_id` is a 16-bit value encoding both the Gateway ID and Node ID:

```
combined_id[15:11] = Gateway ID  (5 bits,  values 0–31)
combined_id[10:0]  = Node ID     (11 bits, values 0–2047)
```

Helper functions:
```cpp
uint8_t  extract_gateway(uint16_t combined_id) { return (combined_id >> 11) & 0x1F; }
uint16_t extract_node(uint16_t combined_id)    { return combined_id & 0x07FF; }
```

---

## Document History

| Version | Date          | Changes                                                                                                                                                                                               |
| ------: | ------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
|     1.0 | December 2025 | Initial protocol specification                                                                                                                                                                        |
|     1.2 | February 2026 | Add TDMA, airtime, network capacity, battery-aware sleep                                                                                                                                              |
|     1.3 | March 2026    | Fix result codes to match firmware enum, add CMD_BYTE encoding table, add ACK frame structure, add counter persistence, add counter reconstruction, fix resync timeout to 16h, add combined ID format |