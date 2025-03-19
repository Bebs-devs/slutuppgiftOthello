#include <iostream>
#include <conio.h>
#include <locale>
#include <vector>
#include <string>
#include <cctype> //för isAlpha och isSpace
#include <chrono>
#include <thread>
#include <ctime>

typedef signed char int8;

struct GameSettings
{
	bool player1iscomp, player2iscomp;
	int comp1Difficulty, comp2Difficulty;

	GameSettings() {
		player1iscomp = false;
		player2iscomp = false;
		comp1Difficulty = 0;
		comp2Difficulty = 0;
	}
};

struct GameCoordinates
{
	static const int8 translations[8][2];
	int y;
	int x;

	bool isInValid() {
		if (x < 0 || x > 7 || y < 0 || y > 7) return true;
		return false;
	}
};
const int8 GameCoordinates::translations[8][2] = {
	  {-1,-1},{-1,0 },{-1,1 },{0,-1 },{0,1 },{1,-1 },{1,0 },{1,1 } };

struct Board
{
	bool isBlacksTurn;
	int numberOfDiscs[2] = { 0, 0 }; //{antal svarta brickor, antal vita brickor}
	char discs[8][8] = {0}; //a tom, b svart, c vit
	//[y][x] gäller för alla arrayer som håller positioner
	bool adjacents[8][8] = {0}; //är ruta bredvid en ruta med bricka i.

	Board() {
		//den här funktionen körs när ett objekt av typ (klass) Board skapas

		isBlacksTurn = true; //svart börjar alltid

		//antal svarta brickor = antal vita brickor = 2
		numberOfDiscs[0] = numberOfDiscs[1] = 2;

		//iterera alla rutor och sätt deras status till tom ('a')
		for (int i = 0; i < 8; ++i) {
			for (int j = 0; j < 8; ++j) {
				discs[i][j] = 'a';
			}
		}
		//placera startbrickorna      ...
		discs[3][3] = 'c'; //... a a ...
		discs[3][4] = 'b'; //..a c b a..
		discs[4][3] = 'b'; //..a b c a..
		discs[4][4] = 'c'; //... a a ...
		                        //    ...
		//specifiera de rutor som är bredvid en ruta med en bricka i
		adjacents[2][2] = true;
		adjacents[2][3] = true;//    ...
		adjacents[2][4] = true;//..1 1 1 1..
		adjacents[2][5] = true;//..1 0 0 1..
		adjacents[3][2] = true;//..1 0 0 1..
		adjacents[3][5] = true;//..1 1 1 1..
		adjacents[4][2] = true;//    ...
		adjacents[4][5] = true;
		adjacents[5][2] = true;
		adjacents[5][3] = true;
		adjacents[5][4] = true;
		adjacents[5][5] = true;
	}

	bool countDiscs(){
		int black = 0, white = 0;
		for (int i = 0; i < 8; ++i) {
			for (int j = 0; j < 8; j++) {
				if (discs[i][j] == 'b') black++;
				else if (discs[i][j] == 'c') white++;
			}
		}
		if (numberOfDiscs[0] != black || numberOfDiscs[1] != white) {
			std::cout << "ALARM ALARM";
			return true;
		}
		return false;
	}
};

//[2.1]
bool isValidMove(Board & board, GameCoordinates move) {
	//relativ färg till färg på det drag som kollas
	char sameColor = board.isBlacksTurn ? 'b' : 'c';
	char otherColor = board.isBlacksTurn ? 'c' : 'b';

	//iterera alla riktningar
	for (int i = 0; i < 8; ++i) {
		//gå ett steg i riktningen
		GameCoordinates currCoords = { move.y + GameCoordinates::translations[i][0],
			move.x + GameCoordinates::translations[i][1] };

		//gå till nästa riktning om vi är utanför spelplanen
		if (currCoords.isInValid()) continue;
		//eller om INTE motsatt färg (alltså tom eller samma)
		if (board.discs[currCoords.y][currCoords.x] != otherColor) continue;

		/*har vi kommit så här långt har vi en bricka med motsatt färg
		för att utföra ett drag krävs också en bricka med samma färg i 
		slutet av kedjan med brickor i motsatt färg. 
		*/
		while (true) {
			//gå ett steg i riktningen
			currCoords = { currCoords.y + GameCoordinates::translations[i][0],
			currCoords.x + GameCoordinates::translations[i][1] };

			//fortsätt till nästa riktning om vi är utanför spelplanen
			if (currCoords.isInValid()) break;
			//eller på en tom ruta
			if (board.discs[currCoords.y][currCoords.x] == 'a') break;
			 
			//hittar vi däremot en bricka med samma färg är draget giltigt
			if (board.discs[currCoords.y][currCoords.x] == sameColor) return true;

			//om det är motsatt färg fortsätter vi helt enkelt
		}
		
	}
	/*om vi har kollat alla riktningar utan att förutsättningarna för
	ett giltigt drag mötts*/
	return false;
}

//[2]
std::vector<GameCoordinates> getListOfPossibleMoves(Board &board) {
	std::vector<GameCoordinates> listOfPossibleMoves;
	for (int y = 0; y < 8; ++y) {
		for (int x = 0; x < 8; ++x) {
			if (!board.adjacents[y][x]) continue;
			if (isValidMove(board, { y,x })) listOfPossibleMoves.push_back({ y,x });
		}
	}
	return listOfPossibleMoves;
}

//[3]
void placeDisc(Board& board, GameCoordinates coords) {
	//relativ färg till färg på det drag som kollas
	char sameColor = board.isBlacksTurn ? 'b' : 'c';
	char otherColor = board.isBlacksTurn ? 'c' : 'b';

	board.discs[coords.y][coords.x] = sameColor;
	board.numberOfDiscs[board.isBlacksTurn ? 0 : 1] += 1;

	board.adjacents[coords.y][coords.x] = false;
	for (int i = 0; i < 8; ++i) {
		GameCoordinates adjCoords = { coords.y + GameCoordinates::translations[i][0], 
			coords.x + GameCoordinates::translations[i][1] };
		if (adjCoords.isInValid()) continue;
		if (board.discs[adjCoords.y][adjCoords.x] > 'a') continue;
		board.adjacents[adjCoords.y][adjCoords.x] = true;
	}

	

	//iterera alla riktningar
	for (int i = 0; i < 8; ++i) {
		bool flipConfirmed = false;
		std::vector<GameCoordinates> squaresToBeFlipped = {};
		////gå ett steg i riktningen
		//GameCoordinates currCoords = { coords.y + GameCoordinates::translations[i][0],
		//	coords.x + GameCoordinates::translations[i][1] };

		////gå till nästa riktning om vi är utanför spelplanen
		//if (currCoords.isInValid()) continue;
		////eller om INTE motsatt färg (alltså tom eller samma)
		//if (board.discs[currCoords.y][currCoords.x] != otherColor) continue;

		///*har vi kommit så här långt har vi en bricka med motsatt färg
		//för att utföra ett drag krävs också en bricka med samma färg i
		//slutet av kedjan med brickor i motsatt färg.
		//*/
		GameCoordinates currCoords = coords;
		while (true) {
			//gå ett steg i riktningen
			currCoords = { currCoords.y + GameCoordinates::translations[i][0],
			currCoords.x + GameCoordinates::translations[i][1] };

			//fortsätt till nästa riktning om vi är utanför spelplanen
			if (currCoords.isInValid()) break;
			//eller på en tom ruta
			if (board.discs[currCoords.y][currCoords.x] == 'a') break;

			//hittar vi däremot en bricka med samma färg kan vi flippa (förutsatt att det finns något att flippa)
			if (board.discs[currCoords.y][currCoords.x] == sameColor) {
				if (!squaresToBeFlipped.empty()) flipConfirmed = true;
				break;
			}

			//om det är motsatt färg lägger vi till
			squaresToBeFlipped.push_back(currCoords);
		}
		//om vi har bekräftat vändningen flippar vi rutorna 
		if (flipConfirmed) {
			for (GameCoordinates square : squaresToBeFlipped) {
				board.discs[square.y][square.x] = sameColor;
				board.numberOfDiscs[board.isBlacksTurn ? 0 : 1]++;
				board.numberOfDiscs[board.isBlacksTurn ? 1 : 0]--;
			}
		}


	}
	board.isBlacksTurn = board.isBlacksTurn ? false : true;
}

//[4]
void displayBoard(Board& board, GameSettings& settings, std::vector<GameCoordinates> movesOverlay = { }, GameCoordinates highlighted = {9,9}) {
	//symboler som ska skrivas ut
	std::string emptySymbol = "  ", 
		blackSymbol = "\033[30m X", whiteSymbol = "\033[37m O", 
		possibleSymbol = "\033[38;5;27m +", 
		highlightBlackSymbol = "\033[33m X", highlightWhiteSymbol = "\033[33m O";

	//skriv ut brädets norra kant
	std::cout << "\033[2J\033[H\n  a b c d e f g h\n";

	//håll koll på vart vi är i listan över möjliga drag
	int overlayCounter = 0; 
	//iterera alla rader
	for (int rowNumber = 0; rowNumber < 8; ++rowNumber) {

		std::cout << rowNumber + 1; //skriv ut radnummret (+1 för 1-8 istället för 0-7)
		std::cout << "\033[48;5;28m";

		char* rowArray = board.discs[rowNumber]; //spara alla rutor på raden i en temp-variabel

		//iterera genom raden
		for (int columnNumber = 0; columnNumber < 8; ++columnNumber) {
			
			//OVERLAY AV MÖJLIGA DRAG
			//vi förutsätter att listan över möjliga drag är sorterad
			//om rutan vi är på är ett möjligt drag
			if (!movesOverlay.empty() && movesOverlay[overlayCounter].y == rowNumber && movesOverlay[overlayCounter].x == columnNumber) {
				//inkrementera räknaren men inte om vi har nått slutet på listan
				overlayCounter += overlayCounter<movesOverlay.size()-1 ? 1 : 0;
				//skriv ut symbolen som representerar ett möjligt drag
				std::cout << possibleSymbol;
				//gå direkt till nästa ruta (vi vill ju inte skriva ut dubbelt)
				continue;
			}
			bool highlightPosition = false;
			if (highlighted.y == rowNumber && highlighted.x == columnNumber) {
				highlightPosition = true;
			}

			//kolla vad rutan har för status
			switch (rowArray[columnNumber])
			{
			case 'a': //om tom
				std::cout << emptySymbol;
				break;
			case 'b': //om svart
				std::cout << (highlightPosition ? highlightBlackSymbol : blackSymbol);
				break;
			case 'c': //om vit
				std::cout << (highlightPosition ? highlightWhiteSymbol : whiteSymbol);
				break;
			default: //om annat, vilket endast kan betyda fel
				std::cout << "ERROR";
				return;
			}
		}

		/*till höger om brädet står vissa detaljer. Eftersom dessa skrivs ut
		på specifika rader, kollar vi här efter rader 1, 2 och 4 (från användarens perspektiv)*/
		std::cout << "\033[0m";
		//spelare 1 (svart) - antal brickor och spelartyp
		if (rowNumber == 0) std::cout << "  " << board.numberOfDiscs[0]
			<< " - Svart" << (settings.player1iscomp ? " (AI)" : "");
		//spelare 2 (vit)
		else if (rowNumber == 1) std::cout << "  " << board.numberOfDiscs[1]
			<< " - Vit" << (settings.player2iscomp ? " (AI)" : "");
		//vems tur det är
		else if (rowNumber == 3) std::cout <<
			(board.isBlacksTurn ? "  *Svarts tur*" : " *Vits tur*");
		
		//nu har vi skrivit klart raden och kan gå till nästa
		std::cout << "\n";
	}
	std::cout << "\n    \033[F";
}

//[5]
GameCoordinates getValidPlayerInput(Board & board, GameSettings settings, std::vector<GameCoordinates> movesOverlay = { {9,9} }) {
	bool invalidInput; //är inputen "trasig"?
	std::string inputLine; //spara rå input

	//del av inputen som är bokstaven respektive siffran
	char letterIn = 0, numberIn = 0; 

	GameCoordinates finalInput; //de bearbetade koordinaterna

	do { //vi ska kolla input minst en gång men fler ifall ogiltig input ges
		
		//prompta användaren
		std::cout << "Skriv \"bokstav siffra\" där du vill lägga:\n";

		letterIn = 0; //måste nollställas för att kunna ta bearbeta input
		invalidInput = false; //innan vi har input får vi anta att den är giltig

		std::getline(std::cin, inputLine); //erhåll input i dess råaste form
		for (char c : inputLine) { //iterera alla chars i inputen
			if (isspace(c)) continue; //skippa mellanrum

			//om vi inte redan har en bokstav och detta är en bokstav
			if (letterIn == 0 && isalpha(c)) letterIn = c;
			//om vi har redan har en bokstav och detta är en siffra
			else if (letterIn != 0 && isdigit(c)) numberIn = c;
			//om vi kommer hit är inputen inte giltig
			else invalidInput = true;
		}
		
		//bearbeta inputen till index-form
		finalInput.x = tolower(letterIn) - 97; //'a' är 97
		finalInput.y = numberIn - 49; //'1' är 49 index börjar på 0
		
		//om ogiltig input eller koordinater utanför spelplanen
		if (invalidInput || finalInput.isInValid()) {
			invalidInput = true;

			displayBoard(board, settings, movesOverlay); //Skriv ut brädet igen
			std::cout << "Inmatning måste vara en koordinat på brädet (ex. f2)\n";
		}
		//om ogiltigt drag (men giltig input i övrigt)
		else if (!isValidMove(board, finalInput)) {
			invalidInput = true;

			displayBoard(board, settings, movesOverlay); //Skriv ut brädet igen
			std::cout << "Du kan tyvärr inte lägga där\n";
		}
	} while (invalidInput); //om ogitlig input, hoppa upp igen

	return finalInput; //returnera bearbetade, gilitga koordinater.
}

//[6.1]
int simpleSelectionChoice(std::string header, std::vector<std::string> options) {
	int keyPress;
	int selection = 0;
	std::cout << "\033[2J";
	while (true) {
		std::cout << "\033[H" << header << '\n';
		for (int i = 0; i < options.size(); ++i) {
			if (i == selection)
				std::cout << "> " << options[i] << " <         \n";
			else std::cout << "  " << options[i] << "           \n";
		}
		keyPress = _getch();
		if (keyPress == 13) break;
		if (keyPress != 224) continue;

		keyPress = _getch();
		if (keyPress == 80) { //ned-pil
			selection += selection < options.size() - 1 ? 1 : 0;
		}
		else if (keyPress == 72) { //upp-pil
			selection -= selection > 0 ? 1 : 0;
		}
		
	}
	return selection;
}

//[6]
void initGameSettings(GameSettings& settings) {
	const std::vector<std::string> optionsPlayer1 = { "Svart ska spelas av människa", "Svart ska spelas av dator" };
	const std::vector<std::string> optionsPlayer2 = { "Vit ska spelas av människa", "Vit ska spelas av dator" };
	const std::vector<std::string> optionsDifficulty = { "0 - Slumpade drag", "1 - Enkel", "2 - Medel", "3 - Svår", "4 - Omöjlig", "5", "6", "7"};

	settings.player1iscomp = (bool)simpleSelectionChoice("\n Välkommen till Othello\nSvart börjar spelet. Välj alternativ för svart:\n",optionsPlayer1);
	
	if (settings.player1iscomp) {
		
		settings.comp1Difficulty = simpleSelectionChoice("\n Välommen till Othello\nSvart spelas av en dator, vilken svårighetsgrad?\n", optionsDifficulty);
	}

	settings.player2iscomp = (bool)simpleSelectionChoice("\n Välkommen till Othello\nVit fortsätter spelet. Välj alternativ för vit:\n", optionsPlayer2);

	if (settings.player2iscomp) {

		settings.comp2Difficulty = simpleSelectionChoice("\n Välkommen till Othello\nVit spelas av en dator, vilken svårighetsgrad?\n", optionsDifficulty);
	}
}

//[7]
bool endGame(Board& board, GameSettings& settings) {
	static int blackWins = 0, whiteWins = 0, ties = 0;
	std::cout << "SPELET ÄR ÖVER\n";
	if (board.numberOfDiscs[0] == board.numberOfDiscs[1]) {
		std::cout << "Se där, det blev lika!\n";
		++ties;
	}
	else if (board.numberOfDiscs[0] > board.numberOfDiscs[1]) {
		std::cout << "Grattis"<< (settings.player1iscomp ? " datorn" : " spelare") << ", SVART vann\n";
		++blackWins;
	}
	else {
		std::cout << "Grattis" << (settings.player2iscomp ? " datorn" : " spelare") << ", VIT vann\n";
		++whiteWins;
	}
	std::cout << "Resultat: \n";
	std::cout << "Svart: " << board.numberOfDiscs[0] << " brickor\n";
	std::cout << "Vit  : " << board.numberOfDiscs[1] << " brickor\n";
	std::cout << "Ställning under programmet körtid:\n";
	std::cout << "Svart " << blackWins << " - " << whiteWins << " Vit\n";
	return false;
}

int evaluatePosition(Board& board){
	return board.numberOfDiscs[0];
}

int minMax(Board& board,int depth){
	if (depth == 0) {
		return evaluatePosition(board);
	}
	std::vector<GameCoordinates> possibleMoves = getListOfPossibleMoves(board);

	if (board.isBlacksTurn) {
		int highestEval = -INT_MAX;
		for (int i = 0; i < possibleMoves.size(); ++i) {
			Board newBoard = board;
			placeDisc(newBoard, possibleMoves[i]);
			int eval = minMax(newBoard, depth-1);
			if (eval > highestEval) {
				highestEval = eval;
			}
		}
		return highestEval;
	}
	else {
		int lowestEval = INT_MAX;
		for (int i = 0; i < possibleMoves.size(); ++i) {
			Board newBoard = board;
			placeDisc(newBoard, possibleMoves[i]);
			int eval = minMax(newBoard, depth-1);
			if (eval < lowestEval) {
				lowestEval = eval;
			}
		}
		return lowestEval;
	}
}

//[8]
GameCoordinates chooseComputerMove(Board& board, int computerDifficulty) {
	std::vector<GameCoordinates> possibleMoves = getListOfPossibleMoves(board);
	int searchDepth = computerDifficulty, bestIndex = -1;

	if (board.isBlacksTurn) {
		int highestEval = -INT_MAX;
		for (int i = 0; i < possibleMoves.size(); ++i) {
			Board newBoard = board;
			placeDisc(newBoard, possibleMoves[i]);
			int eval = minMax(newBoard, searchDepth);
			if (eval >= highestEval) {
				highestEval = eval;
				bestIndex = i;
			}
		}
	}
	else {
		int lowestEval = INT_MAX;
		for (int i = 0; i < possibleMoves.size(); ++i) {
			Board newBoard = board;
			placeDisc(newBoard, possibleMoves[i]);
			int eval = minMax(newBoard, searchDepth);
			if (eval <= lowestEval) {
				lowestEval = eval;
				bestIndex = i;
			}
		}
	}

	return possibleMoves[bestIndex];
}

//[9]
bool makePlayerMove(Board& board, GameSettings settings) {
	std::vector<GameCoordinates> possibleMoves = getListOfPossibleMoves(board);
	displayBoard(board, settings, possibleMoves);
	if (possibleMoves.empty()) {
		std::cout << "Du har inga möjliga drag, turen går över till nästa spelare\n";
		board.isBlacksTurn = board.isBlacksTurn ? false : true;
		std::this_thread::sleep_for(std::chrono::seconds(1));
		return false;
	}

	GameCoordinates coords = getValidPlayerInput(board, settings, possibleMoves);
	placeDisc(board, coords);
	return true;
}

bool makeComputerMove(Board& board, GameSettings settings, int difficulty = 0) {
	std::vector<GameCoordinates> possibleMoves = getListOfPossibleMoves(board);
	
	if (possibleMoves.empty()) {
		std::cout << "Du har inga möjliga drag, turen går över till nästa spelare\n";
		board.isBlacksTurn = board.isBlacksTurn ? false : true;
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
		return false;
	}
	
	int moveIndex = rand() % possibleMoves.size();
	GameCoordinates coords = possibleMoves[moveIndex];
	placeDisc(board, chooseComputerMove(board,difficulty));
	//possibleMoves.erase(possibleMoves.begin() + moveIndex);
	std::cout << "Lade: " << char(coords.x+97) << " " << coords.y+1;
	displayBoard(board, settings, {}, {coords});
	std::this_thread::sleep_for(std::chrono::milliseconds(10));
	return true;
}


int main() {
	srand(time(0));
	setlocale(LC_ALL, "sv_SE");
	Board gameBoard;
	GameSettings settings;
	settings.player1iscomp = true;
	settings.player2iscomp = true;
	initGameSettings(settings);
	while (true) {
		gameBoard = Board();
		bool player1successful = true, player2successful = true;
		while (player1successful || player2successful) {
			if (gameBoard.countDiscs()) break; //DEBUG
			player1successful = settings.player1iscomp ? makeComputerMove(gameBoard, settings, settings.comp1Difficulty) : makePlayerMove(gameBoard, settings);
			player2successful = settings.player2iscomp ? makeComputerMove(gameBoard, settings, settings.comp2Difficulty) : makePlayerMove(gameBoard, settings);
		}
		endGame(gameBoard, settings);
		std::cout << "Vill du köra igen med samma inställningar? Klicka ENTER.\nVill du avsluta? Klicka valfri tangent\n";
		if (_getch() != 13) break; //13 är koden för enter

	}
	return 0;
}