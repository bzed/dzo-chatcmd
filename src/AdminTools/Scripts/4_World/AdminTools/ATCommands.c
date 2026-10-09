class ATCommandInfo
{
	string name;
	string usage;
	string description;
	ref array<string> aliases = new array<string>;

	void ATCommandInfo(string n, string u, string d, string a = "")
	{
		name = n;
		usage = u;
		description = d;
		if (a.Length() > 0)
			a.Split(",", aliases);
	}
}

// Chat commands. Entry point: ATCommands.Handle(). Replies go to the sending admin only.
class ATCommands
{
	protected static ref array<ref ATCommandInfo> s_Commands;

	static array<ref ATCommandInfo> GetCommands()
	{
		if (!s_Commands)
		{
			s_Commands = new array<ref ATCommandInfo>;
			s_Commands.Insert(new ATCommandInfo("help", "/help [command]", "List commands, or show details of one", "?,commands"));
			s_Commands.Insert(new ATCommandInfo("myid", "/myid", "Show your player ID (works for everybody)"));
			s_Commands.Insert(new ATCommandInfo("players", "/players", "List online players with distance", "who,list"));
			s_Commands.Insert(new ATCommandInfo("pos", "/pos [player]", "Show position (usable with /tp x z)", "where"));
			s_Commands.Insert(new ATCommandInfo("tp", "/tp <location | player | x z>", "Teleport yourself", "teleport"));
			s_Commands.Insert(new ATCommandInfo("goto", "/goto <player>", "Teleport to a player", "teleport_me_to"));
			s_Commands.Insert(new ATCommandInfo("bring", "/bring <player>", "Teleport a player to you", "teleport_to_me"));
			s_Commands.Insert(new ATCommandInfo("back", "/back", "Return to where you were before the last teleport"));
			s_Commands.Insert(new ATCommandInfo("locations", "/locations [filter]", "List teleport locations", "locs"));
			s_Commands.Insert(new ATCommandInfo("heal", "/heal [player]", "Full heal: health, blood, food, water, diseases, bleeding, legs"));
			s_Commands.Insert(new ATCommandInfo("fixgear", "/fixgear [player]", "Repair all items in the inventory", "fix_gear"));
			s_Commands.Insert(new ATCommandInfo("kill", "/kill [player]", "Kill a player (default: yourself)"));
			s_Commands.Insert(new ATCommandInfo("kick", "/kick <player> [reason]", "Disconnect a player"));
			s_Commands.Insert(new ATCommandInfo("godmode", "/godmode [0|1]", "Damage immunity (no argument: toggle)", "god"));
			s_Commands.Insert(new ATCommandInfo("invisible", "/invisible [0|1]", "Hide your character from other players", "invis,vanish"));
			s_Commands.Insert(new ATCommandInfo("ammo", "/ammo [0|1]", "Unlimited ammo for magazine weapons", "unlimited_ammo"));
			s_Commands.Insert(new ATCommandInfo("stamina", "/stamina [0|1]", "Unlimited stamina", "unlimited_stamina"));
			s_Commands.Insert(new ATCommandInfo("spawn", "/spawn <classname> [count]", "Spawn item(s) at your feet"));
			s_Commands.Insert(new ATCommandInfo("spawngroup", "/spawngroup [group]", "Spawn a configured item group (no argument: list)", "group,groups"));
			s_Commands.Insert(new ATCommandInfo("car", "/car [type]", "Spawn a fully equipped vehicle (no argument: list)"));
			s_Commands.Insert(new ATCommandInfo("repair", "/repair", "Repair and refuel the vehicle you are in"));
			s_Commands.Insert(new ATCommandInfo("time", "/time [HH[:MM] | day | night]", "Show or set the server time"));
			s_Commands.Insert(new ATCommandInfo("reload", "/reload", "Reload config.json"));
		}
		return s_Commands;
	}

