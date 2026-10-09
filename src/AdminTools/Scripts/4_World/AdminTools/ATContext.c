// One parsed command invocation. All output goes through Reply(), which only reaches the sending admin.
class ATContext
{
	PlayerBase admin;
	string command;
	ref array<string> args = new array<string>;

	string Arg(int index)
	{
		if (index < args.Count())
			return args[index];
		return "";
	}

	// Everything from argument <index> on, joined (player and location names may contain spaces)
	string Rest(int index)
	{
		string text = ATUtil.Join(" ", args, index);
		text.TrimInPlace();
		return text;
	}

	void Reply(string text)
	{
		admin.MessageStatus("[AdminTools] " + text);
	}

	// Long lists are split into several chat lines
	void ReplyList(string header, array<string> items)
	{
		string line = header;
		bool lineHasItem = false;
		foreach (string item : items)
		{
			if (lineHasItem && line.Length() + item.Length() > 160)
			{
				Reply(line);
				line = "  ";
				lineHasItem = false;
			}
			if (lineHasItem)
				line += ", ";
			line += item;
			lineHasItem = true;
		}
		Reply(line);
	}

	void Audit(string action)
	{
		ATLog.Audit(admin, action);
	}

	// Tells <target> what happened to them - only if notifyTargets is enabled and it is somebody else
	void NotifyTarget(PlayerBase target, string text)
	{
		if (target != admin && ATConfig.Get().notifyTargets)
			target.MessageStatus("[AdminTools] " + text);
	}

	// Resolves a player name (case-insensitive, partial allowed). Tells the admin if nothing / too much matched.
	PlayerBase FindPlayer(string query)
	{
		if (query.Length() == 0)
			return null;

		array<Man> men = new array<Man>;
		GetGame().GetPlayers(men);

		array<PlayerBase> players = new array<PlayerBase>;
		array<string> names = new array<string>;
		foreach (Man man : men)
		{
			PlayerBase pb = PlayerBase.Cast(man);
			if (pb && pb.GetIdentity())
			{
				players.Insert(pb);
				names.Insert(pb.GetIdentity().GetName());
			}
		}

		array<int> hits;
		ATUtil.MatchIndices(names, query, hits);
		if (hits.Count() == 1)
			return players[hits[0]];

		if (hits.Count() == 0)
		{
			Reply("No player matches '" + query + "'.");
			return null;
		}

		array<string> candidates = new array<string>;
		foreach (int idx : hits)
			candidates.Insert(names[idx]);
		ReplyList("'" + query + "' is ambiguous: ", candidates);
		return null;
	}

	// Resolves a key of a name list (locations, groups, vehicles). Same rules as FindPlayer.
	string FindName(array<string> names, string query, string what)
	{
		array<int> hits;
		ATUtil.MatchIndices(names, query, hits);
		if (hits.Count() == 1)
			return names[hits[0]];

		if (hits.Count() == 0)
		{
			Reply("Unknown " + what + " '" + query + "'.");
			return "";
		}

		array<string> candidates = new array<string>;
		foreach (int idx : hits)
			candidates.Insert(names[idx]);
		ReplyList("'" + query + "' is ambiguous: ", candidates);
		return "";
	}
}
