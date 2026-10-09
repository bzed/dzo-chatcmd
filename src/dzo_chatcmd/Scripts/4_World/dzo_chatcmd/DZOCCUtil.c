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

class DZOCCUtil
{
	static string Join(string separator, array<string> items, int from = 0)
	{
		string result = "";
		for (int i = from; i < items.Count(); i++)
		{
			if (i > from)
				result += separator;
			result += items[i];
		}
		return result;
	}

	static string Pad2(int value)
	{
		if (value < 10)
			return "0" + value;
		return value.ToString();
	}

	static string Timestamp()
	{
		int year, month, day, hour, minute, second;
		GetYearMonthDay(year, month, day);
		GetHourMinuteSecond(hour, minute, second);
		return year.ToString() + "-" + Pad2(month) + "-" + Pad2(day) + " " + Pad2(hour) + ":" + Pad2(minute) + ":" + Pad2(second);
	}

	// "12", "-3.5", "8090.49" - string.ToFloat() returns 0 for garbage, so check first
	static bool IsNumeric(string s)
	{
		if (s.Length() == 0)
			return false;

		bool digits = false;
		bool dot = false;
		for (int i = 0; i < s.Length(); i++)
		{
			string c = s.Get(i);
			if (c == "-" && i == 0)
				continue;
			if (c == "." && !dot)
			{
				dot = true;
				continue;
			}
			if (c.ToInt() == 0 && c != "0")
				return false;
			digits = true;
		}
		return digits;
	}

	// Case-insensitive lookup in a name list: an exact match wins, otherwise every name containing the query.
	static void MatchIndices(array<string> names, string query, out array<int> hits)
	{
		hits = new array<int>;
		string q = query;
		q.ToLower();

		int i;
		string lower;
		for (i = 0; i < names.Count(); i++)
		{
			lower = names[i];
			lower.ToLower();
			if (lower == q)
				hits.Insert(i);
		}
		if (hits.Count() > 0)
			return;

		for (i = 0; i < names.Count(); i++)
		{
			lower = names[i];
			lower.ToLower();
			if (lower.Contains(q))
				hits.Insert(i);
		}
	}

	// Ground height at x/z; y == 0 means "find the surface from above" (from COT)
	static vector SnapToGround(vector pos)
	{
		float y;
		if (pos[1] == 0.0)
			y = GetGame().SurfaceRoadY3D(pos[0], 1000.0, pos[2], RoadSurfaceDetection.UNDER);
		else
			y = GetGame().SurfaceRoadY3D(pos[0], pos[1], pos[2], RoadSurfaceDetection.CLOSEST);

		vector result = Vector(pos[0], y, pos[2]);
		float waterDepth = GetGame().GetWaterDepth(result);
		if (waterDepth > 0)
			result[1] = result[1] + waterDepth;
		return result;
	}

	static string FormatPos(vector pos)
	{
		return Math.Round(pos[0]).ToString() + " " + Math.Round(pos[2]).ToString() + " (h " + Math.Round(pos[1]).ToString() + ")";
	}

	static bool ClassExists(string className)
	{
		if (className.Length() == 0)
			return false;
		return GetGame().ConfigIsExisting("CfgVehicles " + className) || GetGame().ConfigIsExisting("CfgWeapons " + className) || GetGame().ConfigIsExisting("CfgMagazines " + className);
	}
}
