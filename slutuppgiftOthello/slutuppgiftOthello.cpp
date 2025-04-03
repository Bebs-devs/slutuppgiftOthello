﻿
//Copyright Hugo TD 2025

#include <iostream> //cin cout
#include <conio.h> //getch kbhit
#include <locale> //utskrift av åäö
#include <vector> //std::vector
#include <string> //std::string
#include <cctype> //för isAlpha och isSpace
#include <chrono> //allt med tid
#include <ctime> //för initialisering av rand
#include "clRender.h" //funktioner som skriver ut saker på skärmen
#include "othelloResources.h" //structs och en typedef som används av hela programmet

//[A.a] refererar till planeringen
//koordinater är i formen {y, x}, där {0, 0} är vänstra översta hörnet

//[2.1] kollar om ett drag är giltigt på ett specifikt bräde
bool isValidMove(Board & board, GameCoordinates move) {
	//relativ färg jämfört med vems tur det är
	char sameColor = board.isBlacksTurn ? 'b' : 'c'; //ex. om det är svarts tur kommer sameColor = 'b' , alltså svart
	char otherColor = board.isBlacksTurn ? 'c' : 'b'; //tvärtom

	//kolla om rutan är ockuperad, isåfall går det ju inte att lägga där
	if (board.discs[move.y][move.x] != 'a') return false; 

	//iterera alla riktningar (kardinala riktningar, samt diagonaler)
	for (int i = 0; i < 8; ++i) {
		//gå ett steg i riktningen. //translations är relative koordinater för varje riktning (ex. {1,0} för ned)
		GameCoordinates currCoords = { move.y + GameCoordinates::translations[i][0], 
			move.x + GameCoordinates::translations[i][1] };

		//hoppa direkt till nästa riktning om vi är utanför spelplanen
		if (currCoords.isInValid()) continue;

		//hoppa direkt till nästa om INTE motsatt färg (alltså tom eller samma)
		//detta därför att det krävs en bricka i motsatt färga bredvid där du lägger för att flippa brickor
		if (board.discs[currCoords.y][currCoords.x] != otherColor) continue;

		/*
		Har vi kommit så här långt har vi en bricka med motsatt färg.
		För att utföra ett drag krävs också en bricka med samma färg i 
		slutet av kedjan med brickor i motsatt färg. 
		*/
		while (true) {
			//gå ett till steg i riktningen
			currCoords = { currCoords.y + GameCoordinates::translations[i][0],
			currCoords.x + GameCoordinates::translations[i][1] };

			//fortsätt till nästa riktning om vi är utanför spelplanen
			if (currCoords.isInValid()) break;
			//eller på en tom ruta (här tar kedjan slut)
			if (board.discs[currCoords.y][currCoords.x] == 'a') break;
			 
			//hittar vi däremot en bricka med samma färg är draget giltigt (här tar kedjan slut)
			if (board.discs[currCoords.y][currCoords.x] == sameColor) return true;

			//om det är motsatt färg fortsätter vi i kedjan
		}
		
	}
	/*om vi har kollat alla riktningar utan att förutsättningarna för
	ett giltigt drag mötts, är draget inte giltigt*/
	return false;
}

//[2] returnerar en lista över möjliga drag som kan göras på ett visst bräde
std::vector<GameCoordinates> getListOfPossibleMoves(Board &board) {
	std::vector<GameCoordinates> listOfPossibleMoves; //skapar en vector som vi sen fyller med möjliga drag

	//iterera genom alla rutor på brädet (8 x 8)
	for (int y = 0; y < 8; ++y) {
		for (int x = 0; x < 8; ++x) {
			//för att ett drag ska vara möjligt, måste rutan ligga direkt intill (inkl. diagonalt) en
			//bricka. Vi har en två-dimensionell array som håller koll på detta
			if (!board.adjacents[y][x]) continue;
			
			//om rutan ligger intill en bricka, kollar vi om det är ett giltigt drag
			//är det det, läggs draget till i listan över möjliga drag
			if (isValidMove(board, { y,x })) listOfPossibleMoves.push_back({ y,x });
		}
	}
	//när vi har gått igenom alla rutor returnerar vi listan över möjliga drag
	return listOfPossibleMoves;
}

//[3] placera en bricka på ett bräde, varefter brädet uppdateras
void placeDisc(Board& board, GameCoordinates coords) {
	//OBS: koden här liknar funktionen isValidMove, därför är kommentarerna här färre.
	
	//relativ färg till vems tur det är.
	char sameColor = board.isBlacksTurn ? 'b' : 'c';
	char otherColor = board.isBlacksTurn ? 'c' : 'b';

	//ändra status på rutan där brickan placeras
	board.discs[coords.y][coords.x] = sameColor;

	//lägg till ett på den spelarens räkning över antal brickor
	board.numberOfDiscs[board.isBlacksTurn ? 0 : 1] += 1;

	//eftersom bricken är ockuperad, räknas den inte som en intilliggande ruta
	//även fast den tekniskt sett är det. Detta, eftersom adjacents bara bryr sig om tomma rutor
	board.adjacents[coords.y][coords.x] = false;

	//Nu ändrar vi status på intilliggande rutor, alltså adjacents.
	//iterera alla riktningar
	for (int i = 0; i < 8; ++i) {
		//gå ett steg i riktningen
		GameCoordinates adjCoords = { coords.y + GameCoordinates::translations[i][0], 
			coords.x + GameCoordinates::translations[i][1] };

		//hoppa till nästa riktning om vi är utanför spelplanen
		if (adjCoords.isInValid()) continue;

		//hoppa till nästa riktning om rutan inte är tom
		if (board.discs[adjCoords.y][adjCoords.x] > 'a') continue;

		//sätt rutans status till intilliggande
		board.adjacents[adjCoords.y][adjCoords.x] = true;
	}

	//Nu är det dags att vända på påverkade brickor

	//iterera alla riktningar
	for (int i = 0; i < 8; ++i) {
		//håller koll på om en vändning faktiskt ska ske
		bool flipConfirmed = false;
		//isåfall, är dessa rutorna som ska vändas
		std::vector<GameCoordinates> squaresToBeFlipped = {};
	
		//rutan vi kikar på just nu
		GameCoordinates currCoords = coords;

		//låt oss gå i riktningen
		while (true) {
			//gå ett steg i riktningen
			currCoords = { currCoords.y + GameCoordinates::translations[i][0],
			currCoords.x + GameCoordinates::translations[i][1] };

			//fortsätt till nästa riktning om vi är utanför spelplanen
			if (currCoords.isInValid()) break;
			//eller på en tom ruta
			if (board.discs[currCoords.y][currCoords.x] == 'a') break;

			//hittar vi däremot en bricka med samma färg kan vi vända (förutsatt att det finns något att vända)
			//notera att en svart bricka bredvid en svart bricka kommer förbi första if:n men
			//inte den andra.
			if (board.discs[currCoords.y][currCoords.x] == sameColor) {
				if (!squaresToBeFlipped.empty()) flipConfirmed = true;
				break;
			}

			//om det är motsatt färg lägger vi till rutan i rutor som kanske ska vändas
			squaresToBeFlipped.push_back(currCoords);
		}

		//om en vändning ska ske, flippar vi rutorna i den riktningen
		if (flipConfirmed) {
			//iterera alla rutor som ska vändas i riktningen
			for (GameCoordinates square : squaresToBeFlipped) {
				//ändra status på ruta
				board.discs[square.y][square.x] = sameColor;

				//*här skulle man kunna lägga till animation av vändning*

				//ändra poäng, alltså hur många brickor i varje färg
				//numberOfDiscs[0] är svart och [1] är vit
				board.numberOfDiscs[board.isBlacksTurn ? 0 : 1]++; //ändra i samma färg
				board.numberOfDiscs[board.isBlacksTurn ? 1 : 0]--; //ändra i motsatt
			}
		}


	}
	board.isBlacksTurn = board.isBlacksTurn ? false : true;
}

//[4] DEPRECATED den här funktionen används inte sedan den nya render-motorn, men funktionen har inte tagits bort
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

//[5] låter användaren välja en ruta med piltangenter eller WASD och enter
GameCoordinates getValidPlayerInput(Board & board) {
	bool submit = false;
	GameCoordinates selection = { 4,4 };
	GameCoordinates attemptedSelection = selection;
	render::updateSelectedSquare(selection);

	while (true) {
		if (_kbhit()) {
			int keycode = _getch();
			if (keycode == 224) keycode = _getch(); //vid piltangenttryckning skapas två koder, först 224 sedan koden 

			switch (keycode)
			{
			case 72:  //upppil (eller H)
			case 119: //w
				attemptedSelection.y--;
				break;
			case 80:  //nedpil (eller P)
			case 115: //s
				attemptedSelection.y++;
				break;
			case 75: //vänsterpil (eller K)
			case 97: //a
				attemptedSelection.x--;
				break;
			case 77: //högerpil (eller M)
			case 100: //d
				attemptedSelection.x++;
				break;
			case 13: //enter
			case 32: //space
				submit = true;
				break;
			case 114: //r
				render::restoreScreen();
				break;
			default:
				break;
			}

			if (attemptedSelection.isInValid()) attemptedSelection = selection;
			else selection = attemptedSelection;

			if (submit && isValidMove(board, selection)) {
				render::updateSelectedSquare({ -1,-1 });
				return selection;
			}
			else submit = false;


			render::updateSelectedSquare(selection);
		}
		render::updateScreenAndAnimations();
	}
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
		//om enter eller space
		if (keyPress == 13 || keyPress == 32) break;
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
	static const GameCoordinates corners[] = {
		{0,0},{0,7},{7,0},{7,7}
	};
	int pointsInBlackFavor = board.numberOfDiscs[0];
	/*for (int i = 0; i < 4; ++i) {
		char cornerStatus = board.discs[corners[i].y][corners[i].x];
		if (cornerStatus == 'b') pointsInBlackFavor += 2;
		else if (cornerStatus == 'c') pointsInBlackFavor -= 2;
	}

	render::updateDebugText(std::to_string(pointsInBlackFavor));*/
	return pointsInBlackFavor;
}

int minMax(Board& board,int depth, int blackMaxMin, int whiteMaxMin){
	if (depth == 0) {
		return evaluatePosition(board);
	}
	std::vector<GameCoordinates> possibleMoves = getListOfPossibleMoves(board);

	if (board.isBlacksTurn) {
		int highestEval = -INT_MAX;
		for (int i = 0; i < possibleMoves.size(); ++i) {
			Board newBoard = board;
			placeDisc(newBoard, possibleMoves[i]);
			int eval = minMax(newBoard, depth-1, blackMaxMin, whiteMaxMin);
			highestEval = std::max(eval, highestEval);
			blackMaxMin = std::max(eval, blackMaxMin);
			if (whiteMaxMin <= blackMaxMin) break;
		}
		return highestEval;
	}
	else {
		int lowestEval = INT_MAX;
		for (int i = 0; i < possibleMoves.size(); ++i) {
			Board newBoard = board;
			placeDisc(newBoard, possibleMoves[i]);
			int eval = minMax(newBoard, depth-1, blackMaxMin, whiteMaxMin);
			lowestEval = std::min(eval, lowestEval);
			whiteMaxMin = std::min(eval, whiteMaxMin);
			if (whiteMaxMin <= blackMaxMin) break;
		}
		return lowestEval;
	}
}

//[8]
GameCoordinates chooseComputerMove(Board& board, int computerDifficulty, std::vector<GameCoordinates> possibleMoves) {
	ComputerProgress progress;
	progress.checkedMoves = 0;
	render::updateComputerProgress(progress);
	int searchDepth = computerDifficulty, bestIndex = -1;

	if (board.isBlacksTurn) {
		int highestEval = -INT_MAX;
		for (int i = 0; i < possibleMoves.size(); ++i) {
			Board newBoard = board;
			placeDisc(newBoard, possibleMoves[i]);
			int eval = minMax(newBoard, searchDepth, -INT_MAX, INT_MAX);
			if (eval == highestEval) bestIndex = rand() % 2 ? i : bestIndex; 
			else if (eval > highestEval) {
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
			int eval = minMax(newBoard, searchDepth, -INT_MAX, INT_MAX);
			if (eval == lowestEval) bestIndex = rand() % 2 ? i : bestIndex;
			if (eval < lowestEval) {
				lowestEval = eval;
				bestIndex = i;
			}
		}
	}

	progress.checkedMoves = 1;
	render::updateComputerProgress(progress);
	return possibleMoves[bestIndex];
}

//[9]
bool makePlayerMove(Board& board, GameSettings settings) {
	std::vector<GameCoordinates> possibleMoves = getListOfPossibleMoves(board);
	render::updatePossibleMoves(possibleMoves);

	if (possibleMoves.empty()) {

		board.isBlacksTurn = board.isBlacksTurn ? false : true;
		render::splashText("Spelare har inga möjliga drag", 1500, true);
		return false;
	}

	GameCoordinates move = getValidPlayerInput(board);
	placeDisc(board, move);
	//render::updateDebugText(std::to_string(move.y) + " " + std::to_string(move.x));
	render::updateBoard(false);
	render::updateLastMove(move);
	return true;
}

bool makeComputerMove(Board& board, GameSettings settings, int difficulty = 0) {
	std::vector<GameCoordinates> possibleMoves = getListOfPossibleMoves(board);
	
	if (possibleMoves.empty()) {
		board.isBlacksTurn = board.isBlacksTurn ? false : true;
		render::splashText("Ai har inga möjliga drag", 1500, true);
		return false;
	}
	
	GameCoordinates chosenMove = chooseComputerMove(board, difficulty, possibleMoves);
	placeDisc(board, chosenMove);
	render::updateBoard(false);
	render::updateLastMove(chosenMove);
	return true;
}


int main() {
	srand((int)time(0));
	setlocale(LC_ALL, "sv_SE");
	Board gameBoard;
	GameSettings settings;
	render::init();
	render::setBoard(&gameBoard);
	render::setSettings(&settings);
	
	//std::this_thread::sleep_for(std::chrono::milliseconds(10000));
	while (true) {
		gameBoard = Board();
		initGameSettings(settings);
		system("cls");
		render::updateBoard();
		render::updateSettings();
		bool player1successful = true, player2successful = true;
		while (player1successful || player2successful) {
			player1successful = settings.player1iscomp ? makeComputerMove(gameBoard, settings, settings.comp1Difficulty) : makePlayerMove(gameBoard, settings);
			player2successful = settings.player2iscomp ? makeComputerMove(gameBoard, settings, settings.comp2Difficulty) : makePlayerMove(gameBoard, settings);
		}
		endGame(gameBoard, settings);
		std::cout << "Vill du köra igen? Klicka ENTER.\nVill du avsluta? Klicka valfri tangent\n";
		if (_getch() != 13) break; //13 är koden för enter
		
	}
	return 0;
}