#include <iostream>
#include <locale>
#include <vector>


struct GameSettings
{
	bool player1iscomp, player2iscomp;
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
	int numberOfBlackDiscs,numberOfWhiteDiscs;
	char boardDiscs[8][8] = {0}; //a tom, b svart, c vit
	bool boardAdjacents[8][8] = {0}; //är ruta bredvid en ruta med bricka i.

	Board() {
		isBlacksTurn = true;
		numberOfBlackDiscs = numberOfWhiteDiscs = 2;
		for (int i = 0; i < 8; ++i) {
			for (int j = 0; j < 8; ++j) {
				boardDiscs[i][j] = 'a';
			}
		}
		boardDiscs[3][3] = 'c';
		boardDiscs[3][4] = 'b';
		boardDiscs[4][3] = 'b';
		boardDiscs[4][4] = 'c';
		boardAdjacents[2][3] = true;
		boardAdjacents[3][2] = true;
		boardAdjacents[3][5] = true;
		boardAdjacents[2][4] = true;
		boardAdjacents[4][2] = true;
		boardAdjacents[4][5] = true;
		boardAdjacents[5][3] = true;
		boardAdjacents[5][4] = true;
	}
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
	return { 0,0 };
}



int main() {
	setlocale(LC_ALL, "sv_SE");
	std::cout << "hej ugituiåäöehf";
	return 0;
}