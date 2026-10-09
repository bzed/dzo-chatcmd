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

// Server-side state for the toggle commands. Nothing here is synchronized to clients.
modded class PlayerBase
{
	bool m_AT_God;
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
}
