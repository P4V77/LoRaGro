# LoRaGro Protocol Specification v1.2

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
After any reset (power-on or 24h timeout), the synchronization is strictly controlled:
* **Initial State:** **RX Counter** = `0`, **TX Counter** = `1`.
* **First Contact:** The first frame received after a reset **MUST** have the `Frame Counter` LSB equal to `1`. Otherwise, it is rejected.

### 2.2 24-Hour Auto-Resync
* **Gateway:** If no valid frame is received from a Node for **> 24 hours**, it resets that Node's `last_rx_counter` to `0`.
* **Node:** If no valid ACK/Response is received for **> 24 hours**, it resets its `TX Counter` to `1`.

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
| Field         | Size | Byte Order | Description                                    |
| :------------ | :--- | :--------- | :--------------------------------------------- |
| **Header**    | 4 B  | —          | Type `0x02`.                                   |
| **Cmd Count** | 1 B  | —          | Number of commands in the frame.               |
| **Prot. Ver** | 1 B  | —          | Must match `PROTOCOL_VERSION` (1).             |
| **Commands**  | n B  | **LE**     | Variable length sequence (ID + Length + Data). |
| **CMAC Tag**  | 4 B  | —          | AES-CMAC signature.                            |

### 3.3 RESPONSE Frame (Uplink: Node → Gateway)
| Field           | Size | Byte Order | Description                         |
| :-------------- | :--- | :--------- | :---------------------------------- |
| **Header**      | 4 B  | —          | Type `0x5A`.                        |
| **Result Code** | 1 B  | —          | Execution status (see Section 3.4). |
| **CMAC Tag**    | 4 B  | —          | AES-CMAC signature.                 |

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
| CMD ID | Name                  | Payload Size | Byte Order | Description                                          |
| :----- | :-------------------- | :----------- | :--------- | :--------------------------------------------------- |
| `0x00` | **SET_COMBINED_ID**   | 2 B          | **LE**     | Set new device combined ID (Gateway ID + Node ID).   |
| `0x01` | **SAMPLING_INTERVAL** | 1 B          | —          | Set sample interval in minutes.                      |
| `0x02` | **REBOOT**            | 0 B          | —          | Trigger immediate device reboot.                     |
| `0x03` | **SET_UNIX_TIME**     | 8 B          | **LE**     | Sync RTC — Unix timestamp in seconds.                |
| `0x04` | **LORA_CONFIG**       | 10 B         | **LE**     | Reconfigure LoRa radio parameters (see Section 4.2). |

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

## 7. Node Cycle & TDMA Time Multiplexing

### 7.1 Node Run Cycle

Each node executes the following sequence once per sleep cycle:

```
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
node_id             = combined_id >> 5   (lower 11 bits)
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
| SF11 | ≈ 4.92 s           | ~11 km       |        **≈ 183** |        **≈ 366** |        **≈ 732** |
| SF12 | ≈ 9.75 s           | ~15 km       |         **≈ 92** |        **≈ 185** |        **≈ 369** |

> Range estimates are indicative values for open terrain with a typical gateway antenna. Actual range depends on environment, antenna gain, and obstacles.

> ⚠️ **SF12 capacity warning:** For deployments exceeding **91 nodes on SF12 with a 15-minute cycle**, TDMA slots will overlap. Use the formula below to calculate the required interval:
>
> ```
> required_interval_min = ceil(node_count × node_tdma_window / 60)
> ```
>
> Example: 150 nodes on SF12 → `ceil(150 × 9.75 / 60)` = **25 minutes minimum**.

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