#include "clRender.h"
#include <iostream>
#include <vector>
#include <string>
#include <chrono>


using namespace render;

static const std::string ANSI_BOARD_BG = "\033[48;5;28m", ANSI_DEFAULT = "\033[0m";
static const int LEFT_PADDING = 2, TOP_PADDING = 1, RIGHT_PADDING = 1, BOTTOM_PADDING = 0, SCOREBOARD_HEIGHT = 2;
static const std::string NAME_PLAYER_1 = "Svart", NAME_PLAYER_2 = "Vit", NAME_COMPUTER = "AI";

//static std::string mainDisplayState[12]; //index 1-8 för brädet
static std::string boardDisplayState[8][8];
static std::string secondaryDisplayState[12]; //allt här kommer renderas till höger om mainDisplayState
static Board* renderedBoardPtr = nullptr;
static GameSettings* gameSettingsPtr = nullptr;
static GameCoordinates lastMove;
static bool lastMoveVisible = false;
static std::string splash = "";
static std::chrono::time_point<std::chrono::system_clock> splashExpiryTime;


static void print() {
	std::cout << "\033[H"; //gå till "hem-positionen"
	std::cout << std::string(TOP_PADDING, '\n'); //genererar string med längd TOP_PADDING med alla symboler '\n'
	std::cout << std::string(LEFT_PADDING, ' ') + secondaryDisplayState[0] << '\n';
	for (int lineIndex = 1; lineIndex < 9; ++lineIndex) {
		std::cout << std::string(LEFT_PADDING, ' ') + ANSI_BOARD_BG;
		for (int column = 0; column < 8; ++column) {
			std::cout << boardDisplayState[lineIndex-1][column];
		}
		std::cout << ANSI_DEFAULT + std::string(RIGHT_PADDING, ' ') + secondaryDisplayState[lineIndex] << '\n';
	}
	std::cout << std::string(BOTTOM_PADDING, '\n');
	for (int lineIndex = 9; lineIndex < 12; ++lineIndex) {
		std::cout << std::string(LEFT_PADDING, ' ') + secondaryDisplayState[lineIndex] << '\n';
	}
}


void render::init() {
	std::cout << "\033[?25l"; //göm skriv-markören
	secondaryDisplayState[0] = " O T H E L L O";
	for (int lineIndex = 1; lineIndex <= 8; ++lineIndex) {
		secondaryDisplayState[lineIndex] = "";
		for (int column = 0; column < 8; ++column) {
			boardDisplayState[lineIndex-1][column] = "  ";
		}
	}
	secondaryDisplayState[SCOREBOARD_HEIGHT] = "2 - " + NAME_PLAYER_1 + " (" + NAME_COMPUTER + ")";
	secondaryDisplayState[SCOREBOARD_HEIGHT+1] = "2 - " + NAME_PLAYER_2 + " (" + NAME_COMPUTER + ")";

	int longestPlayerName = std::max(NAME_PLAYER_1.length(), NAME_PLAYER_2.length());
	secondaryDisplayState[SCOREBOARD_HEIGHT+3] = '*' + std::string(longestPlayerName, 'x') + "s tur*";
	secondaryDisplayState[9] = "Funkart";
	secondaryDisplayState[10] = "[---------     ]";
	secondaryDisplayState[11] = "123/13948";
	print();
}



void render::setBoard(Board* board)
{
	renderedBoardPtr = board;
}

void render::setSettings(GameSettings* settings)
{
	gameSettingsPtr = settings;
}

