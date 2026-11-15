# Changelog

All notable changes to the KrtuworksSteam plugin for Unreal Engine. Versions follow semantic versioning; a breaking change is marked **Breaking**.

## Unreleased

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
