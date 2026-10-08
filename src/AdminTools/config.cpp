class CfgPatches {
    class AdminTools {
        units[] = {};
        weapons[] = {};
        requiredVersion = 1.1;
        requiredAddons[] = {"DZ_Data"};
    };
};

class CfgMods {
    class AdminTools {
        type = "servermod";
        name = "AdminTools";
        credits = "bzed";
        author = "bzed";
        version = "1.1";
        dependencies[] = {"Game", "World", "Mission"};

        class defs {
            class worldScriptModule {
                files[] = {"AdminTools/Scripts/4_World"};
            };
            class missionScriptModule {
                files[] = {"AdminTools/Scripts/5_Mission"};
            };
        };
    };
};
