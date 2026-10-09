// Logging: everything goes to the script log, admin actions also to $profile:AdminTools/audit.log.
class ATLog
{
	static const string TAG = "[AdminTools] ";

	static void Info(string msg)
	{
		Print(TAG + msg);
	}

	static void Debug(string msg)
	{
		if (ATConfig.Get().verbose)
			Print(TAG + "DEBUG " + msg);
	}

	// Who did what. Never sent to players.
	static void Audit(PlayerBase admin, string action)
	{
		string who = "?";
		if (admin && admin.GetIdentity())
			who = admin.GetIdentity().GetName() + " (" + admin.GetIdentity().GetId() + ")";

		string line = ATUtil.Timestamp() + " " + who + ": " + action;
		Print(TAG + "AUDIT " + line);

		if (!ATConfig.Get().auditLog)
			return;

		FileHandle fh = OpenFile(ATConfig.AUDIT_FILE, FileMode.APPEND);
		if (fh != 0)
		{
			FPrintln(fh, line);
			CloseFile(fh);
		}
	}
}
