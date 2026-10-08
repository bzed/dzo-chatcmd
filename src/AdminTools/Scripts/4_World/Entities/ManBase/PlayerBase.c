modded class PlayerBase
{
    /*
    bool m_UnlimitedAmmo = false;
    bool m_UnlimitedStamina = false;

    void PlayerBase()
    {
        // Registering events (only on server)
        if (GetGame().IsServer()) {
            RegisterNetSyncVariableBool("m_UnlimitedAmmo");
            RegisterNetSyncVariableBool("m_UnlimitedStamina");
        }
    }

    // Called regularly (not necessarily every frame)
    override void OnScheduledTick(float deltaTime)
    {
        super.OnScheduledTick(deltaTime);

        if (!GetGame().IsServer()) {
            return; // Only run this logic on the server
        }

        if (m_UnlimitedStamina)
        {
            // Set the stamina and stamina capacity to the maximum.
            SetStamina(GameConstants.STAMINA_MAX, GameConstants.STAMINA_MAX);
        }

        if (m_UnlimitedAmmo)
        {
            // Refill ammo in weapon
            Weapon_Base weapon = Weapon_Base.Cast(GetItemInHands());
            if (weapon)
            {
                Magazine magazine = weapon.GetMagazine(weapon.GetCurrentMuzzle());
                if (magazine)
                {
                    magazine.ServerSetAmmoCount(magazine.GetAmmoMax()); // Server-authoritative refill
                }
            }
        }
    }


    // Example of how to toggle these (e.g., via admin menu)
    void SetUnlimitedAmmo(bool enabled)
    {
        if (GetGame().IsServer())
        {
            m_UnlimitedAmmo = enabled;
            SetSynchDirty(); // VERY IMPORTANT: Tell the game to synchronize this variable
        }
    }

    void SetUnlimitedStamina(bool enabled)
    {
        if (GetGame().IsServer())
        {
            m_UnlimitedStamina = enabled;
            SetSynchDirty(); // VERY IMPORTANT: Tell the game to synchronize this variable
        }
    }
    */
};