	protected static ATCommandInfo FindCommand(string name)
	{
		string lname = name;
		lname.ToLower();
		array<ref ATCommandInfo> all = GetCommands();
		for (int i = 0; i < all.Count(); i++)
		{
			ATCommandInfo info = all[i];
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

		ATCommandInfo info = FindCommand(parts[0]);
		if (!info)
			return false;

		ATContext ctx = new ATContext();
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

		if (!ATConfig.IsAdmin(sender.GetIdentity()))
		{
			ATLog.Debug("ignored /" + info.name + " from non-admin " + sender.GetIdentity().GetName());
			return false;
		}

		ATLog.Debug("/" + info.name + " " + ctx.Rest(0) + " from " + sender.GetIdentity().GetName());
		Run(info, ctx);
		return true;
	}

	protected static void Run(ATCommandInfo info, ATContext ctx)
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
			case "locations": Locations(ctx); break;
			case "heal":      Heal(ctx); break;
			case "fixgear":   FixGear(ctx); break;
			case "kill":      Kill(ctx); break;
			case "kick":      Kick(ctx); break;
			case "godmode":   God(ctx); break;
			case "invisible": Invisible(ctx); break;
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

	protected static void Usage(ATContext ctx)
	{
		ATCommandInfo info = FindCommand(ctx.command);
		if (info)
			ctx.Reply("Usage: " + info.usage);
	}

	// 0 = off, 1 = on, no argument = toggle. Returns -1 on bad input.
	protected static int ParseSwitch(ATContext ctx, bool current)
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
	protected static bool MoveTo(ATContext ctx, PlayerBase who, vector pos)
	{
		if (who.IsInTransport())
		{
			ctx.Reply(who.GetIdentity().GetName() + " is in a vehicle, cannot teleport.");
			return false;
		}
		who.m_AT_LastPos = who.GetPosition();
		who.m_AT_HasLastPos = true;
		who.SetPosition(pos);
		return true;
	}

	// ---- information ----

	protected static void Help(ATContext ctx)
	{
		if (ctx.args.Count() > 0)
		{
			ATCommandInfo info = FindCommand(ctx.Arg(0));
			if (!info)
			{
				ctx.Reply("No such command: " + ctx.Arg(0));
				return;
			}
			ctx.Reply(info.usage + " - " + info.description);
			if (info.aliases.Count() > 0)
				ctx.Reply("Aliases: /" + ATUtil.Join(", /", info.aliases));
			return;
		}

		ctx.Reply("Commands (admin only, answers are private; /help <command> for details):");
		array<ref ATCommandInfo> all = GetCommands();
		for (int i = 0; i < all.Count(); i++)
			ctx.Reply(all[i].usage + " - " + all[i].description);
	}

	protected static void Players(ATContext ctx)
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

	protected static void Pos(ATContext ctx)
	{
		PlayerBase target = ctx.admin;
		if (ctx.args.Count() > 0)
		{
			target = ctx.FindPlayer(ctx.Rest(0));
			if (!target)
				return;
		}
		ctx.Reply(target.GetIdentity().GetName() + ": " + ATUtil.FormatPos(target.GetPosition()));
	}

	protected static void Locations(ATContext ctx)
	{
		array<string> names = ATConfig.Get().teleportLocations.GetKeyArray();
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

	protected static void Teleport(ATContext ctx)
	{
		if (ctx.args.Count() == 0)
		{
			Usage(ctx);
			return;
		}

		// /tp x z
		if (ctx.args.Count() == 2 && ATUtil.IsNumeric(ctx.Arg(0)) && ATUtil.IsNumeric(ctx.Arg(1)))
		{
			vector coords = Vector(ctx.Arg(0).ToFloat(), 0, ctx.Arg(1).ToFloat());
			if (MoveTo(ctx, ctx.admin, ATUtil.SnapToGround(coords)))
			{
				ctx.Reply("Teleported to " + ATUtil.FormatPos(ctx.admin.GetPosition()));
				ctx.Audit("teleported to " + ATUtil.FormatPos(ctx.admin.GetPosition()));
			}
			return;
		}

		string query = ctx.Rest(0);
		array<string> locationNames = ATConfig.Get().teleportLocations.GetKeyArray();

		// locations first, then players
		array<int> locHits;
		ATUtil.MatchIndices(locationNames, query, locHits);
		if (locHits.Count() > 0)
		{
			string locName = ctx.FindName(locationNames, query, "location");
			if (locName.Length() == 0)
				return;
			vector locPos = ATConfig.Get().teleportLocations.Get(locName);
			if (MoveTo(ctx, ctx.admin, ATUtil.SnapToGround(locPos)))
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

	protected static void GotoPlayer(ATContext ctx)
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

	protected static void Bring(ATContext ctx)
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
			ctx.Audit("brought player " + target.GetIdentity().GetName() + " to " + ATUtil.FormatPos(ctx.admin.GetPosition()));
		}
	}

	protected static void Back(ATContext ctx)
	{
		if (!ctx.admin.m_AT_HasLastPos)
		{
			ctx.Reply("No previous position.");
			return;
		}
		vector previous = ctx.admin.m_AT_LastPos;
		if (MoveTo(ctx, ctx.admin, previous))
		{
			ctx.Reply("Back at " + ATUtil.FormatPos(previous));
			ctx.Audit("teleported back to " + ATUtil.FormatPos(previous));
		}
	}

	// ---- players ----

	protected static void Heal(ATContext ctx)
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

	protected static void FixGear(ATContext ctx)
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

	protected static void Kill(ATContext ctx)
	{
		PlayerBase target = ctx.admin;
		if (ctx.args.Count() > 0)
		{
			target = ctx.FindPlayer(ctx.Rest(0));
			if (!target)
				return;
		}
		// god mode would swallow the damage
		target.AT_SetGod(false);
		target.SetHealth("", "", 0);
		ctx.Reply("Killed " + target.GetIdentity().GetName() + ".");
		ctx.Audit("killed " + target.GetIdentity().GetName());
	}

	protected static void Kick(ATContext ctx)
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

	protected static void God(ATContext ctx)
	{
		int mode = ParseSwitch(ctx, ctx.admin.m_AT_God);
		if (mode < 0)
			return;
		ctx.admin.AT_SetGod(mode == 1);
		ctx.Reply("God mode " + OnOff(mode == 1) + ".");
		ctx.Audit("god mode " + OnOff(mode == 1));
	}

	protected static void Invisible(ATContext ctx)
	{
		int mode = ParseSwitch(ctx, ctx.admin.m_AT_Invisible);
		if (mode < 0)
			return;
		ctx.admin.AT_SetInvisible(mode == 1);
		ctx.Reply("Invisibility " + OnOff(mode == 1) + ".");
		ctx.Audit("invisibility " + OnOff(mode == 1));
	}

	protected static void UnlimitedAmmo(ATContext ctx)
	{
		int mode = ParseSwitch(ctx, ctx.admin.m_AT_UnlimitedAmmo);
		if (mode < 0)
			return;
		ctx.admin.m_AT_UnlimitedAmmo = (mode == 1);
		ctx.Reply("Unlimited ammo " + OnOff(mode == 1) + ".");
		ctx.Audit("unlimited ammo " + OnOff(mode == 1));
	}

	protected static void UnlimitedStamina(ATContext ctx)
	{
		int mode = ParseSwitch(ctx, ctx.admin.m_AT_UnlimitedStamina);
		if (mode < 0)
			return;
		ctx.admin.m_AT_UnlimitedStamina = (mode == 1);
		ctx.Reply("Unlimited stamina " + OnOff(mode == 1) + ".");
		ctx.Audit("unlimited stamina " + OnOff(mode == 1));
	}

	// ---- spawning ----

	protected static EntityAI SpawnAt(string className, vector pos)
	{
		return EntityAI.Cast(GetGame().CreateObjectEx(className, pos, ECE_PLACE_ON_SURFACE | ECE_INITAI | ECE_EQUIP_ATTACHMENTS));
	}

	protected static void Spawn(ATContext ctx)
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
		count = Math.Clamp(count, 1, ATConfig.Get().maxSpawnCount);

		if (!ATUtil.ClassExists(className))
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
		ctx.Audit("spawned " + spawned + "x " + className + " at " + ATUtil.FormatPos(ctx.admin.GetPosition()));
	}

	protected static void SpawnGroup(ATContext ctx)
	{
		map<string, ref array<string>> groups = ATConfig.Get().spawnGroups;
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
			if (ATUtil.ClassExists(className) && SpawnAt(className, ctx.admin.GetPosition()))
				spawned++;
			else
				failed.Insert(className);
		}

		ctx.Reply("Spawned " + spawned + " item(s) from '" + groupName + "'.");
		if (failed.Count() > 0)
			ctx.ReplyList("Could not spawn: ", failed);
		ctx.Audit("spawned group " + groupName + " (" + spawned + " items) at " + ATUtil.FormatPos(ctx.admin.GetPosition()));
	}

	protected static void SpawnCar(ATContext ctx)
	{
		array<string> vehicles = ATConfig.Get().vehicles;
		if (ctx.args.Count() == 0)
		{
			ctx.ReplyList("Vehicles: ", vehicles);
			return;
		}

		string carType = ctx.FindName(vehicles, ctx.Arg(0), "vehicle");
		if (carType.Length() == 0)
			return;

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
		ctx.Audit("spawned vehicle " + carType + " at " + ATUtil.FormatPos(pos));
	}

	protected static void Repair(ATContext ctx)
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

	protected static void ShowTime(ATContext ctx)
	{
		int year, month, day, hour, minute;
		GetGame().GetWorld().GetDate(year, month, day, hour, minute);

		if (ctx.args.Count() == 0)
		{
			ctx.Reply("Server time: " + ATUtil.Pad2(hour) + ":" + ATUtil.Pad2(minute));
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
			if (hm.Count() < 1 || hm.Count() > 2 || !ATUtil.IsNumeric(hm[0]) || (hm.Count() == 2 && !ATUtil.IsNumeric(hm[1])))
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
		ctx.Reply("Time set to " + ATUtil.Pad2(newHour) + ":" + ATUtil.Pad2(newMinute));
		ctx.Audit("set time to " + ATUtil.Pad2(newHour) + ":" + ATUtil.Pad2(newMinute));
	}

	protected static void Reload(ATContext ctx)
	{
		ATConfig.Load();
		ctx.Reply("Config reloaded.");
		ctx.Audit("reloaded config");
	}
}
