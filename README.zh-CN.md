<p align="center">
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="docs/brand/ubot-environment-light.svg">
    <source media="(prefers-color-scheme: light)" srcset="docs/brand/ubot-environment-dark.svg">
    <img alt="uBot Environment" src="docs/brand/ubot-environment-dark.svg" height="100">
  </picture>
</p>
<h1 align="center">uBot Environment</h1>

[English](README.md)

Unreal Engine 5.5 机器人仿真的共享风场插件。尚未发布的 0.2.0 源码要求 uBotCore `^0.2.0`，不依赖 uBotAerial，暂不能通过 uBot Manager 安装。

所有依赖模块须统一重编，不能混用 0.1.x 与 0.2.0 DLL，见 [Core API 迁移](https://astreox.github.io/uBotDocs/develop/migration/#core-旧-ros-api-迁移020-开发版)。

## 风场

在工程中启用 **UBotEnvironment**，向关卡放置 **uBot Wind**。可配置：

- 恒定风速：UE 世界坐标，单位 m/s，方向表示空气运动方向。
- 各世界坐标轴的阵风标准差：单位 m/s，零表示关闭该轴阵风。
- 阵风相关时间：默认 2 s，最小 0.05 s；以及随机种子。

风场在空间中均匀，随仿真时间变化。相同配置、种子和仿真时刻得到相同阵风。多个已启用的风 Actor 叠加；Actor 的位置和旋转不改变风场，编辑器箭头显示配置的恒定风方向。

运行时可调用 **Set Wind Settings** 和 **Set Wind Enabled**。通过 **Get uBot Wind Subsystem → Set Scripted Wind** 添加一个脚本风源，它与关卡 Actor 的风叠加；传入 `Enabled = false` 清除脚本风源。**Get Wind Velocity At** 接收世界坐标位置（cm），按 uBot 仿真时钟返回当前风速（m/s）。所有风源的默认配置都是无风。

`IUBotWindProvider` 和 `UUBotWindQuerySubsystem` 均由本插件提供。`UUBotWindSubsystem` 实现提供者接口，并向所属世界的查询子系统注册。使用者按自己的仿真时刻查询，传入世界位置（cm），获得 UE 世界坐标轴上的风速（m/s）。注册和查询在游戏线程执行；没有有效提供者时返回零风速。各世界的风源互相隔离。

查询只保存一个提供者的弱引用；风契约、查询、生成和风源叠加均归本插件，使用者负责响应。uBotAerial 采用可选编译依赖：本插件缺失或禁用时仍可编译，风速取零；启用接入后须重编受影响模块。无需新增连接插件。其他环境效果应先复用已有契约：感知介质可以实现 Core 的 `IUBotPerceptionMedium`，由 uBotSensor 已有逻辑读取。这是接入约定，当前版本尚未实现烟雾或雾。见[环境架构约定](https://astreox.github.io/uBotDocs/develop/package-authoring/#环境与使用者的职责边界)。

当前不模拟局部风区、空间湍流、地形或建筑物绕流、旋翼尾流。本插件提供风速，受风对象自行计算物理响应。

## 从 uBotAerial 迁移

这套风场原先位于 uBotAerial。设置字段、默认值、带种子的阵风算法、风源叠加和蓝图函数签名保持不变。

**打开已有风场蓝图或关卡之前，请先启用 UBotEnvironment。** 插件包含 `UBotWind`、`UBotWindSubsystem` 的精确类重定向，以及 `UBotWindSettings` 的结构体重定向，将它们从 `/Script/UBotAerial` 指向 `/Script/UBotEnvironment`。已有资产中的风 Actor 属性、子系统节点和风设置引脚通过这些重定向解析。使用这些资产时应继续启用本插件；单独启用 uBotAerial 不再提供风场。

直接使用 `AUBotWind`、`UUBotWindSubsystem`、`FUBotWindSettings` 或 `UBot::Wind` 的 C++ 模块需要添加 `UBotEnvironment` 模块依赖并重新编译。`FUBotWindSettings` 改为包含 `UBotEnvironmentTypes.h`；风 Actor 和子系统头文件名保持不变。只需读取风速的代码应使用本插件的 `UUBotWindQuerySubsystem`，避免依赖具体风场实现。

## 从 uBotCore 迁移风查询

0.2.0 开发源码将 `IUBotWindProvider`、`UUBotWindQuerySubsystem` 从 uBotCore 移到本插件。C++ 类型名、头文件名、单位和查询语义保持不变。使用这些接口的代码须添加 `UBotEnvironment` 模块依赖，并统一重编受影响模块；Core 不再导出这些类型，也不保留兼容包装。

启用 uBotEnvironment 后，类重定向将已保存的 `/Script/UBotCore.UBotWindProvider`、`/Script/UBotCore.UBotWindQuerySubsystem` 引用解析为对应的 `/Script/UBotEnvironment` 类型。加载受影响资产前先启用本插件，再编译并重新保存。直接引用风类型的资产与代码需要本插件；uBotAerial 独立的飞行 API 可在缺少本插件时继续使用。

## 测试

在启用本插件的宿主工程中运行 `UBotEnvironment.Wind` 自动化测试，覆盖阵风复现、风源叠加与运行时变更、提供者查询、提供者弱引用生命周期、多个世界之间的隔离，以及通过类重定向加载原 Core 类型路径。

## 许可

Apache-2.0，见 [LICENSE](LICENSE) 和 [NOTICE](NOTICE)。
