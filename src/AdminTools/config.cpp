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
