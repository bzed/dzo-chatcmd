// Server-side state for the toggle commands. Nothing here is synchronized to clients.
modded class PlayerBase
{
	bool m_AT_God;
	bool m_AT_Invisible;
	bool m_AT_UnlimitedAmmo;
	bool m_AT_UnlimitedStamina;
	bool m_AT_HasLastPos;
	vector m_AT_LastPos;

	override void OnScheduledTick(float deltaTime)
	{
		super.OnScheduledTick(deltaTime);

		if (!GetGame().IsServer())
			return;

		if (m_AT_UnlimitedStamina && GetStaminaHandler())
			GetStaminaHandler().SetStamina(GameConstants.STAMINA_MAX);

		if (m_AT_UnlimitedAmmo)
			AT_RefillMagazine();
	}

	protected void AT_RefillMagazine()
	{
		Weapon_Base weapon = Weapon_Base.Cast(GetItemInHands());
		if (!weapon)
			return;

		Magazine magazine = weapon.GetMagazine(weapon.GetCurrentMuzzle());
		if (magazine && magazine.GetAmmoCount() < magazine.GetAmmoMax())
			magazine.ServerSetAmmoCount(magazine.GetAmmoMax());
	}

	void AT_SetGod(bool enabled)
	{
		m_AT_God = enabled;
		SetAllowDamage(!enabled);
	}

	void AT_SetInvisible(bool enabled)
	{
		m_AT_Invisible = enabled;
		SetInvisibleRecursive(enabled, this);
	}
}
