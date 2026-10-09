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

class CfgPatches
{
	class AdminTools
	{
		units[] = {};
		weapons[] = {};
		requiredVersion = 0.1;
		requiredAddons[] = {"DZ_Data", "DZ_Scripts"};
	};
};

class CfgMods
{
	class AdminTools
	{
		type = "servermod";
		name = "AdminTools";
		credits = "bzed";
		author = "bzed";
		version = "2.0";
		dependencies[] = {"Game", "World", "Mission"};

		class defs
		{
			class worldScriptModule
			{
				value = "";
				files[] = {"AdminTools/Scripts/4_World"};
			};
			class missionScriptModule
			{
				value = "";
				files[] = {"AdminTools/Scripts/5_Mission"};
			};
		};
	};
};
