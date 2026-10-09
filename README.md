# AdminTools

Server-side admin chat commands for DayZ (servermod, no client download, no Community Framework needed).
Runs on **1.29 stable and 1.30** from the same PBO (tested on 1.29.163709 and 1.30.164014.27 with a real client).

## Install

1. Build/sign: `dayz-mod-pack.sh build`, copy `build/@AdminTools` to the server, put `keys/AdminTools.bikey` into the server's `keys/`.
2. Start with `-servermod=@AdminTools`.
3. First start writes `profiles/AdminTools/config.json`. Put your player ID into `adminUids`
   (type `/myid` in chat, or read it from the `.ADM` log: `Player "name"(id=...)`) and send `/reload`.
   Your own ID is not yet an admin on a fresh install, so on the first run edit the file and restart once.
4. **1.30:** `-profiles=` must be a folder inside the server directory (symlink is fine). The mod reads and writes
   `$profile:` files directly (no `FindFile`), so nothing else is needed.

`examples/config.json` is a ready-made config (spawn groups and Chernarus teleport locations from AdminTools 1.x).

## Commands

Type them in chat. Only IDs in `adminUids` are obeyed (except `/myid`). `/help [command]` lists them in game;
partial player/location/group names work (`/tp chern`), ambiguous ones are listed back to you.

| Command | What it does |
|---|---|
| `/help [cmd]` | list commands / details |
| `/myid` | your player ID (everybody) |
| `/players` | online players with distance |
| `/pos [player]` | position (`/tp x z` format) |
| `/tp <location \| player \| x z>` | teleport (alias `/teleport`) |
| `/goto <player>`, `/bring <player>` | teleport to / fetch a player (old: `/teleport_me_to`, `/teleport_to_me`) |
| `/back` | return to the position before the last teleport |
| `/locations [filter]` | list configured locations |
| `/heal [player]` | health, blood, food/water, diseases, bleeding, legs |
| `/fixgear [player]` | repair inventory (old: `/fix_gear`) |
| `/kill [player]`, `/kick <player> [reason]` | |
| `/godmode [0\|1]`, `/invisible [0\|1]` | toggles, no argument = toggle |
| `/ammo [0\|1]`, `/stamina [0\|1]` | unlimited ammo (magazine weapons) / stamina |
| `/spawn <class> [count]` | spawn items at your feet (class is checked first) |
| `/spawngroup [group]` | spawn a configured group, no argument lists them |
| `/car [type]` | spawn a fully equipped vehicle from `vehicles` |
| `/repair` | repair + refuel the vehicle you sit in |
| `/time [HH[:MM] \| day \| night]` | show/set time |
| `/reload` | reload config.json |

## Config (`profiles/AdminTools/config.json`)

`adminUids`, `spawnGroups`, `teleportLocations` (as in 1.x), plus `vehicles` (class names for `/car`),
`maxSpawnCount`, `auditLog` (write `profiles/AdminTools/audit.log`), `verbose` (extra script-log output),
`notifyTargets` (tell a healed/brought player; default off).

## Privacy: what other players can see

- **Replies, errors and help go only to the admin** (`ChatMP` to that player). Teleported/healed players are
  **not** told unless `notifyTargets` is on. Who did what is written to the audit log, never to chat.
- **The command line itself is the limit.** DayZ's server relays every chat line natively before/while scripts
  see it, and a server-only mod has no hook to stop that (verified: the sender's own client gets the echo of
  `/heal` as a normal chat message). Players in range of the channel you typed in can read the command,
  not the reply. Options: type commands while alone / in direct chat away from players, or add a small client
  mod that sends commands by RPC instead of chat (not part of this mod).
- Chat only carries the display name, so the mod refuses to act when two online players share a name
  (otherwise someone could copy an admin's name and run commands as them).

## Layout

`src/AdminTools/Scripts/4_World/AdminTools/` (config, commands, helpers, `PlayerBase` flags),
`5_Mission/MissionServer.c` (chat hook). Add a command: one `ATCommandInfo` line in `ATCommands.GetCommands()`
(this is also the help text), one `case` in `Run()`, one handler.

## Known gaps

`/ammo` does not refill weapons with an internal magazine (Mosin, shotguns). `/repair` and `notifyTargets`
were not exercised in the real-client run (no second player / vehicle seat).
