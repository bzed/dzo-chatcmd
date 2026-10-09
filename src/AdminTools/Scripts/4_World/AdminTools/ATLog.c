// AdminTools - server-side admin chat commands for DayZ
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
