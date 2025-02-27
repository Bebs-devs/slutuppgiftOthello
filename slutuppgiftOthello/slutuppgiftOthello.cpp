#include <iostream>
#include <conio.h>
#include <locale>
#include <vector>
#include <string>
#include <cctype>


struct GameSettings
{
	bool player1iscomp, player2iscomp;
	int computerDifficulty;

	GameSettings() {
		player1iscomp = false;
		player2iscomp = false;
		computerDifficulty = 0;
	}
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

void displayBoard(Board& board, GameSettings &settings ,std::vector<GameCoordinate> = {}) {
	std::string emptySymbol = " .", blackSymbol = " S", whiteSymbol = " V";
	std::cout << "\033[H\n  a b c d e f g h\n";
	for (int rowNumber = 0; rowNumber < 8; ++rowNumber) {
		std::cout << rowNumber + 1;
		char* rowArray = board.boardDiscs[rowNumber];
		for (int columnNumber = 0; columnNumber < 8; ++columnNumber) {
			switch (rowArray[columnNumber])
			{
			case 'a':
				std::cout << emptySymbol;
				break;
			case 'b':
				std::cout << blackSymbol;
				break;
			case 'c':
				std::cout << whiteSymbol;
				break;
			default:
				std::cout << "ERROR";
				return;
			}
		}
		if (rowNumber == 0) std::cout << "  " << board.numberOfBlackDiscs
			<< " - Svart" << (settings.player1iscomp ? " (AI)" : "");
		else if (rowNumber == 1) std::cout << "  " << board.numberOfWhiteDiscs
			<< " - Vit" << (settings.player2iscomp ? " (AI)" : "");
		else if (rowNumber == 3) std::cout <<
			(board.isBlacksTurn ? "  *Svarts tur*" : " *Vits tur");
		std::cout << "\n";
	}
	std::cout << "\n    \033[F";
}

GameCoordinate getPlayerInput() {
	bool invalidInput;
	char letterIn;
	int numberIn, x,y;
	std::cout << "Skriv \"bokstav siffra\" där du vill lägga:\n";
	do {
		std::cin >> letterIn >> numberIn;
		letterIn = tolower(letterIn);
		x = letterIn - 97; //'a' är 97
		y = numberIn - 1; //koden räknar från 0
		if (x < 0 || x > 7 || y < 0 || y > 7) {
			invalidInput = true;
			std::cin.clear();
			std::cin.ignore(INT_MAX, '\n');
			std::cout << "\033[F\033[F";
			std::cout << "Var god mata in giltiga värden.            \n     \n\033[F";
		}
		else invalidInput = false;

	} while (invalidInput);

	return { x,y };
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
	Board gameBoard;
	GameSettings settings;
	settings.computerDifficulty = 0;
	settings.player1iscomp = false;
	settings.player2iscomp = true;
	while (_getch()) {
		displayBoard(gameBoard, settings);
		getPlayerInput();
	}
	return 0;
}