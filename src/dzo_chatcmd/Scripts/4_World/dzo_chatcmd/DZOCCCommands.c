// dzo_chatcmd - server-side admin chat commands for DayZ
// Copyright (C) 2026 Bernd Zeimetz
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published
// by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
// SPDX-License-Identifier: AGPL-3.0-or-later

class DZOCCCommandInfo
{
	string name;
	string usage;
	string description;
	ref array<string> aliases = new array<string>;

	void DZOCCCommandInfo(string n, string u, string d, string a = "")
	{
		name = n;
		usage = u;
		description = d;
		if (a.Length() > 0)
			a.Split(",", aliases);
	}
}

// Chat commands. Entry point: DZOCCCommands.Handle(). Replies go to the sending admin only.
class DZOCCCommands
{
	protected static ref array<ref DZOCCCommandInfo> s_Commands;

	static array<ref DZOCCCommandInfo> GetCommands()
	{
		if (!s_Commands)
		{
			s_Commands = new array<ref DZOCCCommandInfo>;
			s_Commands.Insert(new DZOCCCommandInfo("help", "/help [command]", "List commands, or show details of one", "?,commands"));
			s_Commands.Insert(new DZOCCCommandInfo("myid", "/myid", "Show your player ID (works for everybody)"));
			s_Commands.Insert(new DZOCCCommandInfo("players", "/players", "List online players with distance", "who,list"));
			s_Commands.Insert(new DZOCCCommandInfo("pos", "/pos [player]", "Show position (usable with /tp x z)", "where"));
			s_Commands.Insert(new DZOCCCommandInfo("tp", "/tp <location | player | x z>", "Teleport yourself", "teleport"));
			s_Commands.Insert(new DZOCCCommandInfo("goto", "/goto <player>", "Teleport to a player", "teleport_me_to"));
			s_Commands.Insert(new DZOCCCommandInfo("bring", "/bring <player>", "Teleport a player to you", "teleport_to_me"));
			s_Commands.Insert(new DZOCCCommandInfo("back", "/back", "Return to where you were before the last teleport"));
			s_Commands.Insert(new DZOCCCommandInfo("nearby", "/nearby", "Players within 1000 m, nearest first (max 10)"));
			s_Commands.Insert(new DZOCCCommandInfo("broadcast", "/broadcast <message>", "Send a message to ALL players", "bc,say"));
			s_Commands.Insert(new DZOCCCommandInfo("locations", "/locations [filter]", "List teleport locations", "locs"));
			s_Commands.Insert(new DZOCCCommandInfo("heal", "/heal [player]", "Full heal: health, blood, food, water, diseases, bleeding, legs"));
			s_Commands.Insert(new DZOCCCommandInfo("fixgear", "/fixgear [player]", "Repair all items in the inventory", "fix_gear"));
			s_Commands.Insert(new DZOCCCommandInfo("kill", "/kill [player]", "Kill a player (default: yourself)"));
			s_Commands.Insert(new DZOCCCommandInfo("kick", "/kick <player> [reason]", "Disconnect a player"));
			s_Commands.Insert(new DZOCCCommandInfo("godmode", "/godmode [0|1]", "Damage immunity (no argument: toggle)", "god"));
			s_Commands.Insert(new DZOCCCommandInfo("ammo", "/ammo [0|1]", "Unlimited ammo for magazine weapons", "unlimited_ammo"));
			s_Commands.Insert(new DZOCCCommandInfo("stamina", "/stamina [0|1]", "Unlimited stamina", "unlimited_stamina"));
			s_Commands.Insert(new DZOCCCommandInfo("spawn", "/spawn <classname> [count]", "Spawn item(s) at your feet"));
			s_Commands.Insert(new DZOCCCommandInfo("spawngroup", "/spawngroup [group]", "Spawn a configured item group (no argument: list)", "group,groups"));
			s_Commands.Insert(new DZOCCCommandInfo("car", "/car <name> [color]", "Spawn a fully equipped vehicle, e.g. /car ada black (no argument: list)"));
			s_Commands.Insert(new DZOCCCommandInfo("repair", "/repair", "Repair and refuel the vehicle you are in"));
			s_Commands.Insert(new DZOCCCommandInfo("time", "/time [HH[:MM] | day | night]", "Show or set the server time"));
			s_Commands.Insert(new DZOCCCommandInfo("reload", "/reload", "Reload config.json"));
		}
		return s_Commands;
	}

	protected static DZOCCCommandInfo FindCommand(string name)
	{
		string lname = name;
		lname.ToLower();
		array<ref DZOCCCommandInfo> all = GetCommands();
		for (int i = 0; i < all.Count(); i++)
		{
			DZOCCCommandInfo info = all[i];
			if (info.name == lname || info.aliases.Find(lname) != -1)
				return info;
		}
		return null;
	}

	// Called for every chat line the server sees. Returns true if it was a command that was executed.
	static bool Handle(PlayerBase sender, string message)
	{
		message.TrimInPlace();
		if (message.Length() < 2 || message.Get(0) != "/")
			return false;

		array<string> parts = new array<string>;
		array<string> raw = new array<string>;
		string body = message.Substring(1, message.Length() - 1);
		body.Split(" ", raw);
		foreach (string part : raw)
		{
			if (part.Length() > 0)
				parts.Insert(part);
		}
		if (parts.Count() == 0)
			return false;

		DZOCCCommandInfo info = FindCommand(parts[0]);
		if (!info)
			return false;

		DZOCCContext ctx = new DZOCCContext();
		ctx.admin = sender;
		ctx.command = info.name;
		for (int i = 1; i < parts.Count(); i++)
			ctx.args.Insert(parts[i]);

		// the only command everybody may use
		if (info.name == "myid")
		{
			ctx.Reply("Your player ID: " + sender.GetIdentity().GetId());
			return true;
		}

		if (!DZOCCConfig.IsAdmin(sender.GetIdentity()))
		{
			DZOCCLog.Debug("ignored /" + info.name + " from non-admin " + sender.GetIdentity().GetName());
			return false;
		}

		DZOCCLog.Debug("/" + info.name + " " + ctx.Rest(0) + " from " + sender.GetIdentity().GetName());
		Run(info, ctx);
		return true;
	}

	protected static void Run(DZOCCCommandInfo info, DZOCCContext ctx)
	{
		switch (info.name)
		{
			case "help":      Help(ctx); break;
			case "players":   Players(ctx); break;
			case "pos":       Pos(ctx); break;
			case "tp":        Teleport(ctx); break;
			case "goto":      GotoPlayer(ctx); break;
			case "bring":     Bring(ctx); break;
			case "back":      Back(ctx); break;
			case "nearby":    Nearby(ctx); break;
			case "broadcast": Broadcast(ctx); break;
			case "locations": Locations(ctx); break;
			case "heal":      Heal(ctx); break;
			case "fixgear":   FixGear(ctx); break;
			case "kill":      Kill(ctx); break;
			case "kick":      Kick(ctx); break;
			case "godmode":   God(ctx); break;
			case "ammo":      UnlimitedAmmo(ctx); break;
			case "stamina":   UnlimitedStamina(ctx); break;
			case "spawn":     Spawn(ctx); break;
			case "spawngroup": SpawnGroup(ctx); break;
			case "car":       SpawnCar(ctx); break;
			case "repair":    Repair(ctx); break;
			case "time":      ShowTime(ctx); break;
			case "reload":    Reload(ctx); break;
		}
	}

	protected static void Usage(DZOCCContext ctx)
	{
		DZOCCCommandInfo info = FindCommand(ctx.command);
		if (info)
			ctx.Reply("Usage: " + info.usage);
	}

	// 0 = off, 1 = on, no argument = toggle. Returns -1 on bad input.
	protected static int ParseSwitch(DZOCCContext ctx, bool current)
	{
		string arg = ctx.Arg(0);
		arg.ToLower();
		if (arg.Length() == 0)
			return !current;
		if (arg == "1" || arg == "on")
			return 1;
		if (arg == "0" || arg == "off")
			return 0;
		Usage(ctx);
		return -1;
	}

	protected static string OnOff(bool value)
	{
		if (value)
			return "enabled";
		return "disabled";
	}

	// Teleport that remembers where the player came from; refuses while in a vehicle.
	protected static bool MoveTo(DZOCCContext ctx, PlayerBase who, vector pos)
	{
		if (who.IsInTransport())
		{
			ctx.Reply(who.GetIdentity().GetName() + " is in a vehicle, cannot teleport.");
			return false;
		}
		who.m_DZOCC_LastPos = who.GetPosition();
		who.m_DZOCC_HasLastPos = true;
		who.SetPosition(pos);
		return true;
	}

	// ---- information ----

	protected static void Help(DZOCCContext ctx)
	{
		if (ctx.args.Count() > 0)
		{
			DZOCCCommandInfo info = FindCommand(ctx.Arg(0));
			if (!info)
			{
				ctx.Reply("No such command: " + ctx.Arg(0));
				return;
			}
			ctx.Reply(info.usage + " - " + info.description);
			if (info.aliases.Count() > 0)
				ctx.Reply("Aliases: /" + DZOCCUtil.Join(", /", info.aliases));
			return;
		}

		ctx.Reply("Commands (admin only, answers are private; /help <command> for details):");
		array<ref DZOCCCommandInfo> all = GetCommands();
		for (int i = 0; i < all.Count(); i++)
			ctx.Reply(all[i].usage + " - " + all[i].description);
	}

	protected static void Players(DZOCCContext ctx)
	{
		array<Man> men = new array<Man>;
		GetGame().GetPlayers(men);

		array<string> entries = new array<string>;
		foreach (Man man : men)
		{
			PlayerBase pb = PlayerBase.Cast(man);
			if (!pb || !pb.GetIdentity())
				continue;
			string entry = pb.GetIdentity().GetName();
			if (pb != ctx.admin)
				entry += " (" + Math.Round(vector.Distance(pb.GetPosition(), ctx.admin.GetPosition())) + " m)";
			entries.Insert(entry);
		}
		ctx.ReplyList(entries.Count().ToString() + " online: ", entries);
	}

	protected static void Nearby(DZOCCContext ctx)
	{
		float RANGE = 1000.0;
		int MAX_ENTRIES = 10;

		array<Man> men = new array<Man>;
		GetGame().GetPlayers(men);

		array<string> names = new array<string>;
		array<float> distances = new array<float>;
		foreach (Man man : men)
		{
			PlayerBase pb = PlayerBase.Cast(man);
			if (!pb || pb == ctx.admin || !pb.GetIdentity())
				continue;

			float dist = vector.Distance(pb.GetPosition(), ctx.admin.GetPosition());
			if (dist > RANGE)
				continue;

			// insertion sort, nearest first
			int at = 0;
			while (at < distances.Count() && distances[at] <= dist)
				at++;
			names.InsertAt(pb.GetIdentity().GetName(), at);
			distances.InsertAt(dist, at);
		}

		array<string> entries = new array<string>;
		for (int i = 0; i < names.Count() && i < MAX_ENTRIES; i++)
			entries.Insert(names[i] + " (" + Math.Round(distances[i]) + " m)");

		if (entries.Count() == 0)
		{
			ctx.Reply("Nobody within 1000 m.");
			return;
		}
		ctx.ReplyList(entries.Count().ToString() + " within 1000 m: ", entries);
	}

	protected static void Broadcast(DZOCCContext ctx)
	{
		string text = ctx.Rest(0);
		if (text.Length() == 0)
		{
			Usage(ctx);
			return;
		}

		array<Man> men = new array<Man>;
		GetGame().GetPlayers(men);
		int sent = 0;
		foreach (Man man : men)
		{
			PlayerBase pb = PlayerBase.Cast(man);
			if (pb && pb.GetIdentity())
			{
				pb.MessageImportant("[Broadcast] " + text);
				sent++;
			}
		}
		ctx.Reply("Broadcast sent to " + sent + " player(s).");
		ctx.Audit("broadcast: " + text);
	}

	protected static void Pos(DZOCCContext ctx)
	{
		PlayerBase target = ctx.admin;
		if (ctx.args.Count() > 0)
		{
			target = ctx.FindPlayer(ctx.Rest(0));
			if (!target)
				return;
		}
		ctx.Reply(target.GetIdentity().GetName() + ": " + DZOCCUtil.FormatPos(target.GetPosition()));
	}

	protected static void Locations(DZOCCContext ctx)
	{
		array<string> names = DZOCCConfig.Get().teleportLocations.GetKeyArray();
		if (names.Count() == 0)
		{
			ctx.Reply("No teleport locations configured.");
			return;
		}

		string filter = ctx.Rest(0);
		filter.ToLower();
		array<string> shown = new array<string>;
		foreach (string locName : names)
		{
			string lower = locName;
			lower.ToLower();
			if (filter.Length() == 0 || lower.Contains(filter))
				shown.Insert(locName);
		}
		shown.Sort();
		ctx.ReplyList(shown.Count().ToString() + " location(s): ", shown);
	}

	// ---- teleport ----

	protected static void Teleport(DZOCCContext ctx)
	{
		if (ctx.args.Count() == 0)
		{
			Usage(ctx);
			return;
		}

		// /tp x z
		if (ctx.args.Count() == 2 && DZOCCUtil.IsNumeric(ctx.Arg(0)) && DZOCCUtil.IsNumeric(ctx.Arg(1)))
		{
			vector coords = Vector(ctx.Arg(0).ToFloat(), 0, ctx.Arg(1).ToFloat());
			if (MoveTo(ctx, ctx.admin, DZOCCUtil.SnapToGround(coords)))
			{
				ctx.Reply("Teleported to " + DZOCCUtil.FormatPos(ctx.admin.GetPosition()));
				ctx.Audit("teleported to " + DZOCCUtil.FormatPos(ctx.admin.GetPosition()));
			}
			return;
		}

		string query = ctx.Rest(0);
		array<string> locationNames = DZOCCConfig.Get().teleportLocations.GetKeyArray();

		// locations first, then players
		array<int> locHits;
		DZOCCUtil.MatchIndices(locationNames, query, locHits);
		if (locHits.Count() > 0)
		{
			string locName = ctx.FindName(locationNames, query, "location");
			if (locName.Length() == 0)
				return;
			vector locPos = DZOCCConfig.Get().teleportLocations.Get(locName);
			if (MoveTo(ctx, ctx.admin, DZOCCUtil.SnapToGround(locPos)))
			{
				ctx.Reply("Teleported to " + locName);
				ctx.Audit("teleported to location " + locName);
			}
			return;
		}

		PlayerBase target = ctx.FindPlayer(query);
		if (!target)
			return;
		if (MoveTo(ctx, ctx.admin, target.GetPosition()))
		{
			ctx.Reply("Teleported to " + target.GetIdentity().GetName());
			ctx.Audit("teleported to player " + target.GetIdentity().GetName());
		}
	}

	protected static void GotoPlayer(DZOCCContext ctx)
	{
		if (ctx.args.Count() == 0)
		{
			Usage(ctx);
			return;
		}
		PlayerBase target = ctx.FindPlayer(ctx.Rest(0));
		if (!target)
			return;
		if (MoveTo(ctx, ctx.admin, target.GetPosition()))
		{
			ctx.Reply("Teleported to " + target.GetIdentity().GetName());
			ctx.Audit("teleported to player " + target.GetIdentity().GetName());
		}
	}

	protected static void Bring(DZOCCContext ctx)
	{
		if (ctx.args.Count() == 0)
		{
			Usage(ctx);
			return;
		}
		PlayerBase target = ctx.FindPlayer(ctx.Rest(0));
		if (!target)
			return;
		if (MoveTo(ctx, target, ctx.admin.GetPosition()))
		{
			ctx.Reply("Brought " + target.GetIdentity().GetName() + " to you.");
			ctx.NotifyTarget(target, "You were teleported.");
			ctx.Audit("brought player " + target.GetIdentity().GetName() + " to " + DZOCCUtil.FormatPos(ctx.admin.GetPosition()));
		}
	}

	protected static void Back(DZOCCContext ctx)
	{
		if (!ctx.admin.m_DZOCC_HasLastPos)
		{
			ctx.Reply("No previous position.");
			return;
		}
		vector previous = ctx.admin.m_DZOCC_LastPos;
		if (MoveTo(ctx, ctx.admin, previous))
		{
			ctx.Reply("Back at " + DZOCCUtil.FormatPos(previous));
			ctx.Audit("teleported back to " + DZOCCUtil.FormatPos(previous));
		}
	}

	// ---- players ----

	protected static void Heal(DZOCCContext ctx)
	{
		PlayerBase target = ctx.admin;
		if (ctx.args.Count() > 0)
		{
			target = ctx.FindPlayer(ctx.Rest(0));
			if (!target)
				return;
		}

		target.GetStomach().ClearContents();
		DamageSystem.ResetAllZones(target);
		target.GetModifiersManager().ResetAll();
		target.RemoveAllAgents();

		BleedingSourcesManagerServer bleeding = target.GetBleedingManagerServer();
		if (bleeding)
			bleeding.RemoveAllSources();

		target.SetHealth("", "", target.GetMaxHealth("", ""));
		target.SetHeatBufferDynamicMax(3.0);
		if (target.IsUnconscious())
			DayZPlayerSyncJunctures.SendPlayerUnconsciousness(target, false);

		target.GetStatWater().Set(target.GetStatWater().GetMax());
		target.GetStatEnergy().Set(target.GetStatEnergy().GetMax());
		target.GetStatHeatBuffer().Set(target.GetStatHeatBuffer().GetMax());
		target.SetBloodyHands(false);
		target.SetBrokenLegs(eBrokenLegs.NO_BROKEN_LEGS);

		ctx.Reply("Healed " + target.GetIdentity().GetName() + ".");
		ctx.NotifyTarget(target, "You were healed.");
		ctx.Audit("healed " + target.GetIdentity().GetName());
	}

	protected static void FixGear(DZOCCContext ctx)
	{
		PlayerBase target = ctx.admin;
		if (ctx.args.Count() > 0)
		{
			target = ctx.FindPlayer(ctx.Rest(0));
			if (!target)
				return;
		}
		target.FixAllInventoryItems();
		ctx.Reply("Repaired the gear of " + target.GetIdentity().GetName() + ".");
		ctx.NotifyTarget(target, "Your gear was repaired.");
		ctx.Audit("repaired gear of " + target.GetIdentity().GetName());
	}

	protected static void Kill(DZOCCContext ctx)
	{
		PlayerBase target = ctx.admin;
		if (ctx.args.Count() > 0)
		{
			target = ctx.FindPlayer(ctx.Rest(0));
			if (!target)
				return;
		}
		// god mode would swallow the damage
		target.DZOCC_SetGod(false);
		target.SetHealth("", "", 0);
		ctx.Reply("Killed " + target.GetIdentity().GetName() + ".");
		ctx.Audit("killed " + target.GetIdentity().GetName());
	}

	protected static void Kick(DZOCCContext ctx)
	{
		if (ctx.args.Count() == 0)
		{
			Usage(ctx);
			return;
		}
		PlayerBase target = ctx.FindPlayer(ctx.Arg(0));
		if (!target)
			return;
		if (target == ctx.admin)
		{
			ctx.Reply("You cannot kick yourself.");
			return;
		}
		string targetName = target.GetIdentity().GetName();
		string reason = ctx.Rest(1);
		ctx.Reply("Kicking " + targetName + ".");
		ctx.Audit("kicked " + targetName + " (" + target.GetIdentity().GetId() + ") reason: " + reason);
		GetGame().DisconnectPlayer(target.GetIdentity());
	}

	// ---- toggles ----

	protected static void God(DZOCCContext ctx)
	{
		int mode = ParseSwitch(ctx, ctx.admin.m_DZOCC_God);
		if (mode < 0)
			return;
		ctx.admin.DZOCC_SetGod(mode == 1);
		ctx.Reply("God mode " + OnOff(mode == 1) + ".");
		ctx.Audit("god mode " + OnOff(mode == 1));
	}

	protected static void UnlimitedAmmo(DZOCCContext ctx)
	{
		int mode = ParseSwitch(ctx, ctx.admin.m_DZOCC_UnlimitedAmmo);
		if (mode < 0)
			return;
		ctx.admin.m_DZOCC_UnlimitedAmmo = (mode == 1);
		ctx.Reply("Unlimited ammo " + OnOff(mode == 1) + ".");
		ctx.Audit("unlimited ammo " + OnOff(mode == 1));
	}

	protected static void UnlimitedStamina(DZOCCContext ctx)
	{
		int mode = ParseSwitch(ctx, ctx.admin.m_DZOCC_UnlimitedStamina);
		if (mode < 0)
			return;
		ctx.admin.m_DZOCC_UnlimitedStamina = (mode == 1);
		ctx.Reply("Unlimited stamina " + OnOff(mode == 1) + ".");
		ctx.Audit("unlimited stamina " + OnOff(mode == 1));
	}

	// ---- spawning ----

	protected static EntityAI SpawnAt(string className, vector pos)
	{
		return EntityAI.Cast(GetGame().CreateObjectEx(className, pos, ECE_PLACE_ON_SURFACE | ECE_INITAI | ECE_EQUIP_ATTACHMENTS));
	}

	protected static void Spawn(DZOCCContext ctx)
	{
		if (ctx.args.Count() < 1 || ctx.args.Count() > 2)
		{
			Usage(ctx);
			return;
		}

		string className = ctx.Arg(0);
		int count = 1;
		if (ctx.args.Count() == 2)
			count = ctx.Arg(1).ToInt();
		count = Math.Clamp(count, 1, DZOCCConfig.Get().maxSpawnCount);

		if (!DZOCCUtil.ClassExists(className))
		{
			ctx.Reply("Unknown classname: " + className);
			return;
		}

		int spawned = 0;
		for (int i = 0; i < count; i++)
		{
			if (SpawnAt(className, ctx.admin.GetPosition()))
				spawned++;
		}
		ctx.Reply("Spawned " + spawned + "x " + className);
		ctx.Audit("spawned " + spawned + "x " + className + " at " + DZOCCUtil.FormatPos(ctx.admin.GetPosition()));
	}

	protected static void SpawnGroup(DZOCCContext ctx)
	{
		map<string, ref array<string>> groups = DZOCCConfig.Get().spawnGroups;
		array<string> groupNames = groups.GetKeyArray();

		if (ctx.args.Count() == 0)
		{
			groupNames.Sort();
			ctx.ReplyList("Groups: ", groupNames);
			return;
		}

		string groupName = ctx.FindName(groupNames, ctx.Rest(0), "group");
		if (groupName.Length() == 0)
			return;

		int spawned = 0;
		array<string> failed = new array<string>;
		array<string> members = groups.Get(groupName);
		foreach (string className : members)
		{
			if (DZOCCUtil.ClassExists(className) && SpawnAt(className, ctx.admin.GetPosition()))
				spawned++;
			else
				failed.Insert(className);
		}

		ctx.Reply("Spawned " + spawned + " item(s) from '" + groupName + "'.");
		if (failed.Count() > 0)
			ctx.ReplyList("Could not spawn: ", failed);
		ctx.Audit("spawned group " + groupName + " (" + spawned + " items) at " + DZOCCUtil.FormatPos(ctx.admin.GetPosition()));
	}

	// Colors that exist as <base>_<Color> classes in this game version
	protected static void ColorsOf(string baseClass, array<string> found)
	{
		array<string> candidates = new array<string>;
		"Black,Blue,White,Red,Grey,Wine,Orange,Green,Yellow,Camo".Split(",", candidates);
		foreach (string candidate : candidates)
		{
			if (DZOCCUtil.ClassExists(baseClass + "_" + candidate))
				found.Insert(candidate);
		}
	}

	protected static void SpawnCar(DZOCCContext ctx)
	{
		DZOCCSettings cfg = DZOCCConfig.Get();
		map<string, string> aliases = cfg.vehicleAliases;

		// selectable names: aliases first, then plain classnames from "vehicles"
		array<string> names = aliases.GetKeyArray();
		names.Sort();
		foreach (string listed : cfg.vehicles)
		{
			if (names.Find(listed) == -1)
				names.Insert(listed);
		}

		if (ctx.args.Count() == 0)
		{
			ctx.ReplyList("Vehicles: ", names);
			ctx.Reply("Usage: /car <name> [color], e.g. /car ada black");
			return;
		}

		string choice = ctx.FindName(names, ctx.Arg(0), "vehicle");
		if (choice.Length() == 0)
			return;

		string baseClass = choice;
		if (aliases.Contains(choice))
			baseClass = aliases.Get(choice);

		string carType;
		string color = ctx.Arg(1);
		color.ToLower();
		if (color.Length() == 0 || color == "default")
		{
			carType = baseClass;
			if (!DZOCCUtil.ClassExists(carType))
			{
				// no plain version (boats): take the first color that exists
				array<string> any = new array<string>;
				ColorsOf(baseClass, any);
				if (any.Count() == 0)
				{
					ctx.Reply(baseClass + " does not exist in this game version.");
					return;
				}
				carType = baseClass + "_" + any[0];
			}
		}
		else
		{
			string first = color.Substring(0, 1);
			first.ToUpper();
			string cased = first + color.Substring(1, color.Length() - 1);
			carType = baseClass + "_" + cased;
			if (!DZOCCUtil.ClassExists(carType))
			{
				array<string> available = new array<string>;
				ColorsOf(baseClass, available);
				if (available.Count() == 0)
					ctx.Reply(choice + " only comes in the default color.");
				else
					ctx.ReplyList(choice + " colors: ", available);
				return;
			}
		}

		// a few meters in front of the admin
		vector pos = ctx.admin.GetPosition() + ctx.admin.GetDirection() * 5;
		EntityAI vehicle = SpawnAt(carType, pos);
		if (!vehicle)
		{
			ctx.Reply("Could not spawn " + carType);
			return;
		}
		vehicle.SetOrientation(ctx.admin.GetOrientation());
		vehicle.OnDebugSpawn();
		ctx.Reply("Spawned " + carType + " (parts, fluids and battery included).");
		ctx.Audit("spawned vehicle " + carType + " at " + DZOCCUtil.FormatPos(pos));
	}

