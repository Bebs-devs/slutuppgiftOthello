#include <vector>
#include <string>
#include "othelloResources.h"
#pragma once


namespace render {
	
	void setBoard(Board* board);
	void setSettings(GameSettings* settings);
	void updateSettings(bool updateScreen = true);
	void updateBoard(bool updateScreen = true);
	void updatePossibleMoves(std::vector<GameCoordinates> moves = {}, bool updateScreen = true);
	void updateScreenAndAnimations();
	void updateSelectedSquare(GameCoordinates square);
	void updateLastMove(GameCoordinates move, bool updateScreen = true);
	void updateDebugText(std::string text, bool updateScreen = true);
	void updateComputerProgress(ComputerProgress progress);
	void splashText(std::string text, int durationMs = 2000, bool returnWhenFinished = false);
	void restoreScreen();
	void init();
}
