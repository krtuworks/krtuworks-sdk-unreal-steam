# Krtuworks SDK: Steam (Unreal Engine): instructions for coding agents

KrtuworksSteam adds Steam to the Krtuworks Unreal plugin: the Steam account's player (a Steam web API ticket for `krtuworks-id`), each session's licence check, achievements both ways, a stats mirror, the player's own Steam leaderboard row, Steam Cloud, rich presence and DLC ownership. It runs on Windows, macOS and Linux (Steam Deck included).

## Required steps
1. Set up the Krtuworks plugin first (its `AGENTS.md`).
2. Put this plugin, in the same version as the Krtuworks plugin, in the project's `Plugins/KrtuworksSteam` folder and enable it. It needs Unreal Engine 5.4 or later, the engine's Online Subsystem Steam plugin and the Steamworks SDK 1.57 or later (UE 5.3 bundles an older one); configure Online Subsystem Steam for the game's Steam app as usual.
3. Nothing to call: the plugin registers its Steam services with the Krtuworks subsystem when the game starts. In an app that is not a game (`type` in `krtuworks-config.json`), Steam is not used.

## Rules
- Do not request a Steam ticket for Krtuworks yourself and do not pass one along: the plugin asks Steam for a new ticket for each sign-in and each session.
- Achievements, stats and leaderboards need their Steam API name set on the partner console. The SDK unlocks achievements on Steam and clears there the ones support took back.
- Steam stats mirror the server: the plugin sets the Steam stat to the server's value. Never write Steam stats yourself; a stat without a Steam API name is not mirrored.
- DLC ownership: the Krtuworks plugin's `ownsDlc(key)` asks Steam (`BIsSubscribedApp` for the DLC's AppID) only when the server could not check in this launch; do not ask Steam for DLC yourself.
- Steam Cloud: the SDK writes the marked slots as `krtuworks-<slot>.bin`; leave them out of Steam Auto-Cloud and do not touch them.
- While Steam sign-in is not yet open for the game (Krtuworks answers `steam_not_configured`), the SDK falls back to a device guest by itself. Do not treat it as an error.

## Verify
Run the Krtuworks `Validate` node in a development build started from Steam. The platform check must pass (STEAM with its services), then the TEST round as in the Krtuworks plugin.
