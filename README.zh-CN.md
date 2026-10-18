<p align="center">
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="docs/brand/ubot-environment-light.svg">
    <source media="(prefers-color-scheme: light)" srcset="docs/brand/ubot-environment-dark.svg">
    <img alt="uBot Environment" src="docs/brand/ubot-environment-dark.svg" height="100">
  </picture>
</p>
<h1 align="center">uBot Environment</h1>

[English](README.md)

Unreal Engine 5.5 机器人仿真的共享风场插件，依赖 uBotCore 0.1.4 及以上的 0.1 系列版本，不依赖 uBotAerial。

## 风场

在工程中启用 **UBotEnvironment**，向关卡放置 **uBot Wind**。可配置：

- 恒定风速：UE 世界坐标，单位 m/s，方向表示空气运动方向。
- 各世界坐标轴的阵风标准差：单位 m/s，零表示关闭该轴阵风。
- 阵风相关时间：默认 2 s，最小 0.05 s；以及随机种子。

风场在空间中均匀，随仿真时间变化。相同配置、种子和仿真时刻得到相同阵风。多个已启用的风 Actor 叠加；Actor 的位置和旋转不改变风场，编辑器箭头显示配置的恒定风方向。

运行时可调用 **Set Wind Settings** 和 **Set Wind Enabled**。通过 **Get uBot Wind Subsystem → Set Scripted Wind** 添加一个脚本风源，它与关卡 Actor 的风叠加；传入 `Enabled = false` 清除脚本风源。**Get Wind Velocity At** 接收世界坐标位置（cm），按 uBot 仿真时钟返回当前风速（m/s）。所有风源的默认配置都是无风。

`UUBotWindSubsystem` 实现 uBotCore 的 `IUBotWindProvider`，并向所属世界的 `UUBotWindQuerySubsystem` 注册。使用风的插件只需依赖 uBotCore，按自己的仿真时刻查询；没有风提供者时返回零风速。各世界的风源互相隔离。

当前不模拟局部风区、空间湍流、地形或建筑物绕流、旋翼尾流。本插件提供风速，受风对象自行计算物理响应。

## 从 uBotAerial 迁移

这套风场原先位于 uBotAerial。设置字段、默认值、带种子的阵风算法、风源叠加和蓝图函数签名保持不变。

**打开已有风场蓝图或关卡之前，请先启用 UBotEnvironment。** 插件包含 `UBotWind`、`UBotWindSubsystem` 的精确类重定向，以及 `UBotWindSettings` 的结构体重定向，将它们从 `/Script/UBotAerial` 指向 `/Script/UBotEnvironment`。已有资产中的风 Actor 属性、子系统节点和风设置引脚通过这些重定向解析。使用这些资产时应继续启用本插件；单独启用 uBotAerial 不再提供风场。

直接使用 `AUBotWind`、`UUBotWindSubsystem`、`FUBotWindSettings` 或 `UBot::Wind` 的 C++ 模块需要添加 `UBotEnvironment` 模块依赖并重新编译。`FUBotWindSettings` 改为包含 `UBotEnvironmentTypes.h`；风 Actor 和子系统头文件名保持不变。只需读取风速的代码应改用 uBotCore 查询接口。

## 测试

在启用本插件的宿主工程中运行 `UBotEnvironment.Wind` 自动化测试，覆盖阵风复现、风源叠加与运行时变更、uBotCore 契约查询、多个世界之间的隔离。

## 许可

Apache-2.0，见 [LICENSE](LICENSE) 和 [NOTICE](NOTICE)。
