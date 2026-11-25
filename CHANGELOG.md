# Changelog

All notable changes to the KrtuworksSteam plugin for Unreal Engine. Versions follow semantic versioning; a breaking change is marked **Breaking**.

## Unreleased

## 1.0.6 - 2026-11-25

### Changed
- No change in this package: the SDK packages share one version, and this one fixes the Krtuworks plugin.

## 1.0.5 - 2026-11-25

### Changed
- No change in this package: the SDK packages share one version, and this one adds the ready-made screens to the Krtuworks plugin.

## 1.0.4 - 2026-11-22

### Changed
- No change in this package: the SDK packages share one version, and this one fixes the Krtuworks plugin.

## 1.0.3 - 2026-11-22

### Changed
- No change in this package: the SDK packages share one version, and this one fixes the Krtuworks plugin.

## 1.0.2 - 2026-11-15

### Fixed
- The plugin did not link in modular builds (the editor): the Krtuworks plugin did not export the Steam half it builds on.

### Changed
- Unreal Engine 5.4 or later: UE 5.3 bundles a Steamworks SDK older than 1.57, and the build there now stops with one message that names the SDK version it needs.

## 1.0.1 - 2026-11-15

### Changed
- No change in this package: the SDK packages share one version, and 1.0.1 fixes the Unity package.

## 1.0.0 - 2026-11-15

First release. Unreal Engine 5.3 or later, Steamworks SDK 1.57 or later, Windows, macOS and Linux.

### Added
- The Steam account's player through a Steam web API ticket for `krtuworks-id`, and each session's licence check.
- Achievements both ways with Steam, including clearing the ones support took back.
- Steam stats mirror the server (`GetStat`, `SetStat`, `StoreStats` on `SteamUserStats`): a stat with a Steam API name is set to the server's value, and the player's existing Steam value can be read in.
- The player's own row of a Steam leaderboard (`FindLeaderboard`, `DownloadLeaderboardEntriesForUsers` for the local user), read once to bring a score made on Steam into Krtuworks. Nothing is ever uploaded to a Steam leaderboard from the game.
- Steam Cloud through `SteamRemoteStorage` (the account's and the app's cloud setting, file read and write with their time, the quota) for the Krtuworks plugin's mirror of the marked slots.
- Rich presence through `SteamFriends` (`SetRichPresence`, `ClearRichPresence`) for the Krtuworks plugin's presence states: the state's params, the group keys and `steam_display` last. Without `SteamFriends()` every write is refused and nothing is left showing.
- The Steam client's own ownership answer for a DLC (`SteamApps()->BIsSubscribedApp` for the DLC's AppID), which the Krtuworks plugin trusts when the server could not check the DLC in this launch. Without `SteamApps()` it cannot tell.
