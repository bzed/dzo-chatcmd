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

modded class MissionServer
{
	override void OnInit()
	{
		super.OnInit();
		ATConfig.Load();
	}

	override void InvokeOnConnect(PlayerBase player, PlayerIdentity identity)
	{
		super.InvokeOnConnect(player, identity);

		if (player && ATConfig.Get().godModeOnLogin && ATConfig.IsAdmin(identity))
		{
			player.AT_SetGod(true);
			ATLog.Audit(player, "god mode enabled on login");
		}
	}

	override void OnEvent(EventType eventTypeId, Param params)
	{
		super.OnEvent(eventTypeId, params);

		if (eventTypeId != ChatMessageEventTypeID)
			return;

		ChatMessageEventParams chat = ChatMessageEventParams.Cast(params);
		if (!chat)
			return;

		string text = chat.param3;
		if (text.Length() < 2 || text.Get(0) != "/")
			return;

		PlayerBase sender = AT_FindSender(chat.param2);
		if (sender)
			ATCommands.Handle(sender, text);
	}

	// The chat event only carries the display name. If two players share it we cannot tell who typed the
	// command (and a non-admin could impersonate an admin by copying the name), so we refuse.
	protected PlayerBase AT_FindSender(string name)
	{
		array<Man> men = new array<Man>;
		GetGame().GetPlayers(men);

		PlayerBase found;
		foreach (Man man : men)
		{
			PlayerBase pb = PlayerBase.Cast(man);
			if (!pb || !pb.GetIdentity() || pb.GetIdentity().GetName() != name)
				continue;

			if (found)
			{
				ATLog.Info("Ignoring chat command: several players are named '" + name + "'");
				return null;
			}
			found = pb;
		}
		return found;
	}
}
