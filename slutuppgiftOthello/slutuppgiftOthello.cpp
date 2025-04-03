
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
			<< " - Svart" << (settings.player1isComp ? " (AI)" : "");
		//spelare 2 (vit)
		else if (rowNumber == 1) std::cout << "  " << board.numberOfDiscs[1]
			<< " - Vit" << (settings.player2isComp ? " (AI)" : "");
		//vems tur det är
		else if (rowNumber == 3) std::cout <<
			(board.isBlacksTurn ? "  *Svarts tur*" : " *Vits tur*");
		
		//nu har vi skrivit klart raden och kan gå till nästa
		std::cout << "\n";
	}
	std::cout << "\n    \033[F";
}

//[5] låter användaren markera en ruta med piltangenter eller WASD och välja med enter/space
GameCoordinates getValidPlayerInput(Board & board) {
	bool submit = false; //håller koll på om användaren valt en ruta

	GameCoordinates selection = { 4,4 }; //markerade koordinaten (börjar alltid i mitten (ungefär))

	GameCoordinates attemptedSelection = selection; //en temporär variabel utifall spelaren försöker gå utanför spelplanen
	
	render::updateSelectedSquare(selection); //visa markerad ruta på skärmen

	//input-loopen
	while (true) { 
		//vi använder kbhit så inte programmet pausas i väntan på input, detta
		//så att animationer kan uppdateras sålänge
		if (_kbhit()) {
			//om tangenbordet har tryckts in kollar vi vilken tangent som tryckts in
			int keycode = _getch();

			//vid piltangenttryckning skapas två koder, först 224 sedan koden 
			if (keycode == 224) keycode = _getch();

			//switcha koden, alltså kolla vilken tangent
			switch (keycode)
			{
			case 72:  //upppil (eller H)
				//koden för upppil är 224 + 72, koden för H är 72, därav funkar både här
				//eftersom upppil och w gör samma sak kan man stacka cases på varandra
			case 119: //w
				attemptedSelection.y--; //flytta markerad ruta upp ett steg
				break;
			case 80:  //nedpil (eller P)
			case 115: //s
				attemptedSelection.y++; //flytta markerad ruta ned ett steg
				break;
			case 75: //vänsterpil (eller K)
			case 97: //a
				attemptedSelection.x--; //flytta markerad ruta vänster ett steg
				break;
			case 77: //högerpil (eller M)
			case 100: //d
				attemptedSelection.x++; //flytta markerad ruta höger ett steg
				break;
			case 13: //enter
			case 32: //space
				submit = true; //vi sparar att användaren vill välja ruta
				break;
			case 114: //r
				render::restoreScreen(); //ett specialfall, som är till för att
				//rensa skärmen och rita upp den igen. Används om man ändrar fönstret.
				break;
			default:
				break;
			}

			//kollar om markering är innanför spelplanen
			//är den utanför sätter vi bara attempetedSelection till senaste selectionen som lyckades
			if (attemptedSelection.isInValid()) attemptedSelection = selection;
			//är den innanför sätter vi den senaste selectionen till markeringen
			else selection = attemptedSelection;

			//om användaren valt ruta OCH det är ett giltigt draag
			if (submit && isValidMove(board, selection)) {
				//koordinaten {-1, -1} är utanför spelplanen, därför kommer ingen markerad
				//ruta längre synas på skärmen.
				render::updateSelectedSquare({ -1,-1 });

				//returna den slutgiltiga koordinaten för rutan användaren valde
				return selection;
			}
			//om det inte var ett giltigt drag nullifierar vi helt enkelt
			//submit-variabeln och loopen fortsätter
			else submit = false;

			//visa markerad ruta
			render::updateSelectedSquare(selection);
		}

		//om ingen input har skett vill vi ändå uppdatera animationerna
		//finns det animationer att uppdatera, tas dem av hand i följande funktion
		render::updateScreenAndAnimations();
	}
}

//[6.1] skapar en enkel flervalsmeny styrd av piltangenter och enter/space, returnerar index av valt val. 
int simpleSelectionChoice(std::string header, std::vector<std::string> options) {
	//header är titeln, eller informationen för menyn
	//options är listan över alternativ

	int keyPress; //koden för tangentnedtryckning
	int selection = 0; //index för valt alternativ

	std::cout << "\033[2J"; //rensar skärmen

	while (true) {
		//hoppar till högst upp på skärmen och skriver ut titeln
		std::cout << "\033[H" << header << '\n';

		//iterera genom alla alternativ i syfte att skriva ut dem
		for (int i = 0; i < options.size(); ++i) {
			if (i == selection) //om markerat
				std::cout << "> " << options[i] << " <              \n";
			//om inte markerat
			else std::cout << "  " << options[i] << "               \n";
			//vi skriver ut extra mycket mellanrum i slutet, eftersom
			//skärmen inte rensas utan bara skrivs över
			//*man skulle kunna kolla vilket alternativ som har längst text
			//för att anpassa marginalen*
		}

		keyPress = _getch(); //få input

		//om enter eller space
		if (keyPress == 13 || keyPress == 32) break; //hoppa ur loopen då ett val har gjorts

		if (keyPress != 224) continue; //om inte piltangent
		//piltangenter genererar alltid två koder: 224 + X

		keyPress = _getch(); //få koden för piltangenten (X alltså)

		if (keyPress == 80) { // om ned-pil
			//flytta markering ned, om vi inte redan är på sista alternativet
			selection += (selection < options.size() - 1 ? 1 : 0);
		}
		else if (keyPress == 72) { //upp-pil
			//flytta markering upp, om vi inte redan är på första alternativet
			selection -= (selection > 0 ? 1 : 0);
		}
		
	}
	//efter att vi har hoppat ur loppen på Enter/Space, returnerar vi indexet var valt alternativ
	return selection;
}

//[6] läs in inställningar från användaren
void initGameSettings(GameSettings& settings) {
	const std::vector<std::string> optionsPlayer1 = { "Svart ska spelas av människa", "Svart ska spelas av dator" }; //alternativ för spelare 1 / Svart
	const std::vector<std::string> optionsPlayer2 = { "Vit ska spelas av människa", "Vit ska spelas av dator" }; //alternativ för spelare 2 / Vit
	//alternativ för ai-svårighetsgrad. 
	const std::vector<std::string> optionsDifficulty = { "0 - Slumpat", "1 - Buslätt", "2 - Enkel", "3 - Medel", "4 - Svår", "5 - Läskig", "6 - Supersvår", "7 - Ett drag på timme"}; 

	//läs in val för huruvida svart är AI eller människa
	//returnering blir 0 eller 1, vilket omvandlas till false/true genom att casta till bool
	settings.player1isComp = (bool)simpleSelectionChoice("\n Välkommen till Othello\nSvart börjar spelet. Välj alternativ för svart:\n",optionsPlayer1);
	
	//om svart är AI
	if (settings.player1isComp) {
		//läs in svårighetsgrad för AI
		settings.comp1Difficulty = simpleSelectionChoice("\n Välommen till Othello\nSvart spelas av en dator, vilken svårighetsgrad?\n", optionsDifficulty);
	}

	//läs in val för vit på samma sätt

	settings.player2isComp = (bool)simpleSelectionChoice("\n Välkommen till Othello\nVit fortsätter spelet. Välj alternativ för vit:\n", optionsPlayer2);

	if (settings.player2isComp) {

		settings.comp2Difficulty = simpleSelectionChoice("\n Välkommen till Othello\nVit spelas av en dator, vilken svårighetsgrad?\n", optionsDifficulty);
	}
}

//[7] när spelet är slut anropas denna funktion, som då visar hur vem som vann och ställning
void endGame(Board& board, GameSettings& settings) {
	//håller koll på statistik under hela programmets körtid
	//*här skulle man kunna lägga till filhantering för att lagra ex. Highscore*
	static int blackWins = 0, whiteWins = 0, ties = 0;

	std::cout << "SPELET ÄR ÖVER\n";
	//om lika
	if (board.numberOfDiscs[0] == board.numberOfDiscs[1]) {
		std::cout << "Se där, det blev lika!\n";
		++ties;
	}
	//om svart vann
	else if (board.numberOfDiscs[0] > board.numberOfDiscs[1]) {
		std::cout << "Grattis"<< (settings.player1isComp ? " datorn" : " spelare") << ", SVART vann\n";
		++blackWins;
	} 
	//om vit vann
	else {
		std::cout << "Grattis" << (settings.player2isComp ? " datorn" : " spelare") << ", VIT vann\n";
		++whiteWins;
	}

	//skriv ut ställningen
	std::cout << "\nStällning under programmet körtid:\n";
	std::cout << "Svart " << blackWins << " - " << whiteWins << " Vit\n";
}

//funktion som används av AIn för att kolla på hur bra ett position är
//returnerar högt värde om det går bra för svart
//returnerar lågt värde om det går bra för vit
int evaluatePosition(Board& board){
	//koordinater för hörnen
	static const GameCoordinates corners[] = {
		{0,0},{0,7},{7,0},{7,7}
	};
	//utgångspunkten är hur många brickor svart har
	//egentligen räcker det med bara det
	int pointsInBlackFavor = board.numberOfDiscs[0];

	//iterera genom alla hör
	for (int i = 0; i < 4; ++i) {
		//spara status i det hörnet
		char cornerStatus = board.discs[corners[i].y][corners[i].x];

		//ge två poäng till svart om hörnet tillhör svart
		if (cornerStatus == 'b') pointsInBlackFavor += 2;
		//subtrahera två poäng från svart om hörnet tillhör vit
		else if (cornerStatus == 'c') pointsInBlackFavor -= 2;
	}
	//detta incentiveserar att få hörn, vilket kanske känns mer mänskligt
	//dock gör detta ingen större skillnad.

	//returnera poäng i svarts favör
	return pointsInBlackFavor;
}

//rekursiv algorithm för att söka igenom ett träd av möjliga drag
int minMax(Board& board,int depth, int blackMaxMin, int whiteMaxMin){
	//notera att board inte är brädet som visas på skärmen
	//utan ett hypotetiskt bräde med framtida positioner
	
	//blackMaxMin och whiteMaxMin, är enbart till för optimisering
	//genom vad som kallas "pruning"
	//det är genom att hålla koll på vad svart och vit är garanterade att få
	//som man kan klippa bort grenar eftersom de inte pågår resultatet

	if (depth == 0) { //utgångvillkoret för funktionen
		//returnerar den statiska utvärderingen av positionen
		return evaluatePosition(board);
	}

	//lista över möjliga drag
	std::vector<GameCoordinates> possibleMoves = getListOfPossibleMoves(board);

	//om det är svarts tur vill vi ha en så hög utvärdering som möjligt
	if (board.isBlacksTurn) {
		int highestEval = -INT_MAX; //vi börjar därför på en oändligt låg utvärdering
		
		//iterera alla möjliga drag
		for (int i = 0; i < possibleMoves.size(); ++i) {
			Board newBoard = board; //skapa ett nytt bräde som är en kopia av det vi har
			//utför draget vi är på på detta nya bräde
			placeDisc(newBoard, possibleMoves[i]);
			//skicka vidare detta ett steg ner i minMax-trädet
			//depth-1 är så vi till slut kommer till slutet, då depth == 0
			int eval = minMax(newBoard, depth-1, blackMaxMin, whiteMaxMin);
			//vi vill ha den bästa utvärderingen
			highestEval = std::max(eval, highestEval);
			//blackMaxMin är vad svart är garanterad att få
			blackMaxMin = std::max(eval, blackMaxMin);
			//om det lägsta vit är garanterad är lägre eller lika med
			//det högsta svart är garanterad att få finns det ingen
			//mening att fortstta gå i trädet, därför breaker vi (klipp grenen)
			//och minmax fortsätter i de andra grenarna
			if (whiteMaxMin <= blackMaxMin) break;
		}
		//returnera utvärderingen för bästa draget vi hittade för svart
		return highestEval;
	}
	//om det är vits tur vill vi ha en så låg utvärdering som möjligt
	else {
		int lowestEval = INT_MAX; //vi börjar med en oändlig hög utvärdering (alltså jättedåligt)
		for (int i = 0; i < possibleMoves.size(); ++i) { //iterera drag
			Board newBoard = board; //temp bräde
			placeDisc(newBoard, possibleMoves[i]); //placera en disk på detta låtsas-bräde
			int eval = minMax(newBoard, depth-1, blackMaxMin, whiteMaxMin);
			lowestEval = std::min(eval, lowestEval); //nu vill vi istället ha den lägsta
			//utvärderingen
			whiteMaxMin = std::min(eval, whiteMaxMin);
			//samma sak som för svarts tur. 
			if (whiteMaxMin <= blackMaxMin) break;
		}
		//returnera utvärderingen för sämsta draget vi hittade för svart (vilket är bästa för vit)
		return lowestEval;
	}
}

//[8] den här funktionen väljer ett drag ifrån möjliga drag på ett bräde, med en viss svårighetsgrad
GameCoordinates chooseComputerMove(Board& board, int computerDifficulty, std::vector<GameCoordinates> possibleMoves) {
	
	ComputerProgress progress; //en struct som håller meta-information om
	//datorns framsteg i trädet. Används enbart för debug / rendering
	progress.checkedMoves = 0; //säger att vi precis har börjat leta efter drag
	render::updateComputerProgress(progress); //säger till render-motorn detta

	//svårighetsgraden är djupet på söket, som skickas till minmax-algoritmen
	//bestIndex är indexet till det drag i possibleMoves som är bäst
	int searchDepth = computerDifficulty, bestIndex = -1;

	//här liknar koden minmax mycket, men med vissa ändringar
	//för förklaring av koden, se minmax
	//för svart
	if (board.isBlacksTurn) {
		int highestEval = -INT_MAX;
		for (int i = 0; i < possibleMoves.size(); ++i) {
			Board newBoard = board;
			placeDisc(newBoard, possibleMoves[i]);
			//initiellt är blackMaxMin och whiteMaxMin lika med det sämsta, respektive bästa för svart
			int eval = minMax(newBoard, searchDepth, -INT_MAX, INT_MAX);
			//om vi hittat ett nytt drag som är lika bra som det bästa draget
			//ändrar vi till det nya draget i 50% av fallen
			//vi ändrar då bestIndex variablen, men inte highestEval, eftersom den är samma.
			if (eval == highestEval) { 
				if (bestIndex >= 0)
					bestIndex = rand() % 2 ? i : bestIndex;
				else //vi kan bara slumpa ett drag om vi har hittat ett bra drag förut, är bestIndex == -1 har vi inte hittat ett drag än
					bestIndex = i;
			}

			
			//om draget är bättre
			else if (eval > highestEval) { 
				highestEval = eval; //ändra värdet på bästa drag hittils
				bestIndex = i; //spara indexet så vi har något att returnera
			}
		}
	}
	//för vit, i princip samma fast min istället för max
	else {
		int lowestEval = INT_MAX;
		for (int i = 0; i < possibleMoves.size(); ++i) {
			Board newBoard = board;
			placeDisc(newBoard, possibleMoves[i]);
			int eval = minMax(newBoard, searchDepth, -INT_MAX, INT_MAX);
			if (eval == lowestEval) {
				if (bestIndex >= 0)
					bestIndex = rand() % 2 ? i : bestIndex;
				else //vi kan bara slumpa ett drag om vi har hittat ett bra drag förut, är bestIndex == -1 har vi inte hittat ett drag än
					bestIndex = i;
			}
			if (eval < lowestEval) { //mindre än istället för mer än
				lowestEval = eval;
				bestIndex = i;
			}
		}
	}

	progress.checkedMoves = 1; //signalerar att vi är klara med att hitta ett drag
	render::updateComputerProgress(progress); //visar tiden som det tog att välja draget
	if (bestIndex < 0 || bestIndex >= possibleMoves.size()) {
		std::cout << "\a";
	}
	return possibleMoves[bestIndex]; //returnerar draget datorn har valt
}

//[9] läser in input från användaren och placerar brickan. Returnerar huruvida ett drag gjordes.
bool makePlayerMove(Board& board, GameSettings settings) {
	//möjliga drag
	std::vector<GameCoordinates> possibleMoves = getListOfPossibleMoves(board);
	render::updatePossibleMoves(possibleMoves); //visar möjliga drag för spelaren på skärmen

	//om spelaren inte kan göra ett drag
	if (possibleMoves.empty()) {
		//det blir nästa spelares tur
		board.isBlacksTurn = board.isBlacksTurn ? false : true;

		//vi visar ett meddelande på skärmen 
		//eftersom argumentet returnWhenFinished är satt till true, kommer
		//exekveringen av resten av koden vänta tills meddelandet har försvunnit, om 1500 ms.
		render::splashText("Spelare har inga möjliga drag", 1500, true);

		//returnerar false, eftersom inget drag gjordes
		return false;
	}

	//läs in ett drag från spelaren
	GameCoordinates move = getValidPlayerInput(board);
	
	//placera brickan
	placeDisc(board, move);

	render::updateBoard(false); //uppdatera brädet utan att uppdatera skärmen
	render::updateLastMove(move); //uppdatera senaste draget, samt uppdatera skärmen

	//returnera true då ett drag gjordes
	return true;
}


//motsvarande funktion till makePlayerMove, fast för datorn.
//returnerar huruvida ett drag gjordes
bool makeComputerMove(Board& board, GameSettings settings, int difficulty = 0) {
	//lista över möjliga drag
	std::vector<GameCoordinates> possibleMoves = getListOfPossibleMoves(board);
	
	//om det inte finns några möjliga drag
	if (possibleMoves.empty()) {
		//nästa spelares tur
		board.isBlacksTurn = board.isBlacksTurn ? false : true;

		//visa meddelande. Exekvering pausas i 1500 ms.
		render::splashText("AI har inga möjliga drag", 1500, true);

		return false; //returnera false, då inget drag gjordes
	}
	
	//välj ett drag
	GameCoordinates chosenMove = chooseComputerMove(board, difficulty, possibleMoves);
	placeDisc(board, chosenMove); //placera brickan på spelbrädet
	render::updateBoard(false); //uppdatera brädet utan att uppdatera skärmen
	render::updateLastMove(chosenMove); //uppdatera senaste drag samt uppdatera skärmen
	return true; //returnera true, då ett drag gjordes
}

//huvudfunktionen, som körs under hela programmets gång
int main() {
	srand((int)time(0)); //ge ett frö åt slumpgeneratorn
	setlocale(LC_ALL, "sv_SE"); //ändra lokalitetsinställningar så att åäö kan visas korrekt

	Board gameBoard; //en instans av Board som håller det aktiva spelbrädet
	GameSettings settings; //en instans av GameSettings som håller de aktiva spelarinställningarna

	render::init(); //initialisera render-motorn
	render::setBoard(&gameBoard); //ge render-motorn en referens till brädet
	render::setSettings(&settings); //ge render-motorn en referens till inställningarna
	
	//loopar mellan matcher
	while (true) {
		//starta en ny match

		gameBoard = Board(); //initialisera spelbrädet på nytt, dvs starta om det
		initGameSettings(settings); //läs in inställningar från användaren
		system("cls"); //rensa skärmen så att inga artifakter finns kvar från menyn

		render::updateBoard(); //visa spelplanen
		render::updateSettings(); //visa inställningar (alltså om spelare är AI eller inte)

		//variabler som håller koll på spelarna lyckades med sina senaste drag
		bool player1successful = true, player2successful = true;

		//medans någon av spelarna kunde göra ett drag senast
		while (player1successful || player2successful) { 
			//gör först ett drag åt spelare 1 (svart), sedan spelare 2 (vit).
			//vi använder inställninsvariablen för att välja vilken funktion som ska anropas
			player1successful = settings.player1isComp ? makeComputerMove(gameBoard, settings, settings.comp1Difficulty) : makePlayerMove(gameBoard, settings);
			player2successful = settings.player2isComp ? makeComputerMove(gameBoard, settings, settings.comp2Difficulty) : makePlayerMove(gameBoard, settings);
		}

		//när ingen av spelarna kunde göra ett drag senast, är spelet slut
		//vi anropar funktionen som skriver ut resultatet
		endGame(gameBoard, settings);

		//enkel input för att antingen avsluta programmet, eller köra en till match
		std::cout << "Vill du köra igen? Klicka ENTER.\nVill du avsluta? Klicka valfri tangent\n";
		if (_getch() != 13) break; //13 är koden för enter
		
	}

	//returnera 0 eftersom programmet kördes framgångsrikt
	return 0;
}