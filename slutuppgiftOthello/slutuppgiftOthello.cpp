#include <iostream>
#include <locale>
#include <vector>

struct GameSettings
{
	bool isPlayingAgainstComputer;
	bool isComputerBlack = false;
	int computerDifficulty = 0;
};

struct GameCoordinate
{
	int x;
	int y;
};

struct Board
{
	bool isBlacksTurn;
	int numberOfBlackDiscs;
	int numberOfWhiteDiscs;
	//positioner
};

std::vector<GameCoordinate> searchPossibleMoves(Board &board) {
	std::vector<GameCoordinate> listOfPossibleMoves;
	return listOfPossibleMoves;
}

bool placeDisc(Board& board, GameCoordinate) {
	return false;
}

void displayBoard(Board& board, std::vector<GameCoordinate> = {}) {

}

GameCoordinate getPlayerInput(bool isBlacksTurn) {
	return { 3,5 };
}

//[6]
void initGameSettings(GameSettings& settings) {

}

//[7]
bool endGame(Board& board) {
	return false;
}

//[8]
GameCoordinate chooseComputerMove(Board& board, std::vector<GameCoordinate>, int computerDifficulty) {

}



int main() {
	setlocale(LC_ALL, "sv_SE");
	std::cout << "hej ugituiåäöehf";
	return 0;
}