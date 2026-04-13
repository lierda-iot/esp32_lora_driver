# ESP LoRa Driver

[简体中文](README.zh-CN.md) | [English](README.en.md)

Repository: <https://github.com/lierda-iot/esp32_lora_driver.git>

`ESP LoRa Driver` is an ESP-IDF component for Semtech LoRa radio chips. The current primary target is `LR20xx / LR2021`. The component exposes layered `RAC`, `RALF`, and `RAL` interfaces, and is intended to expand to `SX126x`, `LR11xx`, and other chip families later.

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

## Documentation Links

Additional component documents:

- [DingTalk Doc](https://alidocs.dingtalk.com/i/nodes/dxXB52LJqnGE6GN4cQRNK0PX8qjMp697?utm_scene=team_space)
- [DingTalk Doc](https://alidocs.dingtalk.com/i/nodes/gpG2NdyVX32N52zwuAREozYpWMwvDqPk?utm_scene=team_space)

## Quick Start

Recommended minimal integration flow:

### 1. Add the Component to Your Project

Add this component to your ESP-IDF project as a local component and make sure the build system can find it.

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

For the current `LR20XX / LR2021` path, the component has already passed a minimal build validation and is the main integration path at this stage.

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

Current `RAC API` version is defined in [smtc_rac_version.h](rac_api/smtc_rac_version.h#L1):

- Major: `1`
- Minor: `0`
- Patch: `0`

## License

This component is derived from and adapted from Semtech upstream code. The primary license is `The Clear BSD License`, with SPDX identifier `BSD-3-Clause-Clear`.

Related files:

- [LICENSE](LICENSE)
- [NOTICE](NOTICE)

When distributing source or binaries, keep the upstream copyright notice, license text, and disclaimer.

## Release Notes

If you plan to publish this component to ESP Component Registry later, verify at least the following first:

- update the `url` field in [idf_component.yml](idf_component.yml)
- add sample project links
- perform at least one build validation for the target chip configuration
- confirm that all public headers are truly stable external interfaces

Current validation status:

- `L-LRMAM36-FANN4` has passed minimal ESP-IDF project build validation
- `L-LRMWP35-FANN4` is in functional verification, and performance tuning is still pending
- `SX126X` / `LR11XX` branches should still be considered unverified

Notes:

- in the current review round, the minimal project was actually built for the `L-LRMAM36-FANN4` path
- `L-LRMWP35-FANN4` has been split into an independent BSP and connected through the basic functional path, but board-level performance parameters still need follow-up refinement
- independent branch build confirmation has not yet been completed for `SX126X` / `LR11XX`, so they should not be claimed as fully validated in a release by default

## Maintenance Notes

The component intentionally retains a strong upstream structural footprint. This is done to:

- simplify diff comparison against Semtech upstream code
- reduce the maintenance cost of future synchronization or troubleshooting
- gradually clean up the internal structure without breaking the public interface

For that reason, the public interface is exposed through `include/`, while the internal implementation directories keep the original layered layout.
