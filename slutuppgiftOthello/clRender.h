#include <vector>
#include <string>
#include "othelloResources.h"
#pragma once


namespace render {
	void updatePossibleMoves(std::vector<GameCoordinates> moves = {});
	void setBoard(Board* board);
	void setSettings(GameSettings* settings);
	void updateSettings();
	void updateBoard();
	void updateAnimations();
	void updateSelectedSquare(GameCoordinates square);
	void updateLastMove(GameCoordinates move);
	void setLastMoveVisible(bool visible);
	void updateDebugText(std::string text);
	void updateComputerProgress(ComputerProgress progress);
	void splashText(std::string, int durationMs = 2000, bool halt = false);
	void init();
}
