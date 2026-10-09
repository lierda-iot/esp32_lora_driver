# ESP LoRa Driver

Language: [中文](README_CN.md) | English

Repository: <https://github.com/lierda-iot/esp32_lora_driver.git>

`ESP LoRa Driver` is an ESP-IDF component for Semtech LoRa radio chips. The current primary target is `LR20xx / LR2021`. The component exposes layered `RAC`, `RALF`, and `RAL` interfaces, and is intended to expand to `SX126x`, `LR11xx`, and other chip families later.

**High-speed FLRC**: with this driver, `LR2021` sends and receives FLRC at a raw bit rate of 2.6 Mbit/s (`RAL_FLRC_RAW_BIT_RATE_2_600_MBPS`). Without coding (`RAL_FLRC_CR_1_1`, which is `LR20XX_RADIO_FLRC_CR_NONE` in the chip driver), the measured application payload rate is close to 2.2 Mbit/s, not counting protocol overhead such as retransmissions and gaps between packets.

This component is based on Semtech's upstream `smtc_rac_lib`, then adapted, trimmed, and ported to the ESP-IDF environment. It keeps the upstream layered abstraction while adding ESP-specific HAL, GPIO, SPI, timer, and logging adaptation.

## Goals

This is not just a single low-level driver file. It is a layered radio access component library designed to:

- Provide radio driver capability for `LR20xx / LR2021` in ESP-IDF projects
- Preserve Semtech's `RAC / RALF / RAL` layering to simplify maintenance and upstream sync
- Support both high-level scheduled access and lower-level direct driver access
- Keep a unified structure for future support of other Semtech radio chips

## Layered Architecture

From the component perspective, the code is mainly split into these layers:

- `RAC API`: high-level radio access API for application use, suitable for unified scheduling, task submission, and transaction management
- `RAC`: radio access control layer responsible for task organization, scheduling, TX/RX flow, and transaction context
- `Radio Planner`: arbitration and scheduling layer for queueing, priorities, and timing control across radio tasks
- `RALF`: radio feature layer built on top of `RAL`
- `RAL`: radio abstraction layer that provides a uniform upper API and connects to concrete chip implementations below
- `Radio Driver`: register-level and command-level chip drivers
- `Radio HAL / Modem HAL`: platform-specific support for SPI, GPIO, interrupts, timing, and logging

That means the component can be used either as a complete radio access stack or by consuming only a specific layer.

## Typical Use Cases

This component is suitable for:

- Quickly integrating an `LR2021` radio in an ESP-IDF project
- Scheduling LoRa, FSK, FLRC, or LR-FHSS radio tasks through a unified interface
- Managing radio transactions directly through the `RAC` layer
- Accessing `RAL` or `RALF` directly for finer-grained radio control

## Supported Lierda Products

The component currently targets these Lierda product forms:

- `L-LRMAM36-FANN4`
- `L-LRMWP35-FANN4`, an `ESP-IDF + LR2021 SPI` development solution

## Public Interface

The component uses a single public entry header:

- [include/LiotLr2021.h](include/LiotLr2021.h)

Recommended include:

```c
#include "LiotLr2021.h"
```

`LiotLr2021.h` aggregates the public interfaces for:

- `RAC API`
  - from [smtc_rac_api.h](rac_api/smtc_rac_api.h)
  - suitable for application-level radio transactions and unified scheduling
- `RAC`
  - from [smtc_rac.h](rac/smtc_rac.h)
  - suitable when directly operating the radio access control layer
- `RALF`
  - from [ralf.h](ralf/ralf.h)
  - suitable for radio feature-layer usage
- `RAL`
  - from [ral.h](ral/ral.h)
  - suitable for low-level radio abstraction access

So although users only need one public header, they can still directly call APIs from different layers when needed.

This design keeps external usage simple while preserving the upstream-style internal layering for maintenance and future sync.

## Directory Layout

The repository is organized roughly as follows:

- `include/`
  - single public entry header
- `rac_api/`
  - high-level application-facing API
- `rac/`
  - radio access control core
- `radio_planner/`
  - radio task scheduler
- `ral/`
  - radio abstraction layer
- `ralf/`
  - radio feature layer
- `radio_drivers/`
  - chip-specific driver implementations
- `radio_hal/`
  - chip-related platform adaptation
- `modem_hal/`
  - ESP-IDF low-level HAL and debug support
- `Kconfig`
  - component configuration options
- `idf_component.yml`
  - component metadata

## Configuration

The component exposes configuration through [Kconfig](Kconfig), including:

- radio chip family selection
  - `SX126x`
  - `LR11xx`
  - `LR20xx`
- SPI host selection for `LR2021`
- `NRST`, `BUSY`, `NSS`, `MOSI`, `MISO`, `SCLK`, and related GPIO assignment for `L-LRMWP35-FANN4` or `Generic / Custom LR2021 SPI board`
- `LR20xx` `DIO` configuration for `L-LRMWP35-FANN4` or `Generic / Custom LR2021 SPI board`

The default target is currently `LR20xx`, which serves as the default base configuration for `LR2021`.

## Logging

The component contains three major logging groups:

- `RAC Core` logs
  - located in [smtc_rac.c](rac/smtc_rac.c)
  - used for `RAC` initialization, context setup, scheduling flow, and API tracing
- `RAC business` logs
  - mainly in `LoRa`, `FLRC`, `LR-FHSS`, and related `RAC` business files
  - emitted through [smtc_rac_log.h](rac_api/smtc_rac_log.h)
- `ESP_LOG` logs
  - mainly in `LR20xx RAL / RALF` and `modem_hal`
  - emitted through `ESP_LOGI`, `ESP_LOGD`, and `ESP_LOGE`

### 1. RAC Core Logs

`RAC Core` uses:

- `RAC_CORE_LOG_ERROR`
- `RAC_CORE_LOG_CONFIG`
- `RAC_CORE_LOG_INFO`
- `RAC_CORE_LOG_WARN`
- `RAC_CORE_LOG_DEBUG`
- `RAC_CORE_LOG_API`
- `RAC_CORE_LOG_RADIO`

Current defaults are defined in [smtc_rac.c](rac/smtc_rac.c#L82):

- enabled by default: `ERROR`, `CONFIG`
- disabled by default: `INFO`, `WARN`, `DEBUG`, `API`, `RADIO`

For local debugging, the most direct approach is to modify those default macros in [smtc_rac.c](rac/smtc_rac.c#L82), for example:

```c
#define RAC_CORE_LOG_INFO_ENABLE 1
#define RAC_CORE_LOG_WARN_ENABLE 1
#define RAC_CORE_LOG_DEBUG_ENABLE 1
#define RAC_CORE_LOG_API_ENABLE 1
#define RAC_CORE_LOG_RADIO_ENABLE 1
```

You can also override them through compile definitions:

```cmake
target_compile_definitions(${COMPONENT_LIB} PRIVATE
    RAC_CORE_LOG_INFO_ENABLE=1
    RAC_CORE_LOG_WARN_ENABLE=1
    RAC_CORE_LOG_DEBUG_ENABLE=1
    RAC_CORE_LOG_API_ENABLE=1
    RAC_CORE_LOG_RADIO_ENABLE=1
)
```

Set any of them to `0` to disable the corresponding log type.

### 2. RAC Business Logs

The `LoRa`, `FLRC`, and `LR-FHSS` paths use the shared logging macros from [smtc_rac_log.h](rac_api/smtc_rac_log.h):

- `RAC_LOG_ERROR`
- `RAC_LOG_WARN`
- `RAC_LOG_INFO`
- `RAC_LOG_DEBUG`
- `RAC_LOG_CONFIG`
- `RAC_LOG_TX`
- `RAC_LOG_RX`
- `RAC_LOG_STATS`
- `RAC_LOG_HEX_DUMP`
- `RAC_LOG_BANNER`
- `RAC_LOG_SIMPLE_BANNER`

Typical output:

```text
[RAC-LORA-INFO ] [    1234 ms] setup ok
```

Where:

- prefixes such as `RAC-LORA`, `RAC-FLRC`, and `RAC-LRFHSS` identify the business module
- timestamps come from `smtc_modem_hal_get_time_in_ms()`
- final output is routed through `SMTC_MODEM_HAL_TRACE_PRINTF(...)` into `smtc_modem_hal_print_trace(...)`

The component defines these log profiles in [CMakeLists.txt](CMakeLists.txt#L222):

- `OFF`
  - disables `FSK`, `LoRa`, and `LR-FHSS` business logs
- `MINIMAL`
  - enables only `LoRa` business logs
- `VERBOSE`
  - enables `FSK`, `LoRa`, and `LR-FHSS` business logs
- `ALL`
  - currently equivalent to `VERBOSE`

These profiles are exposed through `Kconfig` and can be selected in `menuconfig`:

```text
Liot LR2021 (IDF)
  -> Logging
    -> RAC business log profile
```

You can also override them manually through compile definitions:

```cmake
target_compile_definitions(${COMPONENT_LIB} PRIVATE
    RAC_FSK_LOG_ENABLE=1
    RAC_LORA_LOG_ENABLE=1
    RAC_LRFHSS_LOG_ENABLE=1
)
```

### 3. Trace Backend Switches

`RAC` logging ultimately depends on the trace macros in [smtc_modem_hal_dbg_trace.h](modem_hal/smtc_modem_hal_dbg_trace.h).

Current defaults:

- `MODEM_HAL_DBG_TRACE=ON`
- `MODEM_HAL_DBG_TRACE_COLOR=ON`
- `MODEM_HAL_DBG_TRACE_RP=OFF`
- `MODEM_HAL_DEEP_DBG_TRACE=OFF`

To disable `RAC` trace output:

```cmake
target_compile_definitions(${COMPONENT_LIB} PRIVATE
    MODEM_HAL_DBG_TRACE=0
)
```

To disable colored output:

```cmake
target_compile_definitions(${COMPONENT_LIB} PRIVATE
    MODEM_HAL_DBG_TRACE_COLOR=0
)
```

### 4. ESP_LOG Logs

Some logs use native `ESP-IDF` `ESP_LOGx` directly:

- [ralf_lr20xx.c](ralf/lr20xx_ralf/ralf_lr20xx.c)
  - mainly outputs `LR20xx` `setup GFSK / LoRa / FLRC / CAD` information
- [ral_lr20xx.c](ral/lr20xx_ral/ral_lr20xx.c)
  - mainly outputs some `PA` and TX configuration debug information
- [smtc_modem_hal.c](modem_hal/smtc_modem_hal.c)
  - mainly outputs `panic` and part of the HAL runtime information

These logs are not controlled by `RAC_LOG_*` macros. They follow normal `ESP-IDF` log-level control.

Common control methods:

1. Change the default log level in `menuconfig`
2. Call `esp_log_level_set("*", ESP_LOG_INFO);` at runtime
3. Call `esp_log_level_set("SMTC_MODEM", ESP_LOG_DEBUG);` for a specific tag

### 5. Using Logging in Applications

If your application wants to reuse the component's `RAC`-style logging directly, include:

- [smtc_rac_log.h](rac_api/smtc_rac_log.h)

Example:

```c
#include "smtc_rac_log.h"

void app_main(void)
{
    RAC_LOG_SIMPLE_BANNER("LR2021 demo");
    RAC_LOG_INFO("radio init start");
    RAC_LOG_CONFIG("freq=%u", 470300000U);
    RAC_LOG_TX("tx len=%u", 16U);
}
```

If your application only wants native `ESP-IDF` logging, use `ESP_LOGI`, `ESP_LOGW`, and `ESP_LOGE` directly without depending on `RAC_LOG_*`.

## Default Hardware Configuration

The default `SPI` and `GPIO` values in `Kconfig` are preset according to the typical wiring of the `L-LRMAM36-FANN4` module.

So when developing with `L-LRMAM36-FANN4`, you can usually start from the component defaults without manually changing `SPI Host`, `NRST`, `BUSY`, `NSS`, `MOSI`, `MISO`, `SCLK`, or `DIO7` in the initial stage.

For the `LR20XX / LR2021` path, the component currently supports selecting Lierda board types in `menuconfig`:

- `L-LRMAM36-FANN4`
- `L-LRMWP35-FANN4`

Specifically:

- with `L-LRMAM36-FANN4`, the pin parameters are preset and usually need no extra configuration
- with `L-LRMWP35-FANN4`, you can continue configuring `SPI` and `GPIO` parameters in `menuconfig`

In the current first implementation, `L-LRMAM36-FANN4` and `L-LRMWP35-FANN4` already map to separate `LR20XX BSP` source files, making it easier to refine crystal, power table, PA, and RF front-end parameters by board later.

Board-to-BSP mapping:

- `L-LRMAM36-FANN4` -> [ral_lr20xx_bsp_lrmam36_fann4.c](ral/lr20xx_ral/ral_lr20xx_bsp_lrmam36_fann4.c)
- `L-LRMWP35-FANN4` -> [ral_lr20xx_bsp_lrmwp35_fann4.c](ral/lr20xx_ral/ral_lr20xx_bsp_lrmwp35_fann4.c)
- `Generic / Custom LR2021 SPI board` -> [ral_lr20xx_bsp.c](ral/lr20xx_ral/ral_lr20xx_bsp.c)

## Custom Hardware Adaptation

If you use `L-LRMAM36-FANN4`, you can usually keep the default component settings.
For other hardware, relevant configuration should be adjusted based on the actual board.

### 1. Crystal / Oscillator Configuration

For the current `LR20XX` path, the crystal-related configuration is determined by the board BSP:

- `L-LRMAM36-FANN4`: [ral_lr20xx_bsp_lrmam36_fann4.c](ral/lr20xx_ral/ral_lr20xx_bsp_lrmam36_fann4.c#L484)
- `L-LRMWP35-FANN4`: [ral_lr20xx_bsp_lrmwp35_fann4.c](ral/lr20xx_ral/ral_lr20xx_bsp_lrmwp35_fann4.c#L484)
- `Generic / Custom LR2021 SPI board`: [ral_lr20xx_bsp.c](ral/lr20xx_ral/ral_lr20xx_bsp.c#L484)

For custom `ESP-IDF + LR2021 SPI` hardware, verify the following against the actual RF module and reference design:

- whether the board uses `XTAL` or `TCXO`
- if `TCXO` is used, adjust `xosc_cfg`, supply voltage, and startup time in `ral_lr20xx_bsp_get_xosc_cfg(...)`
- if `XTAL` is used, evaluate whether the default `XOSC trim` parameters should be adjusted based on the real crystal and board parasitics

If this configuration does not match the hardware, radio initialization issues, clock stability problems, or abnormal TX/RX behavior may occur.

### 2. Pin Configuration

For `L-LRMWP35-FANN4` and custom `ESP-IDF + LR2021 SPI` hardware, you typically also need to adjust these parameters in `menuconfig` according to the real wiring:

- `LR2021_RADIO_SPI_ID`
- `LR2021_NRST_GPIO`
- `LR2021_BUSY_GPIO`
- `LR2021_NSS_GPIO`
- `LR2021_SPI_MOSI`
- `LR2021_SPI_MISO`
- `LR2021_SPI_CLK`
- `LR20XX_DIO7_GPIO`

These options are defined in [Kconfig](Kconfig).

`LR2021_NRST_GPIO` can be set to `-1` when NRST is not connected. `ral_reset()` then cannot reset the radio: it only makes sure the radio is woken up before the next command. Connecting NRST is recommended, because without it the radio keeps its state, including the loaded PRAM, when only the ESP32 restarts.

If your hardware is not wired like the default `L-LRMAM36-FANN4`, these parameters should be treated as mandatory checks, not optional tuning.

## ESP-IDF Integration

The component is structured as a normal ESP-IDF component and already includes:

- [CMakeLists.txt](CMakeLists.txt)
- [idf_component.yml](idf_component.yml)
- [LICENSE](LICENSE)
- [NOTICE](NOTICE)

After integrating it into a project, users usually only need to:

1. Select the target radio family and GPIO / SPI parameters in `menuconfig`
2. Include the required header files in application code
3. Call `RAC`, `RALF`, or `RAL` interfaces depending on the desired abstraction level

## Example Project

For integration examples and basic usage references, use:

- [esp32_lora_samples](https://github.com/lierda-iot/esp32_lora_samples.git)

Application example:

- [DoorCam-LR](https://github.com/lierda-iot/DoorCam-LR): video doorbell with live video and two-way voice over FLRC bursts, built on this component through `RAL`. Its receive path is described in [Receive Status Flags](#receive-status-flags-lr20xx).

## Documentation Links

Lierda product documents (public DingTalk knowledge base, in Chinese):

- [L-LRMAM36-FANN4 (AM36)](https://alidocs.dingtalk.com/i/nodes/dxXB52LJqnGE6GN4cQRNK0PX8qjMp697?utm_scene=team_space)
- [L-LRMWP35-FANN4 (WP35)](https://alidocs.dingtalk.com/i/nodes/gpG2NdyVX32N52zwuAREozYpWMwvDqPk?utm_scene=team_space)

## Quick Start

Recommended minimal integration flow:

### 1. Add the Component to Your Project

Add the component from the [ESP Component Registry](https://components.espressif.com/components/lierda-iot/esp_lora_driver):

```bash
idf.py add-dependency "lierda-iot/esp_lora_driver^1.0.0"
```

To use the source code directly instead, see [Using the Source Code as a Local Component](#using-the-source-code-as-a-local-component).

### 2. Configure Target Chip and Hardware Parameters

Run:

```bash
idf.py menuconfig
```

Then configure the following in `Liot LR2021 (IDF)`:

- select the target radio chip family
- if you use `L-LRMAM36-FANN4`, start with the default parameters
- if you use `L-LRMWP35-FANN4`, select the matching board and then adjust `SPI` and `GPIO` as needed
- if you use other custom `ESP-IDF + LR2021 SPI` hardware, adjust `SPI` and `GPIO` according to the real wiring and verify that the crystal configuration matches the hardware

### 3. Include the Public Entry Header

```c
#include "LiotLr2021.h"
```

### 4. Call the Required Layer

You can directly use interfaces from these layers through the single entry header:

- `RAC API`: suitable for application-level radio transactions
- `RAC`: suitable for direct radio access management control
- `RALF`: suitable for finer radio feature configuration
- `RAL`: suitable for low-level radio abstraction control

### 5. Build

```bash
idf.py build
```

## Usage Notes

### OOK Modulation (LR20xx)

OOK is available through `RALF` and `RAL` on LR20xx radios (other radios return `RAL_STATUS_UNSUPPORTED_FEATURE`):

- `ralf_setup_ook()` with `ralf_params_ook_t` configures the packet type, frequency, TX power, packet and modulation parameters, receiver detector, sync word, CRC, addresses and whitening in one call
- `ral_set_ook_*()`, `ral_get_ook_rx_pkt_status()` and `ral_get_ook_time_on_air_in_ms()` give access to each step
- transmission and reception use the usual `ral_set_tx()` / `ral_set_rx()` and IRQ functions

Example:

```c
static const uint8_t ook_sync_word[4] = { 0x7F, 0x53, 0x65, 0x64 };

const ralf_params_ook_t ook_params = {
    .rf_freq_in_hz     = 868100000,
    .output_pwr_in_dbm = 14,
    .mod_params = {
        .br_in_bps    = 32000,
        .bw_dsb_in_hz = 153000,
        .pulse_shape  = RAL_OOK_PULSE_SHAPE_OFF,
        .mag_depth    = RAL_OOK_MAG_DEPTH_FULL,
    },
    .pkt_params = {
        .preamble_len_in_bits  = 32,
        .sync_word_len_in_bits = 32,
        .address_filtering     = RAL_OOK_ADDRESS_FILTERING_DISABLE,
        .header_type           = RAL_OOK_PKT_VAR_LEN,
        .pld_len_in_bytes      = 255,
        .crc_type              = RAL_OOK_CRC_2_BYTES,
        .encoding              = RAL_OOK_ENCODING_OFF,
    },
    .rx_detector = {
        .pattern              = 0x5,
        .pattern_len_in_bits  = 4,
        .pattern_repeat_nb    = 8,
        .sfd_type             = RAL_OOK_SFD_FALLING_EDGE,
        .sfd_len_in_bits      = 0,
        .is_sync_word_encoded = false,
    },
    .sync_word            = ook_sync_word,
    .sync_word_bit_order  = RAL_OOK_SYNC_WORD_MSB_FIRST,
    .crc_seed             = 0x1D0F,
    .crc_polynomial       = 0x1021,
    .whitening_polynomial = 0,  // whitening disabled
};

ralf_setup_ook( &radio, &ook_params );
```

Notes:

- `pattern_len_in_bits` is the real pattern length; the driver writes the length minus one to the radio
- whitening is enabled by a non-zero `whitening_polynomial`; `whitening_polynomial` and `whitening_seed` are 12-bit values and `whitening_bit_index` is in [0:15] (the LR20xx reference examples use bit index 1)
- the LR20xx driver documents that an explicit header without CRC is known to cause incorrect OOK reception, so keep the CRC enabled with a length header
- the radio computes a conservative OOK detection threshold; if the packet error rate is too high, the threshold can be changed with `lr20xx_workarounds_ook_set_detection_threshold_level()` (see [radio_drivers/lr20xx_driver/README.md](radio_drivers/lr20xx_driver/README.md))
- `ral_get_ook_time_on_air_in_ms()` returns 0 when the bit rate is 0, and for the configurations the LR20xx driver cannot compute: 16-bit length header (`RAL_OOK_PKT_VAR_LEN_16_BITS`) and bi-phase mark encoding

### Receive Status Flags (LR20xx)

Every LR20xx interrupt is available through `RAL` (`ral_set_dio_irq_params()`, `ral_get_irq_status()`, `ral_get_and_clear_irq_status()`, `ral_clear_irq_status()`). Besides the usual flags, this includes:

- `RAL_IRQ_RX_LEN_ERROR`: received packet longer than the configured payload length, reported with `RAL_IRQ_RX_DONE`
- `RAL_IRQ_RX_ADDR_ERROR`: packet discarded because its address does not match
- `RAL_IRQ_RTTOF_REQ_VALID`, `RAL_IRQ_RX_HDR_TIMESTAMP`, `RAL_IRQ_LOW_BATTERY`, `RAL_IRQ_PA_OVP_OCP`, `RAL_IRQ_LR_FHSS_NEW_TABLE` and `RAL_IRQ_LR_FHSS_NEW_PAYLOAD`

`RAL_IRQ_ALL` clears every LR20xx interrupt, including `PA_OVP_OCP`, which the vendor mask `LR20XX_SYSTEM_IRQ_ALL_MASK` leaves out.

The driver reports these flags; whether to use the hardware CRC and how to react to each flag is up to the application. `RAC` (radio planner) judges a reception from `RX_DONE`, header error and CRC error only, and clears the interrupts after reading them, so applications that need the other flags use `RAL` directly.

Recommendations:

- **One packet per `RX_DONE`** (single LoRa, GFSK, FLRC or OOK packets): enable the hardware CRC, and treat `RX_DONE` together with `RAL_IRQ_RX_CRC_ERROR` or `RAL_IRQ_RX_LEN_ERROR` as a bad packet, as the LR20xx reference examples do.
- **Continuous FLRC bursts**, where several packets can be waiting in the RX FIFO when the application wakes up: the interrupt flags of these packets add up, so they cannot tell which packet is bad. Read the FIFO by its level, rebuild the packets in software and check each one with an application CRC; the hardware CRC can then be turned off. [DoorCam-LR](https://github.com/lierda-iot/DoorCam-LR) works this way: see `flrc_packet_params()` and `handle_rx_packet()` in `main/radio_ping.cpp`.

### Sleep Without Retention

The component itself only uses `ral_set_sleep( radio, true )`, which keeps the radio configuration and the PRAM.

After `ral_set_sleep( radio, false )` the radio loses its configuration and the PRAM. On wake-up, call `ral_init()` again (it loads and checks the PRAM, then restores clock, DIO, calibration and FIFO settings), then apply the radio configuration again (for example `ralf_setup_lora()` and `ral_set_dio_irq_params()`), as after power-up. No separate reset is needed: the HAL wakes the radio up on the first command.

### Using the Source Code as a Local Component

When this repository is used through `override_path` or `path` in `idf_component.yml`, or placed in the project `components/` directory, ESP-IDF takes the component name from the directory name. Application code refers to the component as `esp_lora_driver`, so the directory must be named `esp_lora_driver`, for example:

```bash
git clone https://github.com/lierda-iot/esp32_lora_driver.git esp_lora_driver
```

The project and the component also need to be on the same drive on Windows, as `override_path` is stored as a relative path.

## Recommended Usage Priority

From a maintenance perspective, the suggested usage order is:

- prefer `RAC API` or `RAC` in application code
- use `RALF` when finer-grained radio control is needed
- use `RAL` only when the lowest-level chip abstraction is required

Benefits:

- better decoupling between upper-layer code and the concrete chip
- better reuse when adapting to more Semtech chips later
- lower impact on applications when internal implementation changes

## Version

Current `RAC API` version is defined in [smtc_rac_version.h](rac_api/smtc_rac_version.h#L61):

- Major: `1`
- Minor: `1`
- Patch: `2`

## License

This component is derived from and adapted from Semtech upstream code. The primary license is `The Clear BSD License`, with SPDX identifier `BSD-3-Clause-Clear`.

Related files:

- [LICENSE](LICENSE)
- [NOTICE](NOTICE)

When distributing source or binaries, keep the upstream copyright notice, license text, and disclaimer.

## Release Notes

### 1.0.0

This release contains all changes made after 0.0.5.

Upstream sync:

- Semtech `RAC API` 1.0.0 → 1.1.2, LR20xx driver v1.3.4 → v2.0.2 (with PRAM loading), LR11xx driver v2.7.0 → v3.0.0.

API changes that need code updates when upgrading from 0.0.5:

- FLRC modulation: `br_in_bps` and `bw_dsb_in_hz` in `ral_flrc_mod_params_t` are replaced by `raw_bit_rate` (`RAL_FLRC_RAW_BIT_RATE_0_260_MBPS` to `RAL_FLRC_RAW_BIT_RATE_2_600_MBPS`).
- FLRC preamble: `preamble_len_in_bits` in `ral_flrc_pkt_params_t` is replaced by `preamble_len` (`RAL_FLRC_PREAMBLE_LENGTH_4_BITS` to `RAL_FLRC_PREAMBLE_LENGTH_32_BITS`). For a longer preamble (36 to 16380 bits, multiple of 4), set `RAL_FLRC_PREAMBLE_LENGTH_32_BITS`, then call `ral_set_flrc_long_preamble_len_in_bits()`. This replaces the automatic handling added in 0.0.5.
- FLRC sync words: `sync_word` in `ralf_params_flrc_t` and in the `RAC` FLRC parameters is now an array of three pointers, one per sync word.
- LoRa: `symb_nb_timeout` in `ralf_params_lora_t` and in the `RAC` LoRa parameters is now `uint16_t`.

New in `RAC`:

- FLRC burst transfer: `smtc_rac_flrc_burst()`, `smtc_rac_flrc_burst_rx_done()` and `smtc_rac_radio_flrc_burst_params_t`.
- Immediate radio access: `smtc_rac_immediate_radio_access()` and `smtc_rac_release_immediate_radio_access()`.
- Active time-out: `smtc_rac_set_active_time_out()` and `smtc_rac_release_active_time_out()`.
- `smtc_rac_get_callback_radio_id()`, `smtc_rac_get_context_private()`, `smtc_rac_set_context_private()`, and `keep_radio_awake` in `smtc_rac_context_t`.

New in `RAL` and `RALF`:

- OOK on LR20xx: 9 `RAL` functions (parameters, detector, sync word, CRC, address, whitening, packet status, time on air) and `ralf_setup_ook()`.
- FIFO access: `ral_get_pkt_size()`, `ral_get_data_rx_buffer()`, `ral_clear_rx_fifo()`, `ral_clear_tx_fifo()`, `ral_get_fifo_irq()` and `ral_get_and_clear_fifo_irq()`.
- GFSK: `ral_set_gfsk_whitening_seed_comp()`, and the CRC types `RAL_GFSK_CRC_3_BYTES_INV`, `RAL_GFSK_CRC_4_BYTES` and `RAL_GFSK_CRC_4_BYTES_INV`.
- LoRa: bandwidths `RAL_LORA_BW_083_KHZ` and `RAL_LORA_BW_101_KHZ` on LR20xx, and `freq_offset_hz` in `ral_lora_rx_pkt_status_t`.
- FLRC: `crc_seed` is applied on LR20xx, and `ralf_params_flrc_t` has an `is_tx` field.
- IRQs: RAL flags cover all 28 LR20xx interrupt bits. New flags: `RAL_IRQ_RX_LEN_ERROR`, `RAL_IRQ_RX_ADDR_ERROR`, `RAL_IRQ_CMD_ERROR`, `RAL_IRQ_ERROR`, `RAL_IRQ_RTTOF_REQ_VALID`, `RAL_IRQ_RX_HDR_TIMESTAMP`, `RAL_IRQ_LOW_BATTERY`, `RAL_IRQ_PA_OVP_OCP`, `RAL_IRQ_LR_FHSS_NEW_TABLE` and `RAL_IRQ_LR_FHSS_NEW_PAYLOAD`.

Initialization and calibration:

- Initialization checks the `lr20xx_system_init()` result and verifies the PRAM, enters standby XOSC after the crystal configuration, runs system calibration when a TCXO is used, and clears pending IRQs before configuring the DIOs. The front-end calibration frequencies at initialization are 470, 897.5 and 2441 MHz.
- On a frequency change, PLL/AAF are recalibrated when the frequency moves by more than 50 MHz, front-end calibration runs when it moves by more than 10 MHz, and the RX path and boost are set last.

Fixes:

- `ral_reset()` configures the NRST pin before the first reset pulse, and the reset low time is exactly 1 ms.
- A single SPI transfer can carry a full FIFO (2-byte command plus 1024 bytes): the limit is 1026 bytes, the length is checked on entry, a failed write reports an error, a static DMA buffer is used, and the FIFO read offset follows the actual command length.
- `LR2021_NRST_GPIO = -1` (not connected) no longer evaluates `1ULL << -1`.
- During initialization, the system error read is checked and its variable is initialized.
- `ral_cal_img()` uses the HF receive path for 2.4 GHz frequencies.
- Log format specifiers for 32-bit values.

Hardware and configuration:

- `L-LRMAM36-FANN4` and `L-LRMWP35-FANN4`: the crystal trims XTA and XTB are 11 (were 0), which enables the internal load capacitors.
- The PA and RX path switch from the LF to the HF path at 1.5 GHz (was 1.6 GHz).
- The NSS wake-up pulse is 100 µs (was 1 ms).
- `menuconfig`: LR20xx chip model (`LR2012`, `LR2021`, `LR2022`); SX126x chip model (`SX1261`, `SX1262`, `SX1268`) with BPSK (default off) and LR-FHSS (default on) options; LR11xx CRC over SPI and geolocation options; radio planner margin delay (default 8 ms) and region duty-cycle profile (none, EU 868, RU 864). The logging profiles also set `RAC_LOG_ENABLE`.

Documentation and packaging:

- The Chinese README is now `README_CN.md`, so the registry page shows a language switch.
- README: registry installation command, FLRC data rates, OOK usage, receive status flag guidance with DoorCam-LR as a reference, `LR2021_NRST_GPIO = -1`, sleep without retention, and using the source code as a local component.
- Registry tags `flrc`, `semtech` and `esp32-s3` added; `lr20xx` and `esp-idf` removed.
- The unused `ral/base64.c` and `ral/base64.h` are removed.
- `LICENSE` adds the Lierda copyright line for the ESP-IDF adaptation.

### 0.0.5

- FLRC preambles longer than 32 bits on LR20xx, up to 16380 bits in steps of 4 bits, through `preamble_len_in_bits` in `ral_flrc_pkt_params_t`.

### 0.0.4

- English README by default, with a link to the Chinese README (`README.zh-CN.md`). `README.en.md` removed.

### 0.0.3

- English README added (`README.en.md`); the Chinese README is also available as `README.zh-CN.md`.
- Chinese comments in `modem_hal` and `lr20xx_hal` translated to English (comments only).

### 0.0.2

- First release: ESP-IDF component based on Semtech `smtc_rac_lib`, with `RAC API`, `RAC` with the radio planner, `RALF`, `RAL`, and the LR20xx, LR11xx and SX126x drivers.
- ESP-IDF HAL for SPI, GPIO, timers and logging, and the single entry header `LiotLr2021.h`.
- `menuconfig` options for the radio family, the board (`L-LRMAM36-FANN4` or `L-LRMWP35-FANN4`), SPI, GPIO and logging profiles.
- `LICENSE` and `NOTICE` (The Clear BSD License).

## Maintenance Notes

The component intentionally retains a strong upstream structural footprint. This is done to:

- simplify diff comparison against Semtech upstream code
- reduce the maintenance cost of future synchronization or troubleshooting
- gradually clean up the internal structure without breaking the public interface

For that reason, the public interface is exposed through `include/`, while the internal implementation directories keep the original layered layout.