void render::setLastMoveVisible(bool visible)
{
	lastMoveVisible = visible;
	updateAnimations();
	print();
}
void render::updateSettings()
{
	if (gameSettingsPtr == nullptr) { updateAnimations(); print(); return; }
	if (gameSettingsPtr->player1iscomp && secondaryDisplayState[SCOREBOARD_HEIGHT].back() != ')') {
		secondaryDisplayState[SCOREBOARD_HEIGHT] += '(' + NAME_COMPUTER + ')';
	}
	else if (!(gameSettingsPtr->player1iscomp) && secondaryDisplayState[SCOREBOARD_HEIGHT].back() == ')') {
		secondaryDisplayState[SCOREBOARD_HEIGHT].
			replace(secondaryDisplayState[SCOREBOARD_HEIGHT].end() - NAME_COMPUTER.length() - 2,
			secondaryDisplayState[SCOREBOARD_HEIGHT].end(), NAME_COMPUTER.length() + 2, ' ');
	}

	if (gameSettingsPtr->player2iscomp && secondaryDisplayState[SCOREBOARD_HEIGHT+1].back() != ')') {
		secondaryDisplayState[SCOREBOARD_HEIGHT+1] += '(' + NAME_COMPUTER + ')';
	}
	else if (!(gameSettingsPtr->player2iscomp) && secondaryDisplayState[SCOREBOARD_HEIGHT+1].back() == ')') {
		secondaryDisplayState[SCOREBOARD_HEIGHT+1].
			replace(secondaryDisplayState[SCOREBOARD_HEIGHT+1].end() - NAME_COMPUTER.length() - 2,
				secondaryDisplayState[SCOREBOARD_HEIGHT+1].end(), NAME_COMPUTER.length() + 2, ' ');
	}

	updateAnimations();
	print();
}

void render::updateBoard()
{
	if (renderedBoardPtr == nullptr) { updateAnimations(); print(); return; }

	static const std::string emptySymbol = "  ",
		blackSymbol = "\033[30m X", whiteSymbol = "\033[37m O",
		highlightBlackSymbol = "\033[33m X", highlightWhiteSymbol = "\033[33m O";

	for (int row = 0; row < 8; ++row) {
		for (int col = 0; col < 8; ++col) {
			switch (renderedBoardPtr->discs[row][col])
			{
			case 'a':
				boardDisplayState[row][col] = emptySymbol;
				break;
			case 'b':
				boardDisplayState[row][col] = blackSymbol;
				break;
			case 'c':
				boardDisplayState[row][col] = whiteSymbol;
				break;		
			default:
				std::cout << "ERROR - ett fel har inträffat och brädet kan inte uppdateras";
				return;
			}
		}
	}

	std::string tPlayerInfo = secondaryDisplayState[SCOREBOARD_HEIGHT].substr(secondaryDisplayState[SCOREBOARD_HEIGHT][1] == ' ' ? 1 : 0);
	secondaryDisplayState[SCOREBOARD_HEIGHT] = std::to_string(renderedBoardPtr->numberOfDiscs[0]) + tPlayerInfo;

	tPlayerInfo = secondaryDisplayState[SCOREBOARD_HEIGHT+1].substr(secondaryDisplayState[SCOREBOARD_HEIGHT][1] == ' ' ? 1 : 0);
	secondaryDisplayState[SCOREBOARD_HEIGHT+1] = std::to_string(renderedBoardPtr->numberOfDiscs[1]) + tPlayerInfo;

	secondaryDisplayState[SCOREBOARD_HEIGHT + 3] = '*'  
		+(renderedBoardPtr->isBlacksTurn ? NAME_PLAYER_1 : NAME_PLAYER_2)  //ex svart eller vit
		+ "s tur*" 
		+ std::string(abs( int(NAME_PLAYER_2.length()-NAME_PLAYER_1.length())), ' '); //oavsett vad måste stringen alltid
	//	vara lika lång. Därför lägger vi till ett antal mellanrum som motsvarar skillnaden mellan namn på spelare 1 och 2

	updateAnimations();
	print();
}

//försvinner vid ändring av brädet //TODO ADD ANIMATIONS
void render::updatePossibleMoves(std::vector<GameCoordinates> moves)
{
	static const std::string possibleSymbol = "\033[38;5;27m +";

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



void render::updateDebugText(std::string text)
{
}

void render::updateComputerProgress(ComputerProgress progress)
{
}

void render::splashText(std::string, int durationMs, bool halt)
{
}
