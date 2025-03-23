#include "clRender.h"
#include <iostream>
#include <vector>
#include <string>
#include <chrono>
using namespace render;


static std::string displayState[12]; //index 1-8 för brädet
static Board* renderedBoardPtr;
static GameSettings* gameSettingsPtr;
static GameCoordinates lastMove;
static bool lastMoveVisible;
static std::chrono::time_point<std::chrono::system_clock> splashExpiryTime;

static void print() {
	std::cout << "\033[H";
	for (int i = 0; i < 12; ++i) {
		std::cout << displayState[i] << '\n';
	}
}


void render::init() {
	std::cout << "\033[?25l"; //göm skriv-markören
	displayState[0] = " O T H E L L O";
	for (int i = 1; i <= 8; ++i) {
		displayState[i] = "\033[48;5;28m                \033[0m";
	}
	displayState[2] += " 2 - Svart";
	displayState[3] += " 2 - Vit";

	displayState[5] += " *Svarts Tur*";
	displayState[9] = "Funkart";
	displayState[10] = "[---------     ]";
	displayState[11] = "123/13948";
	print();
}

//försvinner vid ändring av brädet //TODO ADD ANIMATIONS
void render::updatePossibleMoves(std::vector<GameCoordinates> moves)
{
}

void render::setBoard(Board* board)
{
}

void render::setSettings(GameSettings* settings)
{
}

void render::updateSettings()
{
}

void render::updateBoard()
{
}

void render::updateAnimations()
{
}

void render::updateSelectedSquare(GameCoordinates square)
{
}

void render::updateLastMove(GameCoordinates move)
{
}

void render::setLastMoveVisible(bool visible)
{
}

void render::updateDebugText(std::string text)
{
}

void render::updateComputerProgress(ComputerProgress progress)
{
}

void render::splashText(std::string, int durationMs, bool halt)
{
}
