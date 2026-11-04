<p align="center">
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="docs/brand/ubot-environment-light.svg">
    <source media="(prefers-color-scheme: light)" srcset="docs/brand/ubot-environment-dark.svg">
    <img alt="uBot Environment" src="docs/brand/ubot-environment-dark.svg" height="100">
  </picture>
</p>
<h1 align="center">uBot Environment</h1>

[简体中文](README.zh-CN.md)

Shared wind for robotics simulation in Unreal Engine 5.5. The unreleased 0.2.0 source requires uBotCore `^0.2.0` and has no dependency on uBotAerial. It is not yet installable through uBot Manager.

Rebuild all dependent modules together; do not mix 0.1.x DLLs with 0.2.0. See [Core API migration](https://astreox.github.io/uBotDocs/develop/migration/#core-旧-ros-api-迁移020-开发版).

## Wind

Enable **UBotEnvironment** in the project and place **uBot Wind** in a level. Its settings provide:

- Constant wind velocity on UE world axes, in m/s: the direction the air moves.
- Gust standard deviation per world axis, in m/s. Zero disables gusts.
- Gust correlation time (default 2 s, minimum 0.05 s) and a random seed.

Wind is uniform in space and varies with simulation time. The same settings, seed and time produce the same gust velocity. Several enabled wind actors add together. Actor position and rotation do not change the wind; the editor arrow shows the configured constant velocity.

Use **Set Wind Settings** and **Set Wind Enabled** to change an actor at run time. **Get uBot Wind Subsystem → Set Scripted Wind** adds one scripted source to the actors; pass `Enabled = false` to clear that source. **Get Wind Velocity At** returns the current velocity at a location in world centimeters, in m/s, using the uBot simulation clock. All sources are initially calm.

This plugin owns `IUBotWindProvider` and `UUBotWindQuerySubsystem`. `UUBotWindSubsystem` implements the provider and registers with the current world's query subsystem. Consumers query wind at their own simulation time, passing world position in centimeters and receiving velocity on UE world axes in m/s. Registration and queries run on the game thread. Without a live provider, the query returns zero. Each world has its own sources.

The query keeps one weak provider reference; this plugin owns wind contracts, queries, generation and source addition. Consumers own their response. uBotAerial uses an optional build dependency: it compiles without this plugin when absent or disabled and uses zero wind; enabling the integration requires rebuilding the affected modules. No separate connector plugin is needed. Other environmental effects should reuse existing contracts: perception media can implement Core's `IUBotPerceptionMedium`, which uBotSensor already reads. This is an integration rule, not an implementation of smoke or fog in this version. See the [environment architecture contract](https://astreox.github.io/uBotDocs/develop/package-authoring/#环境与使用者的职责边界).

The field does not model local wind zones, spatial turbulence, terrain or building effects, or rotor wakes. This plugin supplies wind velocity; consumers implement their own physical response.

## Migrating wind from uBotAerial

The wind implementation previously shipped in uBotAerial. Its settings, defaults, seeded gust calculation, source addition and Blueprint function signatures are preserved here.

Enable **UBotEnvironment before loading existing wind Blueprints or levels**. The plugin includes exact class redirects for `UBotWind` and `UBotWindSubsystem`, and a struct redirect for `UBotWindSettings`, from `/Script/UBotAerial` to `/Script/UBotEnvironment`. These redirects preserve references in existing assets, including wind actor properties, subsystem nodes and wind settings pins. Keep this plugin enabled for those assets; uBotAerial alone does not supply the wind field.

Native C++ users of `AUBotWind`, `UUBotWindSubsystem`, `FUBotWindSettings` or `UBot::Wind` must add `UBotEnvironment` to their module dependencies and rebuild. Include `UBotEnvironmentTypes.h` for `FUBotWindSettings`; the wind actor and subsystem retain their header names. General wind consumers should use this plugin's `UUBotWindQuerySubsystem` instead of depending on the concrete field implementation.

## Migrating wind queries from uBotCore

In the 0.2.0 development source, `IUBotWindProvider` and `UUBotWindQuerySubsystem` move from uBotCore into this plugin. Their C++ names, headers, units and query semantics remain unchanged. Add a `UBotEnvironment` module dependency for code that uses them and rebuild all affected modules. Core no longer exports these types or retains compatibility wrappers.

With uBotEnvironment enabled, class redirects resolve saved `/Script/UBotCore.UBotWindProvider` and `/Script/UBotCore.UBotWindQuerySubsystem` references to their `/Script/UBotEnvironment` equivalents. Enable this plugin before loading affected assets, then compile and resave them. Direct wind assets and API references require this plugin; uBotAerial's independent flight API remains available without it.

## Tests

Run the `UBotEnvironment.Wind` automation prefix in a host project with this plugin enabled. It covers deterministic gusts, source addition and run-time changes, provider routing, weak provider lifetime, isolation between worlds, and loading the former Core type paths through class redirects.

## License

Apache-2.0. See [LICENSE](LICENSE) and [NOTICE](NOTICE).
