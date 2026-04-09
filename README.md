# LiotLr2021

`LiotLr2021` 是一个面向 ESP-IDF 的无线收发组件，用于集成 Semtech 系列射频芯片，当前以 `LR20xx/LR2021` 为主要目标，同时保留了 `SX126x` 和 `LR11xx` 的分层适配结构。

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

- [include/LiotLr2021.h](/home/cheny/code_temp/Lora/LiotLr2021/include/LiotLr2021.h)

也就是说，推荐的接入方式是：

```c
#include "LiotLr2021.h"
```

`LiotLr2021.h` 本身会聚合以下几层公共接口：

- `RAC API`
  - 来自 [smtc_rac_api.h](/home/cheny/code_temp/Lora/LiotLr2021/rac_api/smtc_rac_api.h)
  - 适合应用层直接发起无线事务、管理调度上下文和使用统一 API
- `RAC`
  - 来自 [smtc_rac.h](/home/cheny/code_temp/Lora/LiotLr2021/rac/smtc_rac.h)
  - 适合需要直接操作无线访问控制层的场景
- `RALF`
  - 来自 [ralf.h](/home/cheny/code_temp/Lora/LiotLr2021/ralf/ralf.h)
  - 适合需要使用射频功能层接口的场景
- `RAL`
  - 来自 [ral.h](/home/cheny/code_temp/Lora/LiotLr2021/ral/ral.h)
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

组件通过 [Kconfig](/home/cheny/code_temp/Lora/LiotLr2021/Kconfig) 暴露配置项，主要包括：

- 射频芯片家族选择
  - `SX126x`
  - `LR11xx`
  - `LR20xx`
- `LR2021` 的 SPI Host 选择
- `L-LRMWP35-FANN4` 或 `Generic / Custom LR2021 SPI board` 下的 `NRST`、`BUSY`、`NSS`、`MOSI`、`MISO`、`SCLK` 等 GPIO 分配
- `L-LRMWP35-FANN4` 或 `Generic / Custom LR2021 SPI board` 下的 `LR20xx` 相关 `DIO` 配置

当前默认目标为 `LR20xx`，适合作为 `LR2021` 的默认配置基础。

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

- `L-LRMAM36-FANN4` -> [ral_lr20xx_bsp_lrmam36_fann4.c](/home/cheny/code_temp/Lora/LiotLr2021/ral/lr20xx_ral/ral_lr20xx_bsp_lrmam36_fann4.c)
- `L-LRMWP35-FANN4` -> [ral_lr20xx_bsp_lrmwp35_fann4.c](/home/cheny/code_temp/Lora/LiotLr2021/ral/lr20xx_ral/ral_lr20xx_bsp_lrmwp35_fann4.c)
- `Generic / Custom LR2021 SPI board` -> [ral_lr20xx_bsp.c](/home/cheny/code_temp/Lora/LiotLr2021/ral/lr20xx_ral/ral_lr20xx_bsp.c)

## 自定义硬件适配说明

如果使用的是 `L-LRMAM36-FANN4`，可以直接沿用组件默认配置。  
如果使用的是其他硬件方案，则需要按实际情况调整相关配置。

### 1. 晶振配置

当前 `LR20XX` 路径下的晶振相关配置由板型 BSP 决定：

- `L-LRMAM36-FANN4`：见 [ral_lr20xx_bsp_lrmam36_fann4.c](/home/cheny/code_temp/Lora/LiotLr2021/ral/lr20xx_ral/ral_lr20xx_bsp_lrmam36_fann4.c#L484)
- `L-LRMWP35-FANN4`：见 [ral_lr20xx_bsp_lrmwp35_fann4.c](/home/cheny/code_temp/Lora/LiotLr2021/ral/lr20xx_ral/ral_lr20xx_bsp_lrmwp35_fann4.c#L484)
- `Generic / Custom LR2021 SPI board`：见 [ral_lr20xx_bsp.c](/home/cheny/code_temp/Lora/LiotLr2021/ral/lr20xx_ral/ral_lr20xx_bsp.c#L484)

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

这些配置项定义见 [Kconfig](/home/cheny/code_temp/Lora/LiotLr2021/Kconfig)。

如果你的硬件不是按 `L-LRMAM36-FANN4` 的默认连接方式设计，那么这里的参数应视为必须检查项，而不是可选项。

## 与 ESP-IDF 的集成方式

该组件按 ESP-IDF 组件方式组织，当前已经包含：

- [CMakeLists.txt](/home/cheny/code_temp/Lora/LiotLr2021/CMakeLists.txt)
- [idf_component.yml](/home/cheny/code_temp/Lora/LiotLr2021/idf_component.yml)
- [LICENSE](/home/cheny/code_temp/Lora/LiotLr2021/LICENSE)
- [NOTICE](/home/cheny/code_temp/Lora/LiotLr2021/NOTICE)

在工程中集成后，用户通常只需要：

1. 在 `menuconfig` 中选择目标射频芯片和 GPIO/SPI 参数
2. 在应用层包含所需头文件
3. 根据所选层级调用 `RAC`、`RALF` 或 `RAL` 接口

## 快速开始

下面给出一个推荐的最小接入流程。

### 1. 将组件加入工程

将当前组件作为本地组件加入 ESP-IDF 工程，并确保工程可以在构建系统中找到它。

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

如果当前目标是 `LR20XX / LR2021` 路径，组件已经完成过最小工程构建验证，可作为当前主要集成路径使用。

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

当前 `RAC API` 版本定义见 [smtc_rac_version.h](/home/cheny/code_temp/Lora/LiotLr2021/rac_api/smtc_rac_version.h#L1)：

- Major: `1`
- Minor: `0`
- Patch: `0`

## 许可证说明

本组件基于 Semtech 上游代码修改和移植，主许可证采用 `The Clear BSD License`，对应 SPDX 标识符为 `BSD-3-Clause-Clear`。

相关文件：

- [LICENSE](/home/cheny/code_temp/Lora/LiotLr2021/LICENSE)
- [NOTICE](/home/cheny/code_temp/Lora/LiotLr2021/NOTICE)

如果分发源码或二进制，请保留上游版权声明、许可证文本和免责声明。

## 发布说明

如果后续要将该组件发布到 ESP Component Registry，建议在发布前再确认以下事项：

- 更新 [idf_component.yml](/home/cheny/code_temp/Lora/LiotLr2021/idf_component.yml) 中的 `url`
- 补充使用例程链接
- 进行至少一次目标芯片配置下的编译验证
- 检查所有公开头文件是否确实属于稳定外部接口

当前验证状态：

- `L-LRMAM36-FANN4` 已完成最小 ESP-IDF 工程构建验证
- `L-LRMWP35-FANN4` 当前处于功能验证阶段，性能参数尚未完成调优
- `SX126X` / `LR11XX` 分支当前仍视为“未完成验证”

说明：

- 本轮审查中，最小工程已实际完成 `L-LRMAM36-FANN4` 对应路径编译
- `L-LRMWP35-FANN4` 已拆分为独立 BSP，并完成基础功能路径接入，但板级性能参数仍需后续细化
- 对 `SX126X` / `LR11XX` 的条件分支尚未完成独立有效的分支构建确认，因此发布时不应默认宣称这两个分支已验证可用

## 维护说明

这个组件当前保留了较强的上游结构痕迹，这是有意为之。这样做的主要目的是：

- 方便和 Semtech 上游代码进行差异比对
- 降低后续继续同步或回溯问题时的维护成本
- 在不破坏组件对外接口的前提下，逐步整理内部结构

因此，对外接口统一通过 `include/` 暴露，而内部实现目录保持原有分层布局。
