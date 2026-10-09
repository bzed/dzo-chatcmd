# dzo_chatcmd

Admin commands for DayZ, typed into the normal in-game chat: teleport, heal, spawn items and vehicles,
god mode, broadcast and more.

**Servermod only.** Players download nothing, the server stays vanilla for them, and the mod will never get a
client part. No Community Framework or other dependency. Works on **DayZ 1.29 and 1.30** from the same PBO.

Part of the dzo (DayZ Server Operator) mod collection. Renamed and rewritten from *AdminTools* 1.x.

Source code and issue tracker: <https://github.com/bzed/dzo-chatcmd>

Copyright (C) 2026 Bernd Zeimetz <bernd@bzed.de>. Licensed AGPL-3.0-or-later, see [`LICENSE`](LICENSE).

## Contents

- [Installation](#installation)
- [Commands](#commands)
- [Configuration](#configuration)
- [Files the mod writes](#files-the-mod-writes)
- [Privacy and security](#privacy-and-security)
- [Limits](#limits)
- [Building, signing, publishing](#building-signing-publishing)
- [Development and testing](#development-and-testing)
- [Upgrading from AdminTools](#upgrading-from-admintools)
- [License](#license)

## Installation

1. Put the mod (`@dzo_chatcmd`) into the server directory.
2. Start the server with it as a **server mod**: `-servermod=@dzo_chatcmd -profiles=profiles`
   (`-servermod`, not `-mod`: clients never see it).
3. If the server verifies signatures, copy `keys/dzo_chatcmd.bikey` into the server's `keys/` folder.
4. Start the server once. It writes `profiles/dzo_chatcmd/config.json` with defaults.
5. Get your player ID: type `/myid` in chat (the answer goes only to you), or read it from the newest `.ADM`
   log: `Player "name"(id=...)`. Put it into `adminUids` and restart. Later config edits are applied with `/reload`.

**DayZ 1.30:** the profiles folder (`-profiles=`) must be inside the server executable's folder (a symlink there is
fine), like for every server on 1.30. The mod reads and writes `$profile:` paths directly and does not use
`FindFile`, so no Community Framework or workaround is needed.

Without any entry in `adminUids` all commands except `/myid` are disabled; the log says so on startup.

`examples/config.json` is a complete example (Chernarus teleport locations, spawn groups, vehicle names, all
switches) to copy to `profiles/dzo_chatcmd/config.json`. Replace the placeholder ID in it with yours.

## Commands

Type them into chat with a leading `/`. Only player IDs listed in `adminUids` are obeyed; everybody else's
lines are ignored (`/myid` works for everybody). `/help` lists the commands in game, `/help <command>` shows
usage and aliases.

**Name matching.** Players, teleport locations, spawn groups and vehicles are matched case-insensitively. An exact
name wins; otherwise a part of a name is enough if it is unique (`/goto joh`). If several match, they are listed
back to you and nothing happens.

| Command | Aliases | What it does |
|---|---|---|
| `/help [command]` | `/?`, `/commands` | list commands / details of one |
| `/myid` | | your player ID (everybody may use it) |
| `/players` | `/who`, `/list` | online players with distance to you |
| `/nearby` | | players within 1000 m, nearest first, at most 10 (name and distance) |
| `/pos [player]` | `/where` | position as `x z (h height)`, the format `/tp x z` takes |
| `/tp <location \| player \| x z>` | `/teleport` | teleport yourself; locations are checked before players |
| `/goto <player>` | `/teleport_me_to` | teleport to a player |
| `/bring <player>` | `/teleport_to_me` | teleport a player to you |
| `/back` | | return to the position before your last teleport |
| `/locations [filter]` | `/locs` | list the configured teleport locations |
| `/heal [player]` | | full heal: health, blood, food, water, diseases, bleeding, broken legs, unconsciousness |
| `/fixgear [player]` | `/fix_gear` | repair every item in the inventory |
| `/kill [player]` | | kill a player (default: yourself; god mode is switched off first) |
| `/kick <player> [reason]` | | disconnect a player (not yourself) |
| `/godmode [0\|1]` | `/god` | damage immunity; no argument toggles |
| `/ammo [0\|1]` | `/unlimited_ammo` | refill the magazine of the weapon in hands |
| `/stamina [0\|1]` | `/unlimited_stamina` | unlimited stamina |
| `/spawn <classname> [count]` | | spawn items at your feet; the class is checked first, `count` is capped by `maxSpawnCount` |
| `/spawngroup [group]` | `/group`, `/groups` | spawn a configured group; without argument list the groups |
| `/car <name> [color]` | | spawn a fully equipped vehicle (see below); without argument list the names |
| `/repair` | | repair and refuel the vehicle you are sitting in |
| `/time [HH[:MM] \| day \| night]` | | show or set the server time |
| `/broadcast <message>` | `/bc`, `/say` | send `[Broadcast] <message>` to **all** players |
| `/reload` | | reload `config.json` |

Teleporting refuses players who sit in a vehicle. `/godmode`, `/ammo` and `/stamina` last until the character
is replaced (death, respawn, relog); `godModeOnLogin` can re-enable god mode automatically.

### Vehicles and colors

`/car ada` spawns the default version, `/car ada blue` a blue one. A vehicle is created with parts, fluids and
battery (the engine's debug spawn). The default names (`vehicleAliases`):

| Name | Class |
|---|---|
| `ada` | `OffroadHatchback` |
| `gunter` | `Hatchback_02` |
| `olga` | `CivilianSedan` |
| `sarka` | `Sedan_02` |
| `truck` | `Truck_01_Covered` |
| `humvee` | `Offroad_02` |
| `boat` | `Boat_01` (vanilla has no `Rubber_Boat`; the boat only exists in colors, plain `/car boat` takes the first one) |

A color `<c>` selects the class `<class>_<C>` (for example `Hatchback_02_Black`). The colors the mod looks for are
Black, Blue, White, Red, Grey, Wine, Orange, Green, Yellow and Camo. If the vehicle has no such variant, the
variants that exist in your game version are listed (`humvee only comes in the default color` when there are
none). Plain class names from `vehicles` work as well (`/car Sedan_02_Grey`).

## Configuration

`profiles/dzo_chatcmd/config.json`. Missing keys keep their defaults; a file that cannot be parsed is reported in the
script log and the previous settings stay active (it never falls back to "everybody is admin").
Booleans may be `true`/`false`.

| Key | Default | Meaning |
|---|---|---|
| `adminUids` | `[]` | player IDs allowed to use the commands |
| `spawnGroups` | two examples | map: group name -> list of class names (repeat a name to spawn several) |
| `teleportLocations` | `{}` | map: name -> `[x, y, z]`; y `0` means "on the ground" (oil rigs and the like use a real height) |
| `vehicles` | six vanilla cars | class names selectable by `/car` |
| `vehicleAliases` | see above | map: short name -> base class |
| `maxSpawnCount` | `50` | upper limit for the count of `/spawn` |
| `godModeOnLogin` | `false` | admins get god mode whenever they connect or respawn |
| `notifyTargets` | `false` | tell a player when an admin healed, repaired or teleported them |
| `auditLog` | `true` | write `audit.log` |
| `verbose` | `false` | extra lines in the script log |

## Files the mod writes

| File | Content |
|---|---|
| `profiles/dzo_chatcmd/config.json` | the configuration (created with defaults on first start) |
| `profiles/dzo_chatcmd/audit.log` | one line per admin action: time, name, player ID, what was done |
| script log | `[dzo_chatcmd]` lines: config summary, problems, and the same audit lines |

## Privacy and security

- **Answers are private.** Replies, errors and help go only to the admin that typed the command (a direct message
  to that player, never to a channel). Targets are not informed unless `notifyTargets` is on. The only public
  output is `/broadcast`, by design.
- **The command line itself is not private.** DayZ's server relays every chat line to the other players on its
  own, and a server-only mod has no hook to stop that (verified: the sender's own client receives its command as a
  normal chat line). Whoever is in range of the channel you typed in can read the command, but not the answer.
  Type commands where nobody listens. A client-side mod that sends commands by RPC would avoid this; it is not
  part of this mod, because the mod must stay server-only.
- **Sender identification.** The chat event carries only the display name. If two online players share a name the
  command is refused and logged, so a player cannot copy an admin's name to run commands.
- **Accountability.** Every action is written to the audit log with the admin's name and ID.
- Nothing is sent from or to the network except chat messages to players in the server.

## Limits

- **No `/invisible`.** `SetInvisible` and similar calls are local to the machine that sets them: tested with a
  real client, setting them on the server did not hide a character or a zombie from the client, and even
  client-side they did not hide a player body. Hiding a player from others therefore needs a client mod, which
  this mod deliberately never requires.
- `/ammo` refills magazines; weapons with an internal magazine (Mosin, shotguns) are not refilled.
- `/repair` and `notifyTargets` were not exercised in the real-client tests (they need a vehicle seat or a second
  player).
- Long lists (players, locations, groups) are sent as several chat lines.

## Building, signing, publishing

Needs the [bzed fork of armake2](https://github.com/bzed/armake2) (upstream does not sign `.c` files) and, for
Workshop upload, the [bzed fork of steam-workshop-uploader](https://github.com/bzed/steam-workshop-uploader).

```sh
dayz-mod-pack.sh keygen          # once: secrets/dzo_chatcmd.biprivatekey (git-ignored, back it up) + keys/dzo_chatcmd.bikey
dayz-mod-pack.sh build           # -> build/@dzo_chatcmd (addons, keys, mod.cpp, meta.cpp)
dayz-mod-pack.sh check           # no private key in git/upload folder, signatures verify
dayz-mod-pack.sh publish-hint    # prints the workshop commands
```

Repository layout:

| Path | Purpose |
|---|---|
| `src/dzo_chatcmd/` | the addon (becomes `addons/dzo_chatcmd.pbo`) |
| `mod.cpp` | mod metadata shown by the launcher (optional, kept for now) |
| `keys/dzo_chatcmd.bikey` | public key, ships with the mod |
| `workshop/description.txt` | Steam Workshop description (Steam BBCode) |
| `workshop/preview.png` | Workshop preview image, 1274x716 |
| `workshop/preview-square.png` | square preview, 512x512 |
| `examples/config.json` | full example configuration |
| `.dayzmod` | tool config: mod name and Workshop tag (`Server`) |

### Workshop upload

Uploading is done by hand with the Steam client running and logged in as an account that owns DayZ. The first
upload creates the item (keep it private until checked):

```sh
workshop create --app-id 221100 --content build/@dzo_chatcmd --title "dzo_chatcmd" \
    --description "$(cat workshop/description.txt)" --preview workshop/preview.png \
    -t Server --ignore-file .workshopignore --glob '!*.biprivatekey' \
    --visibility private -m "first upload"
cp build/@dzo_chatcmd/workshop.toml workshop.toml && git add workshop.toml   # the item id is not secret
```

Check the item in Steam, then make it public. Later releases:

```sh
dayz-mod-pack.sh build
workshop --no-prompt update --content build/@dzo_chatcmd --description "$(cat workshop/description.txt)" \
    --preview workshop/preview.png --ignore-file .workshopignore --glob '!*.biprivatekey' -m "changelog"
```

`mod.cpp` is optional (DayZ only needs the `meta.cpp` that the uploader fork writes, with item id and fresh
timestamp, into the content folder on every upload); it is kept for now. The only Workshop tag is `Server` (type tag for server mods; there is no `servermod` tag, and `Mod` is not combined with it).

## Development and testing

Source layout: `src/dzo_chatcmd/Scripts/4_World/dzo_chatcmd/` holds the config (`DZOCCConfig.c`), the log
(`DZOCCLog.c`), helpers (`DZOCCUtil.c`), the per-command context (`DZOCCContext.c`), the commands
(`DZOCCCommands.c`) and the per-player flags (`PlayerBase.c`, server-side only, nothing is synchronized).
`5_Mission/MissionServer.c` receives the chat events and the login hook.

To add a command: one `DZOCCCommandInfo` line in `DZOCCCommands.GetCommands()` (it is also the help text), one
`case` in `Run()`, one handler. Answer through `DZOCCContext.Reply()` only; never send text to other players.

Compatibility: no `FindFile`, no `$storage:`, forward-slash paths only, no overrides whose signature changed in
1.30. If a version-specific API is ever needed, put the old code under `#ifdef DAYZ_1_29` and the new code in
`#else`.

Testing: the mod was run on a 1.29.163709 and a 1.30.164014.27 dedicated server with a headless real client that
sent the commands through chat; replies, audit log and script logs were checked. To repeat this, build the mod,
start the server with `-servermod=@dzo_chatcmd`, join with a client whose player ID is in `adminUids` and try
the commands. Enforce Script pitfalls found on the way: `debug` is a reserved word (not usable as a field name),
and `foreach` over a function's return value throws a null pointer (assign it to a local first).

## Upgrading from AdminTools

This mod is the renamed and rewritten AdminTools. On the first start, `profiles/AdminTools/config.json` is
copied to `profiles/dzo_chatcmd/config.json` if the new file does not exist yet; the old files are left alone.
Replace `-servermod=@AdminTools` by `-servermod=@dzo_chatcmd` and `AdminTools.bikey` by `dzo_chatcmd.bikey`
(the key is new). The old chat commands keep working; the config keys are unchanged, new ones were added. `/invisible`
is gone (see [Limits](#limits)).

## License

Copyright (C) 2026 Bernd Zeimetz <bernd@bzed.de>, <https://github.com/bzed/dzo-chatcmd>

This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero
General Public License as published by the Free Software Foundation, either version 3 of the License, or (at
your option) any later version. It is distributed WITHOUT ANY WARRANTY. The full text is in [`LICENSE`](LICENSE).
Every source file carries the notice in its header.

*This mod is not affiliated or authorized by Bohemia Interactive a.s. Bohemia Interactive, ARMA, DAYZ and all
associated logos and designs are trademarks or registered trademarks of Bohemia Interactive a.s.*
