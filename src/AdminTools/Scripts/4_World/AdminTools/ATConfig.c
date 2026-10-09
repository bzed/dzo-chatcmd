// AdminTools - server-side admin chat commands for DayZ
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

class ATSettings
{
	ref array<string> adminUids = new array<string>();
	bool verbose = false;
	bool auditLog = true;
	// tell the *target* player about teleports/heals done to them (off: admin actions stay private)
	bool notifyTargets = false;
	int maxSpawnCount = 50;
	// admins get god mode automatically when they log in / respawn
	bool godModeOnLogin = false;
	ref map<string, ref array<string>> spawnGroups = new map<string, ref array<string>>();
	ref map<string, vector> teleportLocations = new map<string, vector>();
	ref array<string> vehicles = new array<string>();
	// short name -> base classname for /car ("ada": "OffroadHatchback")
	ref map<string, string> vehicleAliases = new map<string, string>();
}

class ATConfig
{
	static const string DIR = "$profile:AdminTools";
	static const string FILE = "$profile:AdminTools/config.json";
	static const string AUDIT_FILE = "$profile:AdminTools/audit.log";

	protected static ref ATSettings s_Data;

	static ATSettings Get()
	{
		if (!s_Data)
			Load();
		return s_Data;
	}

	static void Load()
	{
		ATSettings data = new ATSettings();

		if (FileExist(FILE))
		{
			string err;
			if (!JsonFileLoader<ATSettings>.LoadFile(FILE, data, err))
			{
				// keep the previous config if there is one, never fall back to "everybody is admin"
				ATLog.Info("Cannot load " + FILE + ": " + err);
				if (!s_Data)
					s_Data = new ATSettings();
				return;
			}
		}
		else
		{
			MakeDirectory(DIR);
			FillDefaults(data);
			string saveErr;
			if (JsonFileLoader<ATSettings>.SaveFile(FILE, data, saveErr))
				ATLog.Info("No config found, wrote defaults to " + FILE + ". Add your player ID to adminUids.");
			else
				ATLog.Info("Cannot write default config: " + saveErr);
		}

		// Missing keys / nulls from hand-edited JSON
		if (!data.adminUids)
			data.adminUids = new array<string>();
		if (!data.spawnGroups)
			data.spawnGroups = new map<string, ref array<string>>();
		if (!data.teleportLocations)
			data.teleportLocations = new map<string, vector>();
		if (!data.vehicles || data.vehicles.Count() == 0)
		{
			data.vehicles = new array<string>();
			FillDefaultVehicles(data.vehicles);
		}
		if (!data.vehicleAliases || data.vehicleAliases.Count() == 0)
		{
			data.vehicleAliases = new map<string, string>();
			FillDefaultAliases(data.vehicleAliases);
		}
		if (data.maxSpawnCount < 1)
			data.maxSpawnCount = 1;

		s_Data = data;
		ATLog.Info("Config loaded: " + data.adminUids.Count() + " admin(s), " + data.spawnGroups.Count() + " spawn group(s), " + data.teleportLocations.Count() + " location(s), " + data.vehicles.Count() + " vehicle(s)");
		if (data.adminUids.Count() == 0)
			ATLog.Info("WARNING: no adminUids configured, all commands are disabled.");
	}

	static bool IsAdmin(PlayerIdentity identity)
	{
		if (!identity)
			return false;
		return Get().adminUids.Find(identity.GetId()) != -1;
	}

	protected static void FillDefaultVehicles(array<string> list)
	{
		list.Insert("OffroadHatchback");
		list.Insert("Sedan_02");
		list.Insert("Hatchback_02");
		list.Insert("Truck_01_Covered");
		list.Insert("Truck_01_Open");
		list.Insert("CivilianSedan");
	}

	protected static void FillDefaultAliases(map<string, string> aliases)
	{
		aliases.Insert("ada", "OffroadHatchback");
		aliases.Insert("gunter", "Hatchback_02");
		aliases.Insert("olga", "CivilianSedan");
		aliases.Insert("sarka", "Sedan_02");
		aliases.Insert("truck", "Truck_01_Covered");
		aliases.Insert("humvee", "Offroad_02");
		// vanilla has no Rubber_Boat; the boat class is Boat_01_<Color>
		aliases.Insert("boat", "Boat_01");
	}

	protected static void FillDefaults(ATSettings data)
	{
		FillDefaultAliases(data.vehicleAliases);
		FillDefaultVehicles(data.vehicles);
		array<string> medics = {"BandageDressing", "BandageDressing", "Splint", "Morphine", "TetracyclineAntibiotics", "CharcoalTablets", "IodineTincture", "SalineBagIV"};
		array<string> food = {"SpaghettiCan", "SpaghettiCan", "TacticalBaconCan", "Canteen"};
		data.spawnGroups.Insert("medics", medics);
		data.spawnGroups.Insert("food", food);
	}
}