	protected static void Repair(DZOCCContext ctx)
	{
		Transport vehicle = Transport.Cast(ctx.admin.GetParent());
		if (!vehicle)
		{
			ctx.Reply("You are not in a vehicle.");
			return;
		}

		array<string> zones = new array<string>;
		vehicle.GetDamageZones(zones);
		foreach (string zone : zones)
			vehicle.SetHealth01(zone, "", 1.0);
		vehicle.SetHealthMax("", "");

		array<EntityAI> parts = new array<EntityAI>;
		vehicle.GetInventory().EnumerateInventory(InventoryTraversalType.INORDER, parts);
		foreach (EntityAI part : parts)
			part.SetHealthMax("", "");

		CarScript car = CarScript.Cast(vehicle);
		if (car)
		{
			AddFluid(car, CarFluid.FUEL);
			AddFluid(car, CarFluid.COOLANT);
			AddFluid(car, CarFluid.OIL);
			AddFluid(car, CarFluid.BRAKE);
		}

		ctx.Reply("Vehicle repaired and refilled.");
		ctx.Audit("repaired vehicle " + vehicle.GetType());
	}

	protected static void AddFluid(Car car, CarFluid fluid)
	{
		float missing = car.GetFluidCapacity(fluid) * (1.0 - car.GetFluidFraction(fluid));
		if (missing > 0)
			car.Fill(fluid, missing);
	}

	// ---- world / admin ----

	protected static void ShowTime(DZOCCContext ctx)
	{
		int year, month, day, hour, minute;
		GetGame().GetWorld().GetDate(year, month, day, hour, minute);

		if (ctx.args.Count() == 0)
		{
			ctx.Reply("Server time: " + DZOCCUtil.Pad2(hour) + ":" + DZOCCUtil.Pad2(minute));
			return;
		}

		string arg = ctx.Arg(0);
		arg.ToLower();
		int newHour = hour;
		int newMinute = 0;
		if (arg == "day")
			newHour = 12;
		else if (arg == "night")
			newHour = 0;
		else
		{
			array<string> hm = new array<string>;
			arg.Split(":", hm);
			if (hm.Count() < 1 || hm.Count() > 2 || !DZOCCUtil.IsNumeric(hm[0]) || (hm.Count() == 2 && !DZOCCUtil.IsNumeric(hm[1])))
			{
				Usage(ctx);
				return;
			}
			newHour = hm[0].ToInt();
			if (hm.Count() == 2)
				newMinute = hm[1].ToInt();
			if (newHour < 0 || newHour > 23 || newMinute < 0 || newMinute > 59)
			{
				Usage(ctx);
				return;
			}
		}

		GetGame().GetWorld().SetDate(year, month, day, newHour, newMinute);
		ctx.Reply("Time set to " + DZOCCUtil.Pad2(newHour) + ":" + DZOCCUtil.Pad2(newMinute));
		ctx.Audit("set time to " + DZOCCUtil.Pad2(newHour) + ":" + DZOCCUtil.Pad2(newMinute));
	}

	protected static void Reload(DZOCCContext ctx)
	{
		DZOCCConfig.Load();
		ctx.Reply("Config reloaded.");
		ctx.Audit("reloaded config");
	}
}
