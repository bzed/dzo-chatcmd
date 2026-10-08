modded class MissionServer
{
    // Define for enabling/disabling debug messages
    #define DEBUG // Uncomment this line to enable debug messages

    // Define for enabling/disabling admin logging
    #define ADMIN_LOGGING // Uncomment this line to enable admin logging

    // Static array to store the list of admin UIDs
    static ref array<string> s_AdminUids;
    static ref map<string, ref array<string>> s_SpawnGroups; // Ensure inner array is also ref
    static autoptr array<string> s_ValidCarTypes = { // List of valid vehicle classnames
        "OffroadHatchback",
        "Sedan_02",
        "Hatchback_02",
        "Truck_01_Covered",
        "Truck_01_Open",
        "CivilianSedan"
    };
    static ref map<string, vector> s_TeleportLocations; // To store named teleport locations

    // Static instance of the admin config
    static ref AdminConfig s_AdminConfig;

    // Helper function to check if admin logging is enabled
    static bool IsAdminLoggingEnabled()
    {
    #ifdef ADMIN_LOGGING
        return true;
    #else
        return false;
    #endif
    }

    // Helper function to check if debugging is enabled
    static bool IsDebugEnabled()
    {
    #ifdef DEBUG
        return true;
    #else
        return false;
    #endif
    }

    // from COT
    static vector SnapToGround(vector pos, bool aboveWater = true)
    {
        float pos_x = pos[0];
        float pos_z = pos[2];
        float pos_y;

        if (pos[1] == 0.0)
            pos_y = GetGame().SurfaceRoadY3D( pos_x, 1000.0, pos_z, RoadSurfaceDetection.UNDER );
        else
            pos_y = GetGame().SurfaceRoadY3D( pos_x, pos[1], pos_z, RoadSurfaceDetection.CLOSEST );

        vector tmp_pos = Vector( pos_x, pos_y, pos_z );

        if (aboveWater)
        {
            float waterDepth = GetGame().GetWaterDepth(tmp_pos);

            if (waterDepth > 0)
                tmp_pos[1] = tmp_pos[1] + waterDepth;
        }

        return tmp_pos;
    }

    // Function to load admin UIDs, spawn groups, and teleport locations from the config file
    static void LoadConfig()
    {
        s_AdminUids = new ref array<string>;
        s_SpawnGroups = new ref map<string, ref array<string>>();
        s_TeleportLocations = new ref map<string, vector>();
        string configPath = "$profile:AdminTools/config.json"; // Updated mod name

        if (IsDebugEnabled()) Print("[AdminTools] LoadConfig: Attempting to load config from " + configPath);

        s_AdminConfig = new AdminConfig(); // Ensure s_AdminConfig is initialized

        if (FileExist(configPath)) // Check if the file exists
        {
            JsonFileLoader<ref AdminConfig>.JsonLoadFile(configPath, s_AdminConfig);
            if (s_AdminConfig)
            {
                if (s_AdminConfig.adminUids)
                {
                    s_AdminUids = s_AdminConfig.adminUids;
                    if (IsDebugEnabled()) Print("[AdminTools] LoadConfig: Loaded " + s_AdminUids.Count() + " admin UIDs.");
                }
                else
                {
                    Error("[AdminTools] LoadConfig: Error loading admin UIDs from config file.");
                }

                if (s_AdminConfig.spawnGroups)
                {
                    s_SpawnGroups = s_AdminConfig.spawnGroups;
                    if (IsDebugEnabled()) Print("[AdminTools] LoadConfig: Loaded " + s_SpawnGroups.Count() + " spawn groups.");
                }
                else
                {
                    if (IsDebugEnabled()) Print("[AdminTools] LoadConfig: No spawn groups found in config.");
                }

                if (s_AdminConfig.teleportLocations)
                {
                    s_TeleportLocations = s_AdminConfig.teleportLocations;
                    if (IsDebugEnabled()) Print("[AdminTools] LoadConfig: Loaded " + s_TeleportLocations.Count() + " teleport locations.");
                }
                else
                {
                    if (IsDebugEnabled()) Print("[AdminTools] LoadConfig: No teleport locations found in config.");
                }
            }
            else
            {
                Error("[AdminTools] LoadConfig: Error loading config file content.");
            }
        }
        else
        {
            Error("[AdminTools] LoadConfig: Config file not found at: " + configPath);
        }
    }

    // Static function to check if a player is an admin
    static bool IsAdmin(string playerId)
    {
        if (IsDebugEnabled()) Print("[AdminTools] IsAdmin: Checking if player " + playerId + " is an admin.");
        if (!s_AdminUids)
        {
            if (IsDebugEnabled()) Print("[AdminTools] IsAdmin: s_AdminUids is null.");
            return false;
        }

        for (int i = 0; i < s_AdminUids.Count(); i++)
        {

            if (IsDebugEnabled()) Print("[AdminTools] IsAdmin: Player " + playerId + " == " + s_AdminUids[i]);
            if (s_AdminUids[i] == playerId)
            {
                if (IsDebugEnabled()) Print("[AdminTools] IsAdmin: Player " + playerId + " is an admin.");
                return true;
            }
        }
        if (IsDebugEnabled()) Print("[AdminTools] IsAdmin: Player " + playerId + " is NOT an admin.");
        return false;
    }

    // New function to find a player by name
    static PlayerBase FindPlayerByName(string playerName)
    {
        if (IsDebugEnabled()) Print("[AdminTools] FindPlayerByName: Searching for player " + playerName);
        if (playerName.Length() == 0) // Replaced IsEmpty()
        {
            if (IsDebugEnabled()) Print("[AdminTools] FindPlayerByName: Player name is empty.");
            return null;
        }

        array<Man> onlinePlayers = new array<Man>;
        GetGame().GetPlayers(onlinePlayers);
        PlayerBase foundPlayer;

        foreach (Man player : onlinePlayers)
        {
            PlayerIdentity playerId = player.GetIdentity();
            if (playerId && playerId.GetName() == playerName)
            {
                foundPlayer = PlayerBase.Cast(player);
                if (IsDebugEnabled()) Print("[AdminTools] FindPlayerByName: Found player " + playerName + " (UID: " + playerId.GetId() + ").");
                break;
            }
        }

        if (!foundPlayer && IsDebugEnabled())
        {
            Print("[AdminTools] FindPlayerByName: Player " + playerName + " not found.");
            return null;
        }
        return foundPlayer;
    }

    // --- Command Handler Functions ---

    private static void HandleAdminCommand_Godmode(array<string> commandParts, string senderUid, string senderName)
    {
        if (commandParts.Count() != 2) {
            player.MessageStatus("[AdminTools] Usage: /godmode <0/1>");
            return;
        }

        int godModeValue = commandParts[1].ToInt();
        player.SetAllowDamage(!godModeValue); // Setting AllowDamage to false is god mode
        string status;
        if (godModeValue == 1)
        {
            status = "enabled";
        }
        else
        {
            status = "disabled";
        }
        player.MessageStatus("[AdminTools] God mode " + status + " for you.");
        if (IsAdminLoggingEnabled()) {
            Print(
                    "[AdminTools] ADMIN LOG: Admin " + senderName + " (UID: " + senderUid + ") toggled god mode " + status + " for themselves."
            );
        }
    }

    private static void HandleAdminCommand_Invisible(PlayerBase player, array<string> commandParts, string senderUid, string senderName)
    {

        if (commandParts.Count() != 2) {
            player.MessageStatus("[AdminTools] Usage: /invisible <0/1>");
            return;
        }
        int invisibleValue = commandParts[1].ToInt();
        player.SetInvisibleRecursive(invisibleValue == 1, player);
        player.SetSynchDirty();
        string status;
        if (invisibleValue == 1)
        {
            status = "enabled";
        }
        else
        {
            status = "disabled";
        }
        player.MessageStatus("[AdminTools] Invisibility " + status + " for you.");
        if (IsAdminLoggingEnabled()) Print("[AdminTools] ADMIN LOG: Admin " + senderName + " (UID: " + senderUid + ") toggled invisibility " + status + " for themselves.");
    }

    /*
    private static void HandleAdminCommand_UnlimitedAmmo(PlayerBase player, array<string> commandParts, string senderUid, string senderName)
    {
        PlayerIdentity playerId = player.GetIdentity();
        if (!playerId)
            return;

        if (commandParts.Count() == 2)
        {
            int ammoValue = commandParts[1].ToInt();
            player.SetUnlimitedAmmo(ammoValue == 1);
            string status;
            if (ammoValue == 1)
            {
                status = "enabled";
            }
            else
            {
                status = "disabled";
            }
            player.MessageStatus("[AdminTools] Unlimited ammo " + status + " for you.");
            if (IsAdminLoggingEnabled()) Print("[AdminTools] ADMIN LOG: Admin " + senderName + " (UID: " + senderUid + ") toggled unlimited ammo " + status + ".");
        }
        else
        {
            player.MessageStatus("[AdminTools] Usage: /unlimited_ammo <0/1>");
        }
    }
    */

    /*
    private static void HandleAdminCommand_UnlimitedStamina(PlayerBase player, array<string> commandParts, string senderUid, string senderName)
    {
        PlayerIdentity playerId = player.GetIdentity();
        if (!playerId)
            return;

        if (commandParts.Count() == 2)
        {
            int staminaValue = commandParts[1].ToInt();
            player.SetUnlimitedStamina(staminaValue == 1);
            string status;
            if (staminaValue == 1)
            {
                status = "enabled";
            }
            else
            {
                status = "disabled";
            }
            player.MessageStatus("[AdminTools] Unlimited stamina " + status + " for you.");
            if (IsAdminLoggingEnabled()) Print("[AdminTools] ADMIN LOG: Admin " + senderName + " (UID: " + senderUid + ") toggled unlimited stamina " + status + ".");
        }
        else
        {
            player.MessageStatus("[AdminTools] Usage: /unlimited_stamina <0/1>");
        }
    }
    */

    private static void HandleAdminCommand_Spawn(PlayerBase player, array<string> commandParts, string senderUid, string senderName)
    {
        if (commandParts.Count() != 2) {
            player.MessageStatus("[AdminTools] Usage: /spawn <classname>");
            return;
        }

        string className = commandParts[1];
        vector spawnPosition = player.GetPosition(); // Spawn at the admin's position
        if (IsDebugEnabled()) Print("[AdminTools] HandleChatMessage: /spawn command received from " + senderUid + ". Class: " + className + " at " + spawnPosition);
        EntityAI spawnedEntity = EntityAI.Cast(GetGame().CreateObject(className, spawnPosition, false, false, true)); // Safe downcasting
        if (spawnedEntity)
        {
            player.MessageStatus("[AdminTools] Spawned: " + className);
            if (IsAdminLoggingEnabled()) Print("[AdminTools] ADMIN LOG: Admin " + senderName + " (UID: " + senderUid + ") spawned: " + className + " at " + spawnPosition + ".");
        }
        else
        {
            player.MessageStatus("[AdminTools] Could not spawn: " + className);
            Error("[AdminTools] HandleChatMessage: Could not spawn " + className + " by " + senderUid);
        }
    }

    private static void HandleAdminCommand_TeleportMeTo(PlayerBase player, array<string> commandParts, string senderUid, string senderName)
    {
        if (commandParts.Count() != 2) {
            player.MessageStatus("[AdminTools] Usage: /teleport_me_to <player name>");
            return;
        }

        string targetPlayerName = "";
        for (int i = 1; i < commandParts.Count(); i++)
        {
            targetPlayerName += commandParts[i] + " ";
        }
        targetPlayerName.TrimInPlace();
        if (IsDebugEnabled()) Print("[AdminTools] HandleChatMessage: /teleport_me_to command received from " + senderUid + ". Target: " + targetPlayerName);
        PlayerBase targetPlayer = FindPlayerByName(targetPlayerName);
        if (targetPlayer)
        {
            player.SetPosition(targetPlayer.GetPosition());
            player.MessageStatus("[AdminTools] Teleported to " + targetPlayerName + ".");
            if (IsAdminLoggingEnabled()) Print("[AdminTools] ADMIN LOG: Admin " + senderName + " (UID: " + senderUid + ") teleported to player: " + targetPlayerName + " at " + targetPlayer.GetPosition() + ".");
        }
        else
        {
            player.MessageStatus("[AdminTools] Player '" + targetPlayerName + "' not found.");
            if (IsDebugEnabled()) Print("[AdminTools] HandleChatMessage: Player " + targetPlayerName + " not found by " + senderUid + ".");
        }
    }

    private static void HandleAdminCommand_TeleportToMe(PlayerBase player, array<string> commandParts, string senderUid, string senderName)
    {
        if (commandParts.Count() >= 2)
        {
            string targetPlayerName = "";
            for (int i = 1; i < commandParts.Count(); i++)
            {
                targetPlayerName += commandParts[i] + " ";
            }
            targetPlayerName.TrimInPlace();
            if (IsDebugEnabled()) Print("[AdminTools] HandleChatMessage: /teleport_to_me command received from " + senderUid + ". Target: " + targetPlayerName);
            PlayerBase targetPlayer = FindPlayerByName(targetPlayerName);
            if (targetPlayer)
            {
                targetPlayer.SetPosition(player.GetPosition());
                targetPlayer.MessageStatus("[AdminTools] Teleported to you.");
                player.MessageStatus("[AdminTools] Teleported " + targetPlayerName + " to your location.");
                if (IsAdminLoggingEnabled()) Print("[AdminTools] ADMIN LOG: Admin " + senderName + " (UID: " + senderUid + ") teleported player " + targetPlayerName + " to their position at " + player.GetPosition() + ".");
            }
            else
            {
                player.MessageStatus("[AdminTools] Player '" + targetPlayerName + "' not found.");
                if (IsDebugEnabled()) Print("[AdminTools] HandleChatMessage: Player " + targetPlayerName + " not found by " + senderUid + ".");
            }
        }
        else
        {
            player.MessageStatus("[AdminTools] Usage: /teleport_to_me <player name>");
        }
    }

    private static void HandleAdminCommand_SpawnGroup(PlayerBase player, array<string> commandParts, string senderUid, string senderName)
    {
        PlayerIdentity playerId = player.GetIdentity();
        if (!playerId)
            return;

        if (commandParts.Count() >= 2)
        {
            string groupName = commandParts[1];
            if (IsDebugEnabled()) Print("[AdminTools] HandleChatMessage: /spawngroup command received from " + senderUid + ". Group: " + groupName);
            if (s_SpawnGroups.Contains(groupName))
            {
                array<string> classesToSpawn = s_SpawnGroups.Get(groupName);
                if (classesToSpawn && classesToSpawn.Count() > 0)
                {
                    int spawnedCount = 0;
                    foreach (string className : classesToSpawn)
                    {
                        vector spawnPosition = player.GetPosition();
                        if (IsDebugEnabled()) Print("[AdminTools] HandleChatMessage: Attempting to spawn " + className + " from group by " + senderUid + ".");
                        EntityAI spawnedEntity = EntityAI.Cast(GetGame().CreateObject(className, spawnPosition, false, false, true)); // Safe downcasting
                        if (spawnedEntity)
                        {
                            spawnedCount++;
                            if (IsDebugEnabled()) Print("[AdminTools] HandleChatMessage: Spawned " + className + " from group by " + senderUid + ".");
                        }
                        else
                        {
                            player.MessageStatus("[AdminTools] Could not spawn: " + className);
                            Error("[AdminTools] HandleChatMessage: Could not spawn " + className + " from group " + groupName + " by " + senderUid);
                        }
                    }
                    player.MessageStatus("[AdminTools] Spawned " + spawnedCount + " items from group '" + groupName + "'.");
                    if (IsAdminLoggingEnabled()) Print("[AdminTools] ADMIN LOG: Admin " + senderName + " (UID: " + senderUid + ") spawned " + spawnedCount + " items from group '" + groupName + "' at " + player.GetPosition() + ".");
                }
                else
                {
                    player.MessageStatus("[AdminTools] Spawn group '" + groupName + "' is empty.");
                    if (IsDebugEnabled()) Print("[AdminTools] HandleChatMessage: Spawn group " + groupName + " is empty for " + senderUid + ".");
                }
            }
            else
            {
                string availableGroups = "";
                if (s_SpawnGroups.GetKeyArray().Count() > 0)
                {
                    availableGroups = string.Join(", ", s_SpawnGroups.GetKeyArray());
                    player.MessageStatus("[AdminTools] Spawn group '" + groupName + "' not found. Available groups are: " + availableGroups);
                    if (IsDebugEnabled()) Print("[AdminTools] HandleChatMessage: Spawn group " + groupName + " not found by " + senderUid + ". Available: " + availableGroups);
                }
                else
                {
                    player.MessageStatus("[AdminTools] Spawn group '" + groupName + "' not found. No spawn groups are configured.");
                    if (IsDebugEnabled()) Print("[AdminTools] HandleChatMessage: Spawn group " + groupName + " not found for " + senderUid + ". No groups configured.");
                }
            }
        }
        else
        {
            player.MessageStatus("[AdminTools] Usage: /spawngroup <group name>");
        }
    }

    private static void HandleAdminCommand_Repair(PlayerBase player, array<string> commandParts, string senderUid, string senderName)
    {
        if (commandParts.Count() != 1)
        {
            player.MessageStatus("[AdminTools] Usage: /repair");
            return;
        }

        if (IsDebugEnabled()) Print("[AdminTools] HandleChatMessage: /repair command received from " + senderUid + ".");
        CarScript vehicle = CarScript.Cast(player.GetParent()); // Safe downcasting
        if (vehicle)
        {
            bool repaired = false;
            array<string> dmgZones = new array<string>;
            vehicle.GetDamageZones(dmgZones);
            for (int i = 0; i < dmgZones.Count(); i++)
            {
                vehicle.SetHealth01(dmgZones[i], "", 1.0); // Set health to 100%
                repaired = true;
                if (IsDebugEnabled()) Print("[AdminTools] HandleChatMessage: Repaired damage zone: " + dmgZones[i] + " by " + senderUid + ".");
            }
            if (repaired)
            {
                player.MessageStatus("[AdminTools] The vehicle you are in has been repaired.");
                if (IsAdminLoggingEnabled()) Print("[AdminTools] ADMIN LOG: Admin " + senderName + " (UID: " + senderUid + ") repaired the vehicle they were in.");
            }
            else
            {
                player.MessageStatus("[AdminTools] Could not repair the vehicle.");
                if (IsDebugEnabled()) Print("[AdminTools] HandleChatMessage: Could not repair the vehicle for " + senderUid + ".");
            }

            repaired = false;
            array<EntityAI> items = new array<EntityAI>();
            vehicle.GetInventory().EnumerateInventory(InventoryTraversalType.INORDER, items);
            foreach (EntityAI item : items)
            {
                item.SetHealthMax("", "");
                repaired = true;
                if (IsDebugEnabled()) Print("[AdminTools] HandleChatMessage: Repaired car item: " + item + " by " + senderUid + ".");
            }
            if (repaired)
            {
                player.MessageStatus("[AdminTools] The parts of the vehicle you are in have been repaired.");
                if (IsAdminLoggingEnabled()) Print("[AdminTools] ADMIN LOG: Admin " + senderName + " (UID: " + senderUid + ") repaired parts of the vehicle they were in.");
            }
            else
            {
                player.MessageStatus("[AdminTools] Could not repair the parts the vehicle.");
                if (IsDebugEnabled()) Print("[AdminTools] HandleChatMessage: Could not repair the parts of the vehicle for " + senderUid + ".");
            }
            vehicle.Fill(CarFluid.FUEL, 200.0);
            vehicle.Fill(CarFluid.COOLANT, 200.0);
            vehicle.Fill(CarFluid.OIL, 200.0);
        }
        else
        {
            player.MessageStatus("[AdminTools] You are not sitting in a vehicle.");
            if (IsDebugEnabled()) Print("[AdminTools] HandleChatMessage: Player " + senderUid + " is not in a vehicle.");
        }
    }


    private static void HandleAdminCommand_FixGear(PlayerBase player, array<string> commandParts, string senderUid, string senderName)
    {
        PlayerIdentity playerId = player.GetIdentity();
        if (!playerId)
            return;

        string targetName = "";
        if (commandParts.Count() > 1)
        {
            for (int i = 1; i < commandParts.Count(); i++)
            {
                targetName += commandParts[i] + " ";
            }
            targetName.TrimInPlace();
        }
        if (IsDebugEnabled()) Print("[AdminTools] HandleChatMessage: /fix_gear command received from " + senderUid + ". Target: " + targetName);

        PlayerBase targetPlayerToFix = player; // Default to sender
        if (targetName.Length() > 0)
        {
            PlayerBase foundPlayer = FindPlayerByName(targetName);
            if (foundPlayer)
            {
                targetPlayerToFix = foundPlayer;
                if (IsDebugEnabled()) Print("[AdminTools] HandleChatMessage: Found target player for fix_gear: " + targetName + " by " + senderUid + ".");
            }
            else if (targetPlayerToFix != player)
            {
                player.MessageStatus("[AdminTools] Player '" + targetName + "' not found.");
                if (IsDebugEnabled()) Print("[AdminTools] HandleChatMessage: Target player for fix_gear not found: " + targetName + " by " + senderUid + ".");
                return;
            }
        }

        targetPlayerToFix.FixAllInventoryItems();

        string fixedPlayerName;
        if (targetPlayerToFix == player)
        {
            fixedPlayerName = "your";
        }
        else
        {
            fixedPlayerName = targetPlayerToFix.GetIdentity().GetName() + "'s"; // Use GetName() for consistency, even if the player is offline, though command requires them online
        }
        player.MessageStatus("[AdminTools] Fixed items in " + fixedPlayerName + " inventory.");
        if (IsAdminLoggingEnabled()) Print("[AdminTools] ADMIN LOG: Admin " + senderName + " (UID: " + senderUid + ") fixed items in " + fixedPlayerName + " inventory.");
    }


    private static void HandleAdminCommand_Car(PlayerBase player, array<string> commandParts, string senderUid, string senderName)
    {
        PlayerIdentity playerId = player.GetIdentity();
        if (!playerId)
            return;

        string validTypesString;
        if (commandParts.Count() >= 2)
        {
            string carType = commandParts[1];
            if (IsDebugEnabled()) Print("[AdminTools] HandleChatMessage: /car command received from " + senderUid + ". Type: " + carType);
            if (s_ValidCarTypes.Find(carType) != -1) // Check if the car type is in the valid list
            {
                vector currentPosition = player.GetPosition();
                vector spawnPosition = Vector(currentPosition[0] + 1.5, currentPosition[1], currentPosition[2] + 1.5);
                EntityAI spawnedVehicle = EntityAI.Cast(GetGame().CreateObject(carType, spawnPosition, false, false, true));
                CarScript car = CarScript.Cast(spawnedVehicle); // Safe downcasting

                if (car)
                {

                    car.OnDebugSpawn();

                    player.MessageStatus("[AdminTools] Spawned complete vehicle: " + carType);
                    if (IsAdminLoggingEnabled()) Print("[AdminTools] ADMIN LOG: Admin " + senderName + " (UID: " + senderUid + ") spawned a complete vehicle: " + carType + " at " + spawnPosition + ".");
                }
                else
                {
                    player.MessageStatus("[AdminTools] Could not spawn vehicle: " + carType);
                    Error("[AdminTools] HandleChatMessage: Could not spawn vehicle: " + carType + " by " + senderUid);
                }
            }
            else
            {
                validTypesString = string.Join(", ", s_ValidCarTypes);
                player.MessageStatus("[AdminTools] Invalid car type. Valid types are: " + validTypesString);
                if (IsDebugEnabled()) Print("[AdminTools] HandleChatMessage: Invalid car type: " + carType + " by " + senderUid + ". Valid types: " + validTypesString);
            }
        }
        else
        {
            validTypesString = string.Join("|", s_ValidCarTypes);
            player.MessageStatus("[AdminTools] Usage: /car <" + validTypesString + ">");
        }
    }

    private static void HandleAdminCommand_Heal(PlayerBase player, array<string> commandParts, string senderUid, string senderName)
    {
        PlayerIdentity playerId = player.GetIdentity();
        if (!playerId)
            return;

        string targetName = "";
        if (commandParts.Count() > 1)
        {
            for (int i = 1; i < commandParts.Count(); i++)
            {
                targetName += commandParts[i] + " ";
            }
            targetName.TrimInPlace();
        }
        if (IsDebugEnabled()) Print("[AdminTools] HandleChatMessage: /heal command received from " + senderUid + ". Target: " + targetName);
        PlayerBase targetPlayerToHeal = player; // Default to sender
        if (targetName.Length() > 0) // Replaced IsEmpty()
        {
            PlayerBase foundPlayer = FindPlayerByName(targetName);
            if (foundPlayer)
            {
                targetPlayerToHeal = foundPlayer;
                if (IsDebugEnabled()) Print("[AdminTools] HandleChatMessage: Found target player for heal: " + targetName + " by " + senderUid + ".");
            }
            else if (targetPlayerToHeal != player)
            {
                player.MessageStatus("[AdminTools] Player '" + targetName + "' not found.");
                if (IsDebugEnabled()) Print("[AdminTools] HandleChatMessage: Target player for heal not found: " + targetName + " by " + senderUid + ".");
                return;
            }
        }

        targetPlayerToHeal.GetStomach().ClearContents();
        DamageSystem.ResetAllZones(targetPlayerToHeal);
        targetPlayerToHeal.GetModifiersManager().ResetAll();
        targetPlayerToHeal.RemoveAllAgents();

        BleedingSourcesManagerServer bleedingManager = targetPlayerToHeal.GetBleedingManagerServer();
        if (bleedingManager) {
            bleedingManager.RemoveAllSources();
        }

        // Heal health and set heat buffer
        targetPlayerToHeal.SetHealth("", "", targetPlayerToHeal.GetMaxHealth("", ""));
        targetPlayerToHeal.SetHeatBufferDynamicMax(3.0); // Replaced undefined function



        if (targetPlayerToHeal.IsUnconscious()) {
            DayZPlayerSyncJunctures.SendPlayerUnconsciousness(targetPlayerToHeal, false);
        }
        targetPlayerToHeal.GetStatWater().Set(targetPlayerToHeal.GetStatWater().GetMax());
        targetPlayerToHeal.GetStatEnergy().Set(targetPlayerToHeal.GetStatEnergy().GetMax());

        targetPlayerToHeal.GetStatHeatBuffer().Set(targetPlayerToHeal.GetStatHeatBuffer().GetMax());

        // Remove bloody hands
        targetPlayerToHeal.SetBloodyHands(false);

        // Fix broken legs
        targetPlayerToHeal.SetBrokenLegs(false);

        // clean modifiers. might be redundant...
        string healedPlayerName;
        if (targetPlayerToHeal == player)
        {
            healedPlayerName = "You";
        }
        else
        {
            healedPlayerName = targetName;
        }
        player.MessageStatus("[AdminTools] " + healedPlayerName + " have been fully healed.");
        if (IsAdminLoggingEnabled()) Print("[AdminTools] ADMIN LOG: Admin " + senderName + " (UID: " + senderUid + ") healed player: " + healedPlayerName + ".");
    }



    private static void HandleAdminCommand_Teleport(PlayerBase player, array<string> commandParts, string senderUid, string senderName)
    {
        PlayerIdentity playerId = player.GetIdentity();
        if (!playerId)
            return;

        if (commandParts.Count() >= 2)
        {
            string locationOrPlayerName = "";
            for (int i = 1; i < commandParts.Count(); i++)
            {
                locationOrPlayerName += commandParts[i] + " ";
            }
            locationOrPlayerName.TrimInPlace();

            if (IsDebugEnabled()) Print("[AdminTools] HandleChatMessage: /teleport command received from " + senderUid + ". Target: " + locationOrPlayerName);
            if (s_TeleportLocations.Contains(locationOrPlayerName))
            {
                player.SetPosition(SnapToGround(s_TeleportLocations.Get(locationOrPlayerName)));
                player.MessageStatus("[AdminTools] Teleported to location: " + locationOrPlayerName);
                if (IsAdminLoggingEnabled()) Print("[AdminTools] ADMIN LOG: Admin " + senderName + " (UID: " + senderUid + ") teleported to location: " + locationOrPlayerName + " at " + s_TeleportLocations.Get(locationOrPlayerName) + ".");
            }
            else
            {
                PlayerBase targetPlayer = FindPlayerByName(locationOrPlayerName);
                if (targetPlayer)
                {
                    player.SetPosition(targetPlayer.GetPosition());
                    player.MessageStatus("[AdminTools] Teleported to player: " + locationOrPlayerName);
                    if (IsAdminLoggingEnabled()) Print("[AdminTools] ADMIN LOG: Admin " + senderName + " (UID: " + senderUid + ") teleported to player: " + locationOrPlayerName + " at " + targetPlayer.GetPosition() + ".");
                }
                else
                {
                    player.MessageStatus("[AdminTools] Location or player '" + locationOrPlayerName + "' not found.");
                    if (IsDebugEnabled()) Print("[AdminTools] HandleChatMessage: Location or player '" + locationOrPlayerName + "' not found by " + senderUid + ".");
                }
            }
        }
        else
        {
            player.MessageStatus("[AdminTools] Usage: /teleport <location name> or <player name>");
        }
    }

    // Function to handle chat messages
    void HandleChatMessage(PlayerBase player, string message)
    {
        if (!player)
            return;

        PlayerIdentity playerId = player.GetIdentity();
        if (!playerId)
            return;


        string senderUid = player.GetIdentity().GetId();
        string senderName = player.GetIdentity().GetName(); // Get the admin's name for logging
        array<string> commandParts = new array<string>;
        message.Split(" ", commandParts);

        if (!IsAdmin(senderUid))
        {
            if (IsDebugEnabled()) Print("[AdminTools] HandleChatMessage: Player " + senderUid + " is not an admin. Ignoring command.");
            return;
        }

        if (commandParts.Count() > 0)
        {
            switch (commandParts[0])
            {
                case "/godmode":
                    HandleAdminCommand_Godmode(commandParts, senderUid, senderName);
                    break;
                case "/invisible":
                    HandleAdminCommand_Invisible(player, commandParts, senderUid, senderName);
                    break;
                /*
                case "/unlimited_ammo":
                    HandleAdminCommand_UnlimitedAmmo(player, commandParts, senderUid, senderName);
                    break;
                case "/unlimited_stamina":
                    HandleAdminCommand_UnlimitedStamina(player, commandParts, senderUid, senderName);
                    break;
                */
                case "/spawn":
                    HandleAdminCommand_Spawn(player, commandParts, senderUid, senderName);
                    break;
                case "/teleport_me_to":
                    HandleAdminCommand_TeleportMeTo(player, commandParts, senderUid, senderName);
                    break;
                case "/teleport_to_me":
                    HandleAdminCommand_TeleportToMe(player, commandParts, senderUid, senderName);
                    break;
                case "/spawngroup":
                    HandleAdminCommand_SpawnGroup(player, commandParts, senderUid, senderName);
                    break;
                case "/repair":
                    HandleAdminCommand_Repair(player, commandParts, senderUid, senderName);
                    break;
                case "/car":
                    HandleAdminCommand_Car(player, commandParts, senderUid, senderName);
                    break;
                case "/heal":
                    HandleAdminCommand_Heal(player, commandParts, senderUid, senderName);
                    break;
                case "/fix_gear":
                    HandleAdminCommand_FixGear(player, commandParts, senderUid, senderName);
                    break;
                case "/teleport":
                    HandleAdminCommand_Teleport(player, commandParts, senderUid, senderName);
                    break;
                default:
                    //player.MessageStatus("[AdminTools] Unknown command. Type a valid command.");
                    //if (IsDebugEnabled()) Print("[AdminTools] HandleChatMessage: Unknown command '" + commandParts[0] + "' received from " + senderUid + ".");
                    break;
            }
        }
    }

    override void OnEvent(EventType eventTypeId, Param params)
    {
        super.OnEvent(eventTypeId, params);

        if (eventTypeId == ChatMessageEventTypeID)
        {
            ChatMessageEventParams chatParams = ChatMessageEventParams.Cast(params);
            if (chatParams)
            {
                string playerName = chatParams.param2;
                string messageText = chatParams.param3;

                if (IsDebugEnabled())
                {
                    Print("[AdminTools] OnEvent: Chat message received - From: " + playerName + ", Text: " + messageText);
                }

                PlayerBase senderPlayer = FindPlayerByName(playerName);
                if (senderPlayer)
                {
                    HandleChatMessage(senderPlayer, messageText);
                }
                else if (IsDebugEnabled())
                {
                    Print("[AdminTools] OnEvent: Could not find player with name: " + playerName);
                }
            }
        }
    }

    override void OnInit()
    {
        super.OnInit();
        LoadConfig(); // Load admin config on server initialization
    }
};

// Structure to hold the admin config data
class AdminConfig
{
    ref array<string> adminUids;
    ref map<string, ref array<string>> spawnGroups;
    ref map<string, vector> teleportLocations;
} ;
