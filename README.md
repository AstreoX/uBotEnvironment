<p align="center">
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="docs/brand/ubot-environment-light.svg">
    <source media="(prefers-color-scheme: light)" srcset="docs/brand/ubot-environment-dark.svg">
    <img alt="uBot Environment" src="docs/brand/ubot-environment-dark.svg" height="100">
  </picture>
</p>
<h1 align="center">uBot Environment</h1>

[简体中文](README.zh-CN.md)

Shared wind for robotics simulation in Unreal Engine 5.5. This plugin depends on uBotCore 0.1.4 or later in the 0.1 series. It has no dependency on uBotAerial.

## Wind

Enable **UBotEnvironment** in the project and place **uBot Wind** in a level. Its settings provide:

- Constant wind velocity on UE world axes, in m/s: the direction the air moves.
- Gust standard deviation per world axis, in m/s. Zero disables gusts.
- Gust correlation time (default 2 s, minimum 0.05 s) and a random seed.

Wind is uniform in space and varies with simulation time. The same settings, seed and time produce the same gust velocity. Several enabled wind actors add together. Actor position and rotation do not change the wind; the editor arrow shows the configured constant velocity.

Use **Set Wind Settings** and **Set Wind Enabled** to change an actor at run time. **Get uBot Wind Subsystem → Set Scripted Wind** adds one scripted source to the actors; pass `Enabled = false` to clear that source. **Get Wind Velocity At** returns the current velocity at a location in world centimeters, in m/s, using the uBot simulation clock. All sources are initially calm.

`UUBotWindSubsystem` implements uBotCore's `IUBotWindProvider` and registers with the current world's `UUBotWindQuerySubsystem`. Consumers depend only on uBotCore and query that subsystem at their own simulation time. Without a wind provider, it returns zero. Each world has its own sources.

The field does not model local wind zones, spatial turbulence, terrain or building effects, or rotor wakes. This plugin supplies wind velocity; consumers implement their own physical response.

## Migrating wind from uBotAerial

The wind implementation previously shipped in uBotAerial. Its settings, defaults, seeded gust calculation, source addition and Blueprint function signatures are preserved here.

Enable **UBotEnvironment before loading existing wind Blueprints or levels**. The plugin includes exact class redirects for `UBotWind` and `UBotWindSubsystem`, and a struct redirect for `UBotWindSettings`, from `/Script/UBotAerial` to `/Script/UBotEnvironment`. These redirects preserve references in existing assets, including wind actor properties, subsystem nodes and wind settings pins. Keep this plugin enabled for those assets; uBotAerial alone does not supply the wind field.

Native C++ users of `AUBotWind`, `UUBotWindSubsystem`, `FUBotWindSettings` or `UBot::Wind` must add `UBotEnvironment` to their module dependencies and rebuild. Include `UBotEnvironmentTypes.h` for `FUBotWindSettings`; the wind actor and subsystem retain their header names. General wind consumers should use the uBotCore query interface instead.

## Tests

Run the `UBotEnvironment.Wind` automation prefix in a host project with this plugin enabled. It covers deterministic gusts, source addition and run-time changes, routing through the uBotCore contract, and isolation between worlds.

## License

Apache-2.0. See [LICENSE](LICENSE) and [NOTICE](NOTICE).
