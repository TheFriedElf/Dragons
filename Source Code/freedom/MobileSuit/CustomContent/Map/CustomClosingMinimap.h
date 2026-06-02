#pragma once

class CustomClosingMinimap
{
public:
	static bool IsDungeonMinimapAutoShowEnabled();
	static void SetDungeonMinimapAutoShowEnabled(bool bEnabled);
	static void TryHideMinimapIfAlreadyOpened();
};