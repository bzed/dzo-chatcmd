modded class MissionServer
{
	override void OnInit()
	{
		super.OnInit();
		ATConfig.Load();
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
