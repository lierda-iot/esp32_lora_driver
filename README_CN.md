# ESP LoRa Driver

语言：中文 | [English](README.md)

仓库地址：<https://github.com/lierda-iot/esp32_lora_driver.git>

`ESP LoRa Driver` 是一个面向 Semtech LoRa 无线芯片的 ESP-IDF 组件库，当前以 `LR20xx/LR2021` 为主要目标，集成了 ESP 平台适配，并对外公开 `RAC`、`RALF` 和 `RAL` 分层接口，后续将继续支持 `SX126x`、`LR11xx` 等芯片型号。

**FLRC 高速收发**：使用本驱动，`LR2021` 的 FLRC 空口速率可达 2.6 Mbit/s（`RAL_FLRC_RAW_BIT_RATE_2_600_MBPS`）。不编码时（`RAL_FLRC_CR_1_1`，即芯片驱动的 `LR20XX_RADIO_FLRC_CR_NONE`），应用层有效数据实测接近 2.2 Mbit/s，重传、包间隔等协议开销不计在内。

该组件基于 Semtech 上游 `smtc_rac_lib` 修改、裁剪并移植到 ESP-IDF 环境，保留了上游的分层抽象方式，并增加了适用于 ESP 平台的 HAL、GPIO、SPI、定时器和日志适配。

## 组件目标

这个组件不是单一的“裸驱动文件”，而是一套分层的无线接入组件库，目标包括：

- 为 `LR20xx/LR2021` 提供可在 ESP-IDF 下直接集成的射频驱动能力
- 保留 Semtech 上游 `RAC / RALF / RAL` 的分层抽象，便于后续维护和同步
- 同时支持“高层调度接入”和“低层驱动直连”两种使用方式
- 为后续扩展到其他 Semtech 无线芯片预留统一结构

## 分层结构

从组件库视角看，当前代码主要分为以下几层：

- `RAC API`：面向应用层的高层无线访问接口，适合需要统一调度、任务提交和事务管理的场景
- `RAC`：无线访问控制层，负责组织任务、调度、收发流程和无线事务上下文
- `Radio Planner`：调度与仲裁层，负责不同无线任务的排队、优先级和时序控制
- `RALF`：面向具体无线能力的功能层封装，构建在 `RAL` 之上
- `RAL`：射频抽象层，向上提供统一操作接口，向下对接具体芯片驱动
- `Radio Driver`：芯片寄存器与命令级驱动实现
- `Radio HAL / Modem HAL`：平台相关层，负责 SPI、GPIO、中断、时间和日志等底层能力

这意味着本组件既可以作为“完整的射频访问组件”使用，也可以只使用其中的某一层。

## 适用场景

这个组件适合以下几类场景：

- 在 ESP-IDF 工程中快速接入 `LR2021` 射频芯片
- 需要基于统一接口调度 LoRa、FSK、FLRC 或 LR-FHSS 等射频任务
- 需要在应用层直接调用 `RAC` 接口管理无线事务
- 需要在较低层直接访问 `RAL` 或 `RALF` 以实现更细粒度的射频控制

## 支持的利尔达产品

当前组件面向以下利尔达产品形态：

- `L-LRMAM36-FANN4`
- `L-LRMWP35-FANN4`，即基于 `ESP-IDF` 搭配 `LR2021 SPI` 模组的开发方案

## 对外接口

当前组件采用单入口对外方式，外部用户只需要包含一个头文件：

- [include/LiotLr2021.h](include/LiotLr2021.h)

也就是说，推荐的接入方式是：

```c
#include "LiotLr2021.h"
```

`LiotLr2021.h` 本身会聚合以下几层公共接口：

- `RAC API`
  - 来自 [smtc_rac_api.h](rac_api/smtc_rac_api.h)
  - 适合应用层直接发起无线事务、管理调度上下文和使用统一 API
- `RAC`
  - 来自 [smtc_rac.h](rac/smtc_rac.h)
  - 适合需要直接操作无线访问控制层的场景
- `RALF`
  - 来自 [ralf.h](ralf/ralf.h)
  - 适合需要使用射频功能层接口的场景
- `RAL`
  - 来自 [ral.h](ral/ral.h)
  - 适合需要最底层射频抽象接口的场景

因此，虽然用户只需要包含一个入口头文件，但仍然可以根据自己对组件分层的理解，直接调用不同层的接口类型和函数。

这种设计的目的有两个：

- 对外使用方式保持简单，只保留一个正式入口
- 内部仍保留上游风格的分层结构，便于维护、裁剪和继续同步

## 目录说明

当前目录结构大致如下：

- `include/`
  - 组件对外公开的单入口头
- `rac_api/`
  - 应用层使用的高层 API
- `rac/`
  - 无线访问控制核心逻辑
- `radio_planner/`
  - 无线任务调度器
- `ral/`
  - 射频抽象层
- `ralf/`
  - 射频功能层
- `radio_drivers/`
  - 各芯片驱动实现
- `radio_hal/`
  - 芯片相关平台适配层
- `modem_hal/`
  - ESP-IDF 平台底层 HAL 和调试支持
- `Kconfig`
  - 组件配置项
- `idf_component.yml`
  - 组件元数据

## 配置方式

组件通过 [Kconfig](Kconfig) 暴露配置项，主要包括：

- 射频芯片家族选择
  - `SX126x`
  - `LR11xx`
  - `LR20xx`
- `LR2021` 的 SPI Host 选择
- `L-LRMWP35-FANN4` 或 `Generic / Custom LR2021 SPI board` 下的 `NRST`、`BUSY`、`NSS`、`MOSI`、`MISO`、`SCLK` 等 GPIO 分配
- `L-LRMWP35-FANN4` 或 `Generic / Custom LR2021 SPI board` 下的 `LR20xx` 相关 `DIO` 配置

当前默认目标为 `LR20xx`，适合作为 `LR2021` 的默认配置基础。

## 日志输出说明

当前组件内的日志主要分为三类：

- `RAC Core` 日志
  - 位于 [smtc_rac.c](rac/smtc_rac.c)
  - 主要用于 `RAC` 初始化、上下文设置、调度流程和接口调用跟踪
- `RAC 业务日志`
  - 主要位于 `LoRa`、`FLRC`、`LR-FHSS` 等 `RAC` 业务文件
  - 使用统一的 [smtc_rac_log.h](rac_api/smtc_rac_log.h) 宏输出
- `ESP_LOG` 日志
  - 主要位于 `LR20xx RAL / RALF` 和 `modem_hal`
  - 使用 `ESP_LOGI`、`ESP_LOGD`、`ESP_LOGE` 输出

### 1. RAC Core 日志

`RAC Core` 使用以下宏：

- `RAC_CORE_LOG_ERROR`
- `RAC_CORE_LOG_CONFIG`
- `RAC_CORE_LOG_INFO`
- `RAC_CORE_LOG_WARN`
- `RAC_CORE_LOG_DEBUG`
- `RAC_CORE_LOG_API`
- `RAC_CORE_LOG_RADIO`

当前默认行为见 [smtc_rac.c](rac/smtc_rac.c#L82)：

- 默认开启：`ERROR`、`CONFIG`
- 默认关闭：`INFO`、`WARN`、`DEBUG`、`API`、`RADIO`

如果只是临时调试，最直接的方式是修改 [smtc_rac.c](rac/smtc_rac.c#L82) 中这些默认宏值，例如：

```c
#define RAC_CORE_LOG_INFO_ENABLE 1
#define RAC_CORE_LOG_WARN_ENABLE 1
#define RAC_CORE_LOG_DEBUG_ENABLE 1
#define RAC_CORE_LOG_API_ENABLE 1
#define RAC_CORE_LOG_RADIO_ENABLE 1
```

这种方式适合本地快速打开或关闭 `RAC Core` 日志。

如果希望在不改源码的情况下控制日志，也可以通过编译宏覆盖，例如：

```cmake
target_compile_definitions(${COMPONENT_LIB} PRIVATE
    RAC_CORE_LOG_INFO_ENABLE=1
    RAC_CORE_LOG_WARN_ENABLE=1
    RAC_CORE_LOG_DEBUG_ENABLE=1
    RAC_CORE_LOG_API_ENABLE=1
    RAC_CORE_LOG_RADIO_ENABLE=1
)
```

如果想关闭某一类日志，可将对应宏设为 `0`。编译宏会覆盖源码里的默认值。

### 2. RAC 业务日志

`LoRa`、`FLRC`、`LR-FHSS` 路径使用 [smtc_rac_log.h](rac_api/smtc_rac_log.h) 中的统一日志宏：

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

日志输出格式大致如下：

```text
[RAC-LORA-INFO ] [    1234 ms] setup ok
```

其中：

- 前缀如 `RAC-LORA`、`RAC-FLRC`、`RAC-LRFHSS` 用于区分业务模块
- 时间戳来自 `smtc_modem_hal_get_time_in_ms()`
- 最终输出通过 `SMTC_MODEM_HAL_TRACE_PRINTF(...)` 进入 `smtc_modem_hal_print_trace(...)`

当前组件在 [CMakeLists.txt](CMakeLists.txt#L222) 中预留了以下日志 profile：

- `OFF`
  - 关闭 `FSK`、`LoRa`、`LR-FHSS` 业务日志
- `MINIMAL`
  - 仅开启 `LoRa` 业务日志
- `VERBOSE`
  - 开启 `FSK`、`LoRa`、`LR-FHSS` 业务日志
- `ALL`
  - 当前实现与 `VERBOSE` 等效

这些 profile 现在已经通过 `Kconfig` 暴露，可直接在 `menuconfig` 中选择：

```text
Liot LR2021 (IDF)
  -> Logging
    -> RAC business log profile
```

如果你希望手动控制这几类日志，可通过编译宏覆盖，例如：

```cmake
target_compile_definitions(${COMPONENT_LIB} PRIVATE
    RAC_FSK_LOG_ENABLE=1
    RAC_LORA_LOG_ENABLE=1
    RAC_LRFHSS_LOG_ENABLE=1
)
```

### 3. Trace 后端开关

`RAC` 日志最终依赖 [smtc_modem_hal_dbg_trace.h](modem_hal/smtc_modem_hal_dbg_trace.h) 中的 trace 宏。

当前默认值为：

- `MODEM_HAL_DBG_TRACE=ON`
- `MODEM_HAL_DBG_TRACE_COLOR=ON`
- `MODEM_HAL_DBG_TRACE_RP=OFF`
- `MODEM_HAL_DEEP_DBG_TRACE=OFF`

如需关闭 `RAC` 类 trace 输出，可在编译时设置：

```cmake
target_compile_definitions(${COMPONENT_LIB} PRIVATE
    MODEM_HAL_DBG_TRACE=0
)
```

如需关闭颜色输出，可设置：

```cmake
target_compile_definitions(${COMPONENT_LIB} PRIVATE
    MODEM_HAL_DBG_TRACE_COLOR=0
)
```

### 4. ESP_LOG 日志

组件中还有一部分日志直接使用 `ESP-IDF` 的 `ESP_LOGx`：

- [ralf_lr20xx.c](ralf/lr20xx_ralf/ralf_lr20xx.c)
  - 主要输出 `LR20xx` 的 `setup GFSK / LoRa / FLRC / CAD` 信息
- [ral_lr20xx.c](ral/lr20xx_ral/ral_lr20xx.c)
  - 主要输出部分 `PA` 和发射配置调试信息
- [smtc_modem_hal.c](modem_hal/smtc_modem_hal.c)
  - 主要输出 `panic` 和部分 `HAL` 运行信息

这部分日志不受 `RAC_LOG_*` 宏控制，而是受 `ESP-IDF` 自身日志等级控制。

常见控制方式包括：

1. 在 `menuconfig` 中调整默认日志等级
2. 在运行时调用 `esp_log_level_set("*", ESP_LOG_INFO);`
3. 针对特定 tag 调用 `esp_log_level_set("SMTC_MODEM", ESP_LOG_DEBUG);`

### 5. 应用层如何使用

如果你的应用希望直接复用组件自带的 `RAC` 风格日志，可显式包含：

- [smtc_rac_log.h](rac_api/smtc_rac_log.h)

示例：

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

如果你的应用只想使用 `ESP-IDF` 原生日志，则直接使用 `ESP_LOGI`、`ESP_LOGW`、`ESP_LOGE` 即可，不必依赖 `RAC_LOG_*` 宏。

## 默认硬件配置

当前 `Kconfig` 中提供的默认 `SPI` 与 `GPIO` 参数，已经按照 `L-LRMAM36-FANN4` 模组的典型连接方式进行预设。

因此，在使用 `L-LRMAM36-FANN4` 进行开发时，通常可以直接使用组件默认配置作为起点，无需在初始阶段手动调整 `SPI Host`、`NRST`、`BUSY`、`NSS`、`MOSI`、`MISO`、`SCLK` 以及 `DIO7` 等参数。

对于 `LR20XX / LR2021` 路径，组件当前支持在 `menuconfig` 中选择利尔达板型：

- `L-LRMAM36-FANN4`
- `L-LRMWP35-FANN4`

其中：

- 选择 `L-LRMAM36-FANN4` 时，引脚参数默认按该模组预设，不需要额外配置
- 选择 `L-LRMWP35-FANN4` 时，可在 `menuconfig` 中继续配置 `SPI` 与 `GPIO` 参数

当前第一版实现中，`L-LRMAM36-FANN4` 与 `L-LRMWP35-FANN4` 已分别对应独立的 `LR20XX BSP` 源文件，便于后续继续按板型细化晶振、功耗表、PA 和射频前端相关参数。

当前板型与 BSP 对应关系如下：

- `L-LRMAM36-FANN4` -> [ral_lr20xx_bsp_lrmam36_fann4.c](ral/lr20xx_ral/ral_lr20xx_bsp_lrmam36_fann4.c)
- `L-LRMWP35-FANN4` -> [ral_lr20xx_bsp_lrmwp35_fann4.c](ral/lr20xx_ral/ral_lr20xx_bsp_lrmwp35_fann4.c)
- `Generic / Custom LR2021 SPI board` -> [ral_lr20xx_bsp.c](ral/lr20xx_ral/ral_lr20xx_bsp.c)

## 自定义硬件适配说明

如果使用的是 `L-LRMAM36-FANN4`，可以直接沿用组件默认配置。
如果使用的是其他硬件方案，则需要按实际情况调整相关配置。

### 1. 晶振配置

当前 `LR20XX` 路径下的晶振相关配置由板型 BSP 决定：

- `L-LRMAM36-FANN4`：见 [ral_lr20xx_bsp_lrmam36_fann4.c](ral/lr20xx_ral/ral_lr20xx_bsp_lrmam36_fann4.c#L484)
- `L-LRMWP35-FANN4`：见 [ral_lr20xx_bsp_lrmwp35_fann4.c](ral/lr20xx_ral/ral_lr20xx_bsp_lrmwp35_fann4.c#L484)
- `Generic / Custom LR2021 SPI board`：见 [ral_lr20xx_bsp.c](ral/lr20xx_ral/ral_lr20xx_bsp.c#L484)

对于自定义 `ESP-IDF + LR2021 SPI` 硬件，建议按实际射频模组和参考设计确认以下内容：

- 板上使用的是 `XTAL` 还是 `TCXO`
- 如果使用 `TCXO`，需要相应调整 `ral_lr20xx_bsp_get_xosc_cfg(...)` 中的 `xosc_cfg`、供电电压和启动时间
- 如果使用 `XTAL`，需要根据实际晶振及板级寄生参数，评估是否要调整默认的 `XOSC trim` 参数

如果这部分配置与硬件不匹配，可能导致射频初始化异常、时钟稳定性问题或收发行为异常。

### 2. 引脚配置

对于 `L-LRMWP35-FANN4` 和自定义 `ESP-IDF + LR2021 SPI` 硬件，通常还需要在 `menuconfig` 中根据实际连线修改以下参数：

- `LR2021_RADIO_SPI_ID`
- `LR2021_NRST_GPIO`
- `LR2021_BUSY_GPIO`
- `LR2021_NSS_GPIO`
- `LR2021_SPI_MOSI`
- `LR2021_SPI_MISO`
- `LR2021_SPI_CLK`
- `LR20XX_DIO7_GPIO`

这些配置项定义见 [Kconfig](Kconfig)。

没有连接 NRST 时，`LR2021_NRST_GPIO` 可以设为 `-1`。这时 `ral_reset()` 无法复位芯片，只保证在下一条命令前先唤醒芯片。建议连接 NRST：不接的话，只重启 ESP32 时芯片会保留原来的状态，包括已加载的 PRAM。

如果你的硬件不是按 `L-LRMAM36-FANN4` 的默认连接方式设计，那么这里的参数应视为必须检查项，而不是可选项。

## 与 ESP-IDF 的集成方式

该组件按 ESP-IDF 组件方式组织，当前已经包含：

- [CMakeLists.txt](CMakeLists.txt)
- [idf_component.yml](idf_component.yml)
- [LICENSE](LICENSE)
- [NOTICE](NOTICE)

在工程中集成后，用户通常只需要：

1. 在 `menuconfig` 中选择目标射频芯片和 GPIO/SPI 参数
2. 在应用层包含所需头文件
3. 根据所选层级调用 `RAC`、`RALF` 或 `RAL` 接口

## 示例工程

如需参考该组件的使用方式、工程集成方法和基础调用示例，请使用以下例程仓库：

- [esp32_lora_samples](https://github.com/lierda-iot/esp32_lora_samples.git)

应用示例：

- [DoorCam-LR](https://github.com/lierda-iot/DoorCam-LR)：可视门铃，通过 FLRC 突发传输实时视频和双向语音，基于本组件的 `RAL` 接口开发。它的接收方式见"使用注意事项"中的"接收状态标志"一节。

## 文档链接

利尔达产品资料（钉钉知识库，公开访问）：

- [L-LRMAM36-FANN4（AM36）](https://alidocs.dingtalk.com/i/nodes/dxXB52LJqnGE6GN4cQRNK0PX8qjMp697?utm_scene=team_space)
- [L-LRMWP35-FANN4（WP35）](https://alidocs.dingtalk.com/i/nodes/gpG2NdyVX32N52zwuAREozYpWMwvDqPk?utm_scene=team_space)

## 快速开始

下面给出一个推荐的最小接入流程。

### 1. 将组件加入工程

从 [ESP 组件注册表](https://components.espressif.com/components/lierda-iot/esp_lora_driver) 添加组件：

```bash
idf.py add-dependency "lierda-iot/esp_lora_driver^1.0.0"
```

如果要直接使用源码，见下文“以源码形式作为本地组件使用”。

### 2. 配置目标芯片和硬件参数

运行：

```bash
idf.py menuconfig
```

然后在 `Liot LR2021 (IDF)` 菜单中完成以下配置：

- 选择目标射频芯片家族
- 如果使用 `L-LRMAM36-FANN4`，可以直接使用默认参数
- 如果使用 `L-LRMWP35-FANN4`，先选择对应板型，再按需要调整 `SPI` 与 `GPIO` 参数
- 如果使用其他自定义 `ESP-IDF + LR2021 SPI` 硬件，按实际连接修改 `SPI` 与 `GPIO` 参数，并检查晶振配置是否与硬件一致

### 3. 在应用代码中包含组件入口头

```c
#include "LiotLr2021.h"
```

### 4. 按所需层级调用接口

你可以在单入口头基础上直接使用以下层级的接口：

- `RAC API`：适合应用层直接发起无线事务
- `RAC`：适合直接控制无线访问管理逻辑
- `RALF`：适合更细粒度的射频功能配置
- `RAL`：适合底层射频抽象控制

### 5. 编译工程

```bash
idf.py build
```

## 使用注意事项

### OOK 调制（LR20xx）

LR20xx 可以通过 `RALF` 和 `RAL` 使用 OOK；其他芯片调用这些接口会返回 `RAL_STATUS_UNSUPPORTED_FEATURE`。

- `ralf_setup_ook()` 配合 `ralf_params_ook_t`，一次完成包类型、频率、发射功率、包参数、调制参数、接收检测器、同步字、CRC、地址和白化的配置
- `ral_set_ook_*()`、`ral_get_ook_rx_pkt_status()`、`ral_get_ook_time_on_air_in_ms()` 可以单独调用每一步
- 收发与其他调制方式一样，使用 `ral_set_tx()` / `ral_set_rx()` 和中断相关接口

示例：

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
    .whitening_polynomial = 0,  // 不使用白化
};

ralf_setup_ook( &radio, &ook_params );
```

说明：

- `pattern_len_in_bits` 填图样的实际位数，驱动写入芯片时会自动减 1
- `whitening_polynomial` 不为 0 时才开启白化；`whitening_polynomial` 和 `whitening_seed` 是 12 位数值，`whitening_bit_index` 取值 0～15（LR20xx 参考例程用的是 1）
- LR20xx 驱动文档指出，OOK 使用显式包头但不开 CRC 时接收可能出错，所以带长度包头时请开启 CRC
- 芯片自动计算的 OOK 检测门限偏保守；如果误包率偏高，可以调用 `lr20xx_workarounds_ook_set_detection_threshold_level()` 调整，详见 [radio_drivers/lr20xx_driver/README.md](radio_drivers/lr20xx_driver/README.md)
- `ral_get_ook_time_on_air_in_ms()` 在比特率为 0 时返回 0；LR20xx 驱动算不了的配置也返回 0：16 位长度包头（`RAL_OOK_PKT_VAR_LEN_16_BITS`）和 Biphase Mark 编码

### 接收状态标志（LR20xx）

LR20xx 的所有中断都可以通过 `RAL` 使用（`ral_set_dio_irq_params()`、`ral_get_irq_status()`、`ral_get_and_clear_irq_status()`、`ral_clear_irq_status()`）。除了常用标志，还包括：

- `RAL_IRQ_RX_LEN_ERROR`：收到的包比配置的载荷长度长，与 `RAL_IRQ_RX_DONE` 一起上报
- `RAL_IRQ_RX_ADDR_ERROR`：地址不匹配，包被丢弃
- `RAL_IRQ_RTTOF_REQ_VALID`、`RAL_IRQ_RX_HDR_TIMESTAMP`、`RAL_IRQ_LOW_BATTERY`、`RAL_IRQ_PA_OVP_OCP`、`RAL_IRQ_LR_FHSS_NEW_TABLE`、`RAL_IRQ_LR_FHSS_NEW_PAYLOAD`

`RAL_IRQ_ALL` 会清除 LR20xx 的全部中断，包括厂商掩码 `LR20XX_SYSTEM_IRQ_ALL_MASK` 漏掉的 `PA_OVP_OCP`。

驱动负责上报这些标志；要不要用硬件 CRC、每个标志怎么处理，由应用决定。`RAC`（radio planner）只根据 `RX_DONE`、包头错误和 CRC 错误判断接收结果，读完就清除中断，所以需要其他标志的应用请直接使用 `RAL`。

建议：

- **每次 `RX_DONE` 对应一个包**（单包收发的 LoRa、GFSK、FLRC、OOK）：开启硬件 CRC，`RX_DONE` 同时带有 `RAL_IRQ_RX_CRC_ERROR` 或 `RAL_IRQ_RX_LEN_ERROR` 时按坏包处理，与 LR20xx 参考例程一致。
- **FLRC 连续突发接收**，应用醒来时 RX FIFO 里可能已有多个包：这些包的中断标志会叠加在一起，分不出是哪个包出错。建议按 FIFO 水位读取，在软件里恢复包边界，并用应用层 CRC 逐包校验，这时可以关闭硬件 CRC。[DoorCam-LR](https://github.com/lierda-iot/DoorCam-LR) 就是这样做的，见 `main/radio_ping.cpp` 中的 `flrc_packet_params()` 和 `handle_rx_packet()`。

### 不保留 RAM 的睡眠

组件自身只使用 `ral_set_sleep( radio, true )`，睡眠期间保留芯片配置和 PRAM。

调用 `ral_set_sleep( radio, false )` 后，芯片的配置和 PRAM 都会丢失。唤醒后需要重新调用 `ral_init()`：它会加载并校验 PRAM，恢复时钟、DIO、校准和 FIFO 配置。之后再按上电后的流程重新配置射频参数，例如 `ralf_setup_lora()`、`ral_set_dio_irq_params()`。不需要单独复位，HAL 在发送第一条命令时会自动唤醒芯片。

### 以源码形式作为本地组件使用

通过 `idf_component.yml` 的 `override_path` 或 `path` 引用本仓库，或把它放进工程的 `components/` 目录时，ESP-IDF 以目录名作为组件名。应用代码引用的组件名是 `esp_lora_driver`，所以目录必须命名为 `esp_lora_driver`，例如：

```bash
git clone https://github.com/lierda-iot/esp32_lora_driver.git esp_lora_driver
```

另外，`override_path` 以相对路径保存，在 Windows 上工程和组件需要放在同一个盘符下。

## 推荐使用方式

从组件维护角度，推荐按以下优先级使用：

- 应用层优先使用 `RAC API` 或 `RAC`
- 需要更细粒度射频控制时使用 `RALF`
- 仅在需要最底层芯片操作抽象时使用 `RAL`

这样做的好处是：

- 上层代码与具体芯片解耦更好
- 后续适配其他 Semtech 芯片时复用性更高
- 组件内部实现调整时，对应用层影响更小

## 版本信息

当前 `RAC API` 版本定义见 [smtc_rac_version.h](rac_api/smtc_rac_version.h#L61)：

- Major: `1`
- Minor: `1`
- Patch: `2`

## 许可证说明

本组件基于 Semtech 上游代码修改和移植，主许可证采用 `The Clear BSD License`，对应 SPDX 标识符为 `BSD-3-Clause-Clear`。

相关文件：

- [LICENSE](LICENSE)
- [NOTICE](NOTICE)

如果分发源码或二进制，请保留上游版权声明、许可证文本和免责声明。

## 发布说明

### 1.0.0

本版本包含 0.0.5 之后的全部改动。

同步上游：

- Semtech `RAC API` 1.0.0 → 1.1.2，LR20xx 驱动 v1.3.4 → v2.0.2（含 PRAM 加载），LR11xx 驱动 v2.7.0 → v3.0.0。

从 0.0.5 升级时需要改代码的接口变化：

- FLRC 调制参数：`ral_flrc_mod_params_t` 的 `br_in_bps` 和 `bw_dsb_in_hz` 改为 `raw_bit_rate`（`RAL_FLRC_RAW_BIT_RATE_0_260_MBPS` 到 `RAL_FLRC_RAW_BIT_RATE_2_600_MBPS`）。
- FLRC 前导码：`ral_flrc_pkt_params_t` 的 `preamble_len_in_bits` 改为 `preamble_len`（`RAL_FLRC_PREAMBLE_LENGTH_4_BITS` 到 `RAL_FLRC_PREAMBLE_LENGTH_32_BITS`）。需要更长的前导码（36 到 16380 位，4 的倍数）时，先设为 `RAL_FLRC_PREAMBLE_LENGTH_32_BITS`，再调用 `ral_set_flrc_long_preamble_len_in_bits()`。这取代了 0.0.5 的自动处理。
- FLRC 同步字：`ralf_params_flrc_t` 和 `RAC` FLRC 参数里的 `sync_word` 改为 3 个指针的数组，每个同步字一个。
- LoRa：`ralf_params_lora_t` 和 `RAC` LoRa 参数里的 `symb_nb_timeout` 改为 `uint16_t`。

`RAC` 新增：

- FLRC 连续收发：`smtc_rac_flrc_burst()`、`smtc_rac_flrc_burst_rx_done()` 和 `smtc_rac_radio_flrc_burst_params_t`。
- 立即占用射频：`smtc_rac_immediate_radio_access()` 和 `smtc_rac_release_immediate_radio_access()`。
- 活动超时：`smtc_rac_set_active_time_out()` 和 `smtc_rac_release_active_time_out()`。
- `smtc_rac_get_callback_radio_id()`、`smtc_rac_get_context_private()`、`smtc_rac_set_context_private()`，以及 `smtc_rac_context_t` 的 `keep_radio_awake`。

`RAL` 和 `RALF` 新增：

- LR20xx 的 OOK：`RAL` 的 9 个接口（参数、检测器、同步字、CRC、地址、白化、包状态、空中时间）和 `ralf_setup_ook()`。
- FIFO 访问：`ral_get_pkt_size()`、`ral_get_data_rx_buffer()`、`ral_clear_rx_fifo()`、`ral_clear_tx_fifo()`、`ral_get_fifo_irq()` 和 `ral_get_and_clear_fifo_irq()`。
- GFSK：`ral_set_gfsk_whitening_seed_comp()`，以及 CRC 类型 `RAL_GFSK_CRC_3_BYTES_INV`、`RAL_GFSK_CRC_4_BYTES` 和 `RAL_GFSK_CRC_4_BYTES_INV`。
- LoRa：LR20xx 的带宽 `RAL_LORA_BW_083_KHZ` 和 `RAL_LORA_BW_101_KHZ`，`ral_lora_rx_pkt_status_t` 的 `freq_offset_hz`。
- FLRC：LR20xx 上 `crc_seed` 生效，`ralf_params_flrc_t` 增加 `is_tx`。
- 中断：RAL 标志覆盖 LR20xx 全部 28 个中断位。新增 `RAL_IRQ_RX_LEN_ERROR`、`RAL_IRQ_RX_ADDR_ERROR`、`RAL_IRQ_CMD_ERROR`、`RAL_IRQ_ERROR`、`RAL_IRQ_RTTOF_REQ_VALID`、`RAL_IRQ_RX_HDR_TIMESTAMP`、`RAL_IRQ_LOW_BATTERY`、`RAL_IRQ_PA_OVP_OCP`、`RAL_IRQ_LR_FHSS_NEW_TABLE` 和 `RAL_IRQ_LR_FHSS_NEW_PAYLOAD`。

初始化与校准：

- 初始化检查 `lr20xx_system_init()` 的返回值并校验 PRAM；晶振配置后进入 standby XOSC；使用 TCXO 时执行系统校准；配置 DIO 前清除旧的 IRQ。初始化时的前端校准频点为 470、897.5、2441 MHz。
- 切换频点时，变化超过 50 MHz 重新校准 PLL/AAF，超过 10 MHz 做前端校准，最后设置 RX path 和 boost。

修复：

- `ral_reset()` 在第一次复位脉冲前配置 NRST 引脚，复位低电平为精确 1 ms。
- SPI 单次传输可以读写满 FIFO（2 字节命令加 1024 字节）：上限 1026 字节，入口检查长度，写入失败时返回错误，使用静态 DMA 缓冲，FIFO 读取偏移按实际命令长度计算。
- `LR2021_NRST_GPIO` 设为 `-1`（未连接）时不再执行 `1ULL << -1`。
- 初始化读取系统错误时检查返回值，错误码变量已初始化。
- `ral_cal_img()` 对 2.4 GHz 频点使用 HF 接收路径。
- 32 位数值的日志格式符。

硬件与配置：

- `L-LRMAM36-FANN4` 和 `L-LRMWP35-FANN4`：晶振微调 XTA、XTB 设为 11（原为 0），启用芯片内部负载电容。
- PA 和 RX 路径在 1.5 GHz 从 LF 切到 HF（原为 1.6 GHz）。
- NSS 唤醒脉冲为 100 µs（原为 1 ms）。
- `menuconfig`：LR20xx 芯片型号（`LR2012`、`LR2021`、`LR2022`）；SX126x 芯片型号（`SX1261`、`SX1262`、`SX1268`）及 BPSK（默认关）、LR-FHSS（默认开）选项；LR11xx 的 SPI CRC 和地理定位选项；Radio Planner 余量延时（默认 8 ms）和地区占空比配置（不启用、EU 868、RU 864）。日志档位同时设置 `RAC_LOG_ENABLE`。

文档与打包：

- 中文 README 改名为 `README_CN.md`，注册表页面显示语言切换。
- README：注册表安装命令、FLRC 速率、OOK 用法、接收状态标志的使用建议（参考 DoorCam-LR）、`LR2021_NRST_GPIO = -1`、不保留 RAM 的睡眠，以及以源码形式作为本地组件使用。
- 注册表标签加 `flrc`、`semtech`、`esp32-s3`，去掉 `lr20xx`、`esp-idf`。
- 删除未使用的 `ral/base64.c` 和 `ral/base64.h`。
- `LICENSE` 增加利尔达对 ESP-IDF 适配部分的版权行。

### 0.0.5

- LR20xx 支持超过 32 位的 FLRC 前导码，最长 16380 位，步长 4 位，通过 `ral_flrc_pkt_params_t` 的 `preamble_len_in_bits` 设置。

### 0.0.4

- README 默认英文，并链接到中文 README（`README.zh-CN.md`）。删除 `README.en.md`。

### 0.0.3

- 增加英文 README（`README.en.md`）；中文 README 同时提供为 `README.zh-CN.md`。
- `modem_hal` 和 `lr20xx_hal` 中的中文注释翻译为英文（只改注释）。

### 0.0.2

- 首个版本：基于 Semtech `smtc_rac_lib` 的 ESP-IDF 组件，包含 `RAC API`、带 Radio Planner 的 `RAC`、`RALF`、`RAL`，以及 LR20xx、LR11xx 和 SX126x 驱动。
- ESP-IDF 的 SPI、GPIO、定时器和日志 HAL，以及单入口头文件 `LiotLr2021.h`。
- `menuconfig` 中的射频芯片家族、板型（`L-LRMAM36-FANN4` 或 `L-LRMWP35-FANN4`）、SPI、GPIO 和日志档位选项。
- `LICENSE` 和 `NOTICE`（The Clear BSD License）。

## 维护说明

这个组件当前保留了较强的上游结构痕迹，这是有意为之。这样做的主要目的是：

- 方便和 Semtech 上游代码进行差异比对
- 降低后续继续同步或回溯问题时的维护成本
- 在不破坏组件对外接口的前提下，逐步整理内部结构

因此，对外接口统一通过 `include/` 暴露，而内部实现目录保持原有分层布局。
