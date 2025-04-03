#include "clRender.h" //här finns alla deklarationer som används av huvud-filen
#include <iostream> //cout
#include <vector> //std::vector
#include <string> //std::string
#include <chrono> //animationer, FPS, tid för datordrag

//det här är den så kallade render-motorn
//som sköter nästan all utmatning
//för mer information om användning, se header-filen

using namespace render; //ett custom namespace som finns i header-filen

//Globala konstanter

//ANSI-koder, främst för färger. Notera att detta inte är alla ANSI-koder som programmet använder
//RGB signalerar att koderna följer formatet "\033[{3/4}8;2;{r};{g};{b}m"
static const std::string ANSI_BOARD_BG = "\033[48;2;24;140;27m", ANSI_DEFAULT = "\033[0m", 
RGB_WHITE_FG = "\033[38;2;255;255;255m", RGB_BLACK_FG = "\033[38;2;0;0;0m", RGB_BLACK_BG = "\033[48;2;0;0;0m";

//Värden för mängd luft spelbrädet
static const int LEFT_PADDING = 2, TOP_PADDING = 1, RIGHT_PADDING = 1, BOTTOM_PADDING = 0, SCOREBOARD_HEIGHT = 2;

//Namn på spelare och dator som visas i scoreboarden till höger om brädet
static const std::string NAME_PLAYER_1 = "Svart", NAME_PLAYER_2 = "Vit", NAME_COMPUTER = "AI";

//symboler för utmatning, måste vara två karaktär i bredd (bortsedd från Ansi-koder)
static const std::string emptySymbol = "  ", //för rutor inte ockuperade
blackSymbol = "\033[30m X", whiteSymbol = "\033[37m O", //för rutor ockuperade av svart eller vit
highlightBlackSymbol = "\033[33m X", highlightWhiteSymbol = "\033[33m O", //symboler för där senaste draget lades
possibleSymbol = "\033[38;5;27m +", //symbol för alla rutor som är möjliga att lägga i
selectModifier = "\033[48;5;246m"; //inte en symbol, utan en ANSI-kod som ändrar hur det ser ut när man markerar en ruta

//Globala variabler
//dessa håller koll på vad som visas eller kommer att visas på skärmen
static std::chrono::time_point<std::chrono::system_clock> lastFrameUpdate; //när bilden uppdaterades senast
static std::string boardDisplayState[8][8]; //innehåller brädet (8x8)
static std::string secondaryDisplayState[12]; //innehåller övrig information på spelplanen
//titel på [0], debug på [9], datorinfo på [11] exempelvis

static Board* renderedBoardPtr = nullptr; //pekare till det aktiva spelbrädet
static GameSettings* gameSettingsPtr = nullptr; //pekare till aktiva inställningar

static std::vector<GameCoordinates> possibleMovesBuffer = {}; //här lagras listan över möjliga drag

static GameCoordinates selectedMove = { -1,-1 }; //markerad ruta
static std::string selectedMoveBuffer = ""; //lagrar vad som fanns på den markerade rutan förut

static std::string splashTextStr = ""; //lagrar vilken överlagringstext som just nu visas på skärmen
static std::string splashTextStyle = "\033[38;2;255;255;255m\033[48;2;0;0;0m"; //hur överlagringstexten ser ut
static int splashLeftMargin = 7; //var överlagringstexten börjar
static std::chrono::time_point<std::chrono::system_clock> splashExpiryTime; //när överlagringstexten slutar synas

// funktion för att skriva ut överlagringstext, används av print()
// förutsätter att utskrift av splashtext inte redan skett 
// returnerar true om text har skrivits ut
// ändrar column-variabeln till sista kolumnen utskrift har skett
static bool printSplashText(int & column){
	if (splashTextStr.empty()) return false; //finns det ingen överlagringstext, kan ingen utskrift ske

	std::string tempSplashTextStr = splashTextStr; //temporär variabel eftersom en vi behöver att
	//längden på utskriften är ett jämnt antal karaktärer, då rutorna på brädet är två karaktärer breda
	
	//Index  0 1 2 3 4 5 6 7 8 9 ...
	//Kolumn  0   1   2   3   4 ...
	//Margin 1 2 3 4 5 ...
	//om första index i nästa kolumn är mer än margin, då är det dags att skriva ut
	if (2 * (column + 1) > splashLeftMargin) {
		//om margin är udda, lägg till en tom karaktär i början som padding
		//anledning till temp-variabel är så vi inte lägger till padding
		//till den faktiska text-variabeln
		if (splashLeftMargin % 2) tempSplashTextStr = ' ' + tempSplashTextStr;

		//på likande sett lägger vi till ett mellanrum i slutet om det behövs för ett jämnt antal karaktärer
		if (tempSplashTextStr.length() % 2) tempSplashTextStr += ' ';
		
		//skriv ut texten, med rätt style, samt återgå till vanliga stylen efter
		std::cout << splashTextStyle + tempSplashTextStr + ANSI_BOARD_BG;

		//hoppa fram till slutet av överlagringstexten
		//notera -1, detta är för att print fortsätter ytterligare en kolumn fram
		column += (tempSplashTextStr.length()-1) / 2;
		
		return true; //returnera true, eftersom text skrevs ut
	}

	return false; //returnera false, då ingen text skrevs ut
}

//funktionen som skriver ut displayState på skärmen
static void print() {
	//notera att string(int i, char c) skapar en string av längd i där varje karaktär är c

	//håller koll på om överlagringstext har skrivits ut än
	bool splashTextWritten = false;

	std::cout << "\033[H"; //gå till "hem-positionen", alltså översta vänstra hörnet

	//skriver ut luft ovanför spelplanen
	std::cout << std::string(TOP_PADDING, '\n'); 

	//skriver ut luft till vänster + titeln
	std::cout << std::string(LEFT_PADDING, ' ') + secondaryDisplayState[0] << '\n';

	//itererar linje 1-8, vilket motsvarar 0-7 i boardDisplayState
	for (int lineIndex = 1; lineIndex < 9; ++lineIndex) {
		//skriver ut luft och ändrar style till bakgrunden
		std::cout << std::string(LEFT_PADDING, ' ') + ANSI_BOARD_BG;

		//iterera genom kolumner
		for (int column = 0; column < 8; ++column) {
			//överlagringstext skrivs ut på raden mellan "poäng - vit" och *X tur*
			//Scoreboard_height är index för linjen där "poäng - svart" skrivs ut
			//därav, om rätt linje OCH överlagringstext inte skrivits ut ännu
			if (lineIndex == SCOREBOARD_HEIGHT+2 && !splashTextWritten) {
				//då ser vi om vi kan skriva ut någon text, kunde vi det
				if (printSplashText(column)) {
					//ändrar vi variablen till true
					splashTextWritten = true;
					//och går till nästa iteration av kolumnera, som då kommer addera ett till variablen column
					continue;
				}
			}

			//skrevs ingen text ut nyss, kan vi skriva ut rutan
			//lineIndex - 1 eftersom lineindex går mellan 1 och 8
			std::cout << boardDisplayState[lineIndex - 1][column];
		}
		
		//skriver ut luft efter brädet och det som eventuellt fins i secondaryDisplayState på denna rad samt går till nästa rad
		std::cout << ANSI_DEFAULT + std::string(RIGHT_PADDING, ' ') + secondaryDisplayState[lineIndex] << '\n';
	}

	//nu har brädet skrivits ut, då skapar vi lite luft under
	std::cout << std::string(BOTTOM_PADDING, '\n');

	//iterera de linnjer som är kvar (9-11)
	for (int lineIndex = 9; lineIndex < 12; ++lineIndex) {
		//skriv ut luft, information och gå till nästa rad
		std::cout << std::string(LEFT_PADDING, ' ') + secondaryDisplayState[lineIndex] << '\n';
	}

	//uppdatera tiden för senaste bilduppdateringen
	std::chrono::time_point<std::chrono::system_clock> tLastFrameUpdate = lastFrameUpdate;
	lastFrameUpdate = std::chrono::system_clock::now();

	//med hjälp av följande kod kan man se tiden mellan bildrutor
	//updateDebugText(std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(lastFrameUpdate - tLastFrameUpdate).count()), false);
}

// funktion för att linjärt interpolera, alltså ta en punkt mellan, två färger i ANSI-format
// parametern pointBetween ska vara mellan 0 och 1, där 0 blir lika med startfärg och 1 lika med slutfärg
static std::string interpolateColor(std::string startColorAnsi, std::string endColorAnsi, float pointBetween) {
	//input måste vara antingen i format "\033[48;2;X;X;Xm eller "\033[38;2;X;X;Xm" där X är heltal mellan 0 och 255
	//38 eller 48 avgör förgrund eller bakgrund

	//första måste vi ta ut siffrorna ur ANSI-koden, som dessutom kan vara olika karaktärer långa
	//därför använder vi substrings för att ta alla karaktärer fram tills separeringskaraktären (; eller m)

	std::string beginningCode = startColorAnsi.substr(0,7); //ansi-koden början, ex "\033[38;2;"
	std::string remainingCode = startColorAnsi.substr(7); //resten av koden, ex "25;255;0m"

	//vi extraherar röd i startkoden
	int nextSeperatorIndex = remainingCode.find(';'); //var finns nästa semikolon ex index 2
	int startRed = std::stoi(remainingCode.substr(0, nextSeperatorIndex)); //ta karaktärerna fram till semikolon, ex "25", konvertera till int, ex 25
	remainingCode = remainingCode.substr(nextSeperatorIndex+1); //resten av koden, ex "255;0m"

	//samma för grön i startkoden
	nextSeperatorIndex = remainingCode.find(';'); //ex index 3
	int startGreen = std::stoi(remainingCode.substr(0, nextSeperatorIndex)); //ex 255
	remainingCode = remainingCode.substr(nextSeperatorIndex+1); // ex "0m"

	//samma för blå, fast med 'm' istället för ';'
	nextSeperatorIndex = remainingCode.find('m'); //ex index 1
	int startBlue = std::stoi(remainingCode.substr(0, nextSeperatorIndex)); //ex 0
	//och där är vi klara med start-koden

	//dags för slutkoden
	//exakt samma, men vi ändrar inte beginningCode, eftersom vi förutsätter att den är samma
	//är den inte det är det första färgkoden som avgör om det blir för- eller bakgrund
	remainingCode = endColorAnsi.substr(7);
	nextSeperatorIndex = remainingCode.find(';');
	int endRed = std::stoi(remainingCode.substr(0, nextSeperatorIndex));
	remainingCode = remainingCode.substr(nextSeperatorIndex+1);

	nextSeperatorIndex = remainingCode.find(';');
	int endGreen = std::stoi(remainingCode.substr(0, nextSeperatorIndex));
	remainingCode = remainingCode.substr(nextSeperatorIndex+1);

	nextSeperatorIndex = remainingCode.find('m');
	int endBlue = std::stoi(remainingCode.substr(0, nextSeperatorIndex));
	remainingCode = remainingCode.substr(nextSeperatorIndex+1);

	//formeln är interpolation = start + (end - start) * pointBetween
	//vi utför formeln separat på alla tre färger
	int red = startRed + (endRed - startRed) * pointBetween;
	int green = startGreen + (endGreen - startGreen) * pointBetween;
	int blue = startBlue + (endBlue - startBlue) * pointBetween;
	
	//sedan sätter vi ihop det till en ny kod och returnerar den
	return beginningCode + std::to_string(red) + ';' + std::to_string(green) + ';' + std::to_string(blue) + 'm';
}


//den här funktionen uppdaterar animationer
//observera att den inte kan anropas utifrån en annan fil eftersom den är static
//samt att den inte uppdaterar skärmen (den anropar inte print)
//i nuläget är det bara överlagringstexten som är animerad, men det är inte överdrivet krångligt att lägga till mer
static void updateAnimations()
{
	using namespace std::chrono; //så vi slipper skriva std::chrono överallt

	//konstanter som avgör hur snabba animationerna är
	//splashtextspeed är millisekunder / två bokstäver, splashcolorspeed är ms på hela övergången
	const milliseconds splashTextSpeed = milliseconds(30), splashColorSpeed = milliseconds(300);

	//ALL ANIMATION FÖR SPLASH TEXT
	//hur mycket tid det är kvar tills överlagringstexten ska vara helt borta
	milliseconds timeToSplashExpiry = duration_cast<milliseconds>(splashExpiryTime - system_clock::now());

	//hur lång texten får vara. Ex. 10 sekunder kvar, och splashtextspeed är 1 sekund, tillåten längd är 10 karaktärer.
	//när det är 0 sekunder kvar (alltså när texten ska vara helt borta) är tillåten längd 0 karaktärer.
	int allowedSplashLength = timeToSplashExpiry / splashTextSpeed;

	//har tiden redan gått ut, ser vi bara till att texten faktiskt är tom
	//detta är så vi inte behöver krångla med negativa värden
	if (timeToSplashExpiry < milliseconds(0)) {
		splashTextStr = "";
	}
	//annars kollar vi längden på texten överstiger maxgränsen
	else if (splashTextStr.length() > allowedSplashLength) {
		//isåfall skjuter vi fram texten ett steg åt höger (så att den hålls centrerad)
		splashLeftMargin++;

		//och tar bort första och sista bokstaven
		//vi tar length-2 efter det argumentet är för hur lång texten ska vara, inte index
		//som man skulle kunna tro.
		splashTextStr = splashTextStr.substr(1,splashTextStr.length()-2);
	}

	//Dags att animera färgen
	//om tiden som är kvar är mindre än tiden för hela övergången samt att tiden inte redan har utgått
	if (timeToSplashExpiry < splashColorSpeed && timeToSplashExpiry >= milliseconds(0)) {
		//räkna ut andelen tid mellan början och slutet på övergången
		//formel: 1 - (tidkvar / tidhela)
		float colorProgress = 1.0f - timeToSplashExpiry.count() / (float)splashColorSpeed.count();

		//stylen är en kombination av bakgrund (BG) och förgrund (FG)
		//notera att i det första funktionsanropen är FG och BG kombinerat, men det är bara
		//den första som bestämmer om det är FG eller BG
		splashTextStyle = interpolateColor(RGB_WHITE_FG, ANSI_BOARD_BG, colorProgress) + interpolateColor(RGB_BLACK_BG, ANSI_BOARD_BG, colorProgress);
	}
	
}

//den här funktionen är till att användas av restan av programmet för att uppdatera skärmen
//om programmet ex. väntar på input
void render::updateScreenAndAnimations() {
	updateAnimations(); //uppdatera animationer
	print(); //visa animationerna
}

//den här funktionen måste kallas innan övrig uppdatering av skärmen sker
//den initialiserar de variabler som innehåller displaystate
void render::init() {

	std::cout << "\033[?25l"; //göm skriv-markören

	secondaryDisplayState[0] = " O T H E L L O"; //bestäm titeln

	//gå igenom hela brädet (8x8) och initialisera varje ruta
	//samt initialisera secondarydisplaystate index 1-8
	for (int lineIndex = 1; lineIndex <= 8; ++lineIndex) {
		secondaryDisplayState[lineIndex] = "";
		for (int column = 0; column < 8; ++column) {
			//notera lineindex-1 eftersom lineindex går från 1 till 8
			boardDisplayState[lineIndex - 1][column] = emptySymbol;
		}
	}

	//initialisera poängtavlan
	secondaryDisplayState[SCOREBOARD_HEIGHT] = "x - " + NAME_PLAYER_1 + " (" + NAME_COMPUTER + ")";
	secondaryDisplayState[SCOREBOARD_HEIGHT+1] = "x - " + NAME_PLAYER_2 + " (" + NAME_COMPUTER + ")";

	//vi vill inte ha några artifakter (kom ihåg att skärmen inte rensas, utan bara skrivs över)
	//därför räknar vi ut vad det längsta spelarnamnet är (ex. "svart" vs "vit" blir "svart" alltså 5)
	int longestPlayerName = std::max(NAME_PLAYER_1.length(), NAME_PLAYER_2.length());
	//observera också att index scoreboard height + 2 är reserverad för överlagringstext
	secondaryDisplayState[SCOREBOARD_HEIGHT+3] = '*' + std::string(longestPlayerName, 'x') + "s tur*";

	//initialisera index 9-11
	secondaryDisplayState[9] = ""; //debug
	secondaryDisplayState[10] = ""; //progress-bar (inte implementerad)
	secondaryDisplayState[11] = ""; //tid för dator-drag

	//nu har alla index i secondarydisplaystate (0-11) initialiseras

	//initialisera tid för senaste bild-uppdatering
	lastFrameUpdate = std::chrono::system_clock::now();
}

//till för att huvud-filen ska koppla spelbrädet med render-motorn
void render::setBoard(Board* board)
{
	renderedBoardPtr = board;
}

//till för att huvud-filen ska koppla inställningar med render-motorn
void render::setSettings(GameSettings* settings)
{
	gameSettingsPtr = settings;
}

//visar aktuella inställningar på skärmen
//parametern updatescreen är ifall skärman ska ritas om
void render::updateSettings(bool updateScreen)
{
	if (gameSettingsPtr == nullptr) { return; } //om inställningar inte är kopplade finns det inget att göra

	//Sätter AI/människa-status på spelare 1 (svart)
	//kom ihåg, formatet är "x - svart (AI)" alternativ "x - svart", därför kan vi avgöra
	//vad som visas enbart genom att kolla på sista karaktären
	//om spelare 1 är AI och det inte redan står att spelare 1 är AI
	if (gameSettingsPtr->player1isComp && secondaryDisplayState[SCOREBOARD_HEIGHT].back() != ')') {
		secondaryDisplayState[SCOREBOARD_HEIGHT] += '(' + NAME_COMPUTER + ')'; //lägg helt enkelt till "(AI)"
	}
	//om spelare 1 är människa och det står "(AI)", då vill vi ta bort "(AI)" 
	else if (!(gameSettingsPtr->player1isComp) && secondaryDisplayState[SCOREBOARD_HEIGHT].back() == ')') {
		//vi ersätter "(AI)" med "   "
		//vi tar alltså tekniskt sett inte bort någon text, eftersom vi inte vill ha några artifakter
		//vi använder NAME_COMPUTER (ex. "AI") och +-2 för parenteserna
		secondaryDisplayState[SCOREBOARD_HEIGHT].
			replace(secondaryDisplayState[SCOREBOARD_HEIGHT].end() - NAME_COMPUTER.length() - 2,
			secondaryDisplayState[SCOREBOARD_HEIGHT].end(), NAME_COMPUTER.length() + 2, ' ');
	}

	//om ex. spelare 1 är AI men det redan står AI behöver ingen ändring ske

	//vi gör exakt samma sak för spelare 2 / vit med skillnaden SCOREBOARD_HEIGHT+1 istället för bara SCOREBOARD_HEIGHT
	//och player2iscomp istället för player1iscomp
	if (gameSettingsPtr->player2isComp && secondaryDisplayState[SCOREBOARD_HEIGHT+1].back() != ')') {
		secondaryDisplayState[SCOREBOARD_HEIGHT+1] += '(' + NAME_COMPUTER + ')';
	}
	else if (!(gameSettingsPtr->player2isComp) && secondaryDisplayState[SCOREBOARD_HEIGHT+1].back() == ')') {
		secondaryDisplayState[SCOREBOARD_HEIGHT+1].
			replace(secondaryDisplayState[SCOREBOARD_HEIGHT+1].end() - NAME_COMPUTER.length() - 2,
				secondaryDisplayState[SCOREBOARD_HEIGHT+1].end(), NAME_COMPUTER.length() + 2, ' ');
	}

	//uppdatera skärm (och animationer) ifall updatescreen == true
	if (updateScreen) updateScreenAndAnimations();
}

//den här funktionen uppdaterar brädet symboler
//baserat på vilka rutor som är ockuperade
//den uppdaterar också poängtavlan och vems tur det är
void render::updateBoard(bool updateScreen)
{
	if (renderedBoardPtr == nullptr) { return; } //avsluta funktionen om inget bräde har kopplats

	//iterera alla rutor (8x8)
	for (int row = 0; row < 8; ++row) {
		for (int col = 0; col < 8; ++col) {
			//switcha status på brädet som är en char, där 'a' = tom, 'b' = 
			switch (renderedBoardPtr->discs[row][col])
			{
			case 'a': //om tom sätt displaystate på koordinaten till emptysymbol
				boardDisplayState[row][col] = emptySymbol;
				break;
			case 'b': //om svart sätt displaystate på koordinaten till blacksymbol
				boardDisplayState[row][col] = blackSymbol;
				break;
			case 'c': //om vit sätt displaystate på koordianten till blacksymbol
				boardDisplayState[row][col] = whiteSymbol;
				break;		
			default: //om status på ruta är någonting annat har ett fel inträffat
				//detta borde dock aldrig hända dock
				std::cout << "ERROR - ett fel har inträffat och brädet kan inte uppdateras";
				return;
			}
		}
	}

	//Dags att uppdatera poängställningen
	//formatet är XX - svart (AI) där XX kan vara 1 eller 2 karaktärer

	//vi lagrar all text efter XX i en temporär variabel
	//eftersom XX kan vara eller två karaktärer kollar vi första karaktären på index 1
	//är index 1 ett mellanrum kan vi konstatera att XX är en karaktär, annars två.
	std::string tPlayerInfo = secondaryDisplayState[SCOREBOARD_HEIGHT].substr(secondaryDisplayState[SCOREBOARD_HEIGHT][1] == ' ' ? 1 : 2);

	//konvertera antal brickor svart har till en string och lägg till temp-variabeln, detta blir då det nya värdet i displaystaten
	secondaryDisplayState[SCOREBOARD_HEIGHT] = std::to_string(renderedBoardPtr->numberOfDiscs[0]) + tPlayerInfo;

	//gör samma sak för vit
	tPlayerInfo = secondaryDisplayState[SCOREBOARD_HEIGHT+1].substr(secondaryDisplayState[SCOREBOARD_HEIGHT+1][1] == ' ' ? 1 : 2);
	secondaryDisplayState[SCOREBOARD_HEIGHT+1] = std::to_string(renderedBoardPtr->numberOfDiscs[1]) + tPlayerInfo;

	//uppdatera vems tur det är
	secondaryDisplayState[SCOREBOARD_HEIGHT + 3] = '*'  
		+(renderedBoardPtr->isBlacksTurn ? NAME_PLAYER_1 : NAME_PLAYER_2)  //ex svart eller vit
		+ "s tur*" 
		+ std::string(abs( int(NAME_PLAYER_2.length()-NAME_PLAYER_1.length())), ' '); //oavsett vad måste stringen alltid
	//	vara lika lång (för att undvika artifakter). Därför lägger vi till ett antal mellanrum som motsvarar skillnaden 
	// mellan namn på spelare 1 och 2

	possibleMovesBuffer.clear(); //Eftersom vi skriver över hela brädet när vi anropar updateBoard()
	//kommer också alla möjliga drag försvinna, därför rensar vi buffern.

	//uppdatera skärm och animationer om parametern updatescreen == true
	if (updateScreen) updateScreenAndAnimations();
}

//försvinner vid uppdatering av brädet 
//*här skulle man kunna lägga till animationer*
//den här funktionen uppdatera de drag som visas på skärmen som möjliga drag
void render::updatePossibleMoves(std::vector<GameCoordinates> moves, bool updateScreen)
{
	//första rensar vi de gamla möjliga dragen (om sådana finns)
	for (GameCoordinates coords : possibleMovesBuffer) 
		boardDisplayState[coords.y][coords.x] = emptySymbol;

	//sedan uppdaterar vi de nya möjliga dragen
	for (GameCoordinates coords : moves)
		boardDisplayState[coords.y][coords.x] = possibleSymbol;

	//uppdatera buffern
	possibleMovesBuffer = moves;

	//uppdatera skärm och animationer om updatescreen == true
	if (updateScreen) updateScreenAndAnimations();
}

//försvinner vid uppdatering av brädet
//den här funktionen updaterar vilken ruta som visas som senast gjorda drag
void render::updateSelectedSquare(GameCoordinates square)
{
	//återställ rutan som förut var markerad. Buffen lagrar vilken information som var där förut.
	if (!selectedMoveBuffer.empty()) boardDisplayState[selectedMove.y][selectedMove.x] = selectedMoveBuffer;

	//om den nya valda rutan är utanför spelplanen (ex. {-1,-1}) sätter vi buffern till tom
	if (square.isInValid()) {
		selectedMoveBuffer = "";
	}
	//annars uppdaterar vi den markerade rutan
	else {
		selectedMove = square; //kom ihåg vilken ruta som är markerad för nästa gång funktionen anropas

		selectedMoveBuffer = boardDisplayState[selectedMove.y][selectedMove.x]; //uppdatera buffern

		//uppdatera rutan som är markerad med style för markerad ruta + buffern + återställ all style + brädbakgrundsstyle
		boardDisplayState[selectedMove.y][selectedMove.x] = selectModifier + selectedMoveBuffer + ANSI_DEFAULT + ANSI_BOARD_BG;
	}


	//uppdatera skärm och animationer
	updateScreenAndAnimations();
}

//försvinner vid uppdatering av brädet
//den här funktioner uppdaterar den ruta där ett drag gjordes senast
void render::updateLastMove(GameCoordinates move, bool updateScreen)
{	
	//uppdatera displaystate till highlight-symbolen för den färg som lade draget,
	//om det är svarts tur nu, så var det förra draget vit
	boardDisplayState[move.y][move.x] = renderedBoardPtr->isBlacksTurn ? highlightWhiteSymbol : highlightBlackSymbol;
	
	//uppdatera skärm och animationer om updatescreen == true
	if (updateScreen) updateScreenAndAnimations();
}

//den här funktionen kan användas för att enkelt visa information under programmets körtid
void render::updateDebugText(std::string text, bool updateScreen)
{
	//kom ihåg, vi vill inte ha några artifakter, därför kan vi inte korta ner stringen
	//därför kollar vi skillnaden på den gamla texten (den som står där nu) och den nya texten (den vi vill sätta där)
	int differenceOldNewLength = secondaryDisplayState[9].length() - text.length();

	//uppdatera texten i displaystate
	secondaryDisplayState[9] = text;

	//om skillnaden var positiv, alltså att den gamla texten var längre än den nya texten
	//kompensera för mellanskillanden genom att lägga till så många mellanrum
	if (differenceOldNewLength > 0) secondaryDisplayState[9] += std::string(differenceOldNewLength, ' ');

	//uppdatera skärm och animationer om updatescreen == true
	if (updateScreen) updateScreenAndAnimations();
}

//den här funktionen används för att vissa hur långt datorn har kommit i sitt försök att hitta ett drag
//i nuläget är det enda implementerade att mäta hur lång tid ett drag tar att göra.
void render::updateComputerProgress(ComputerProgress progress)
{
	using namespace std::chrono; //så vi slipper skriva std::chrono

	static time_point<system_clock> start; //tidpunkt då draget påbörjades. Static så den bibehålls mellan funktionsanrop

	//checkedmoves == 0 indikerar att draget nyss påbörjades
	if (progress.checkedMoves == 0) start = system_clock::now(); //sätt start-variabeln

	//checkedmoves == 1 indikerar att draget nyss avslutades
	if (progress.checkedMoves == 1) 
		//räkna ut hur lång tid draget tog och lägg till det i displaystate med lite extra luft för att undvika artifakter
		secondaryDisplayState[11] = "senaste AI-draget tog: " + std::to_string(duration_cast<milliseconds>(system_clock::now() - start).count()) + " ms         ";
	
	//uppdatera skärm och animationer
	updateScreenAndAnimations();
}

//skapa en överlagringstext som ska visas i ett visst antal millisekunder
//returnwhenfinished indikerar om funktionen ska vänta tills texten utgått eller returnera direkt
void render::splashText(std::string text, int durationMs, bool returnWhenFinished)
{
	using namespace std::chrono; //så vi slipper skriva std::chrono

	//standardstylen för överlagringstext
	splashTextStyle = RGB_WHITE_FG + RGB_BLACK_BG;

	//uppdatera texten
	splashTextStr = text;

	//om texten är kortare än bredden på brädet, sätt margin så att texten blir centrerad runt brädet
	if (splashTextStr.length() < 16) splashLeftMargin = (16 - splashTextStr.length()) / 2;
	//annars låt den vara 0
	else splashLeftMargin = 0;

	//uppdatera utgångstiden genom att lägga på varaktigheten på tidpunkten just nu
	splashExpiryTime = system_clock::now() + milliseconds(durationMs);

	//uppdatera skärm och animationer
	updateScreenAndAnimations();

	if (returnWhenFinished) {
		//vänta tills texten har gått ut
		while (duration_cast<milliseconds>(splashExpiryTime - system_clock::now()) > milliseconds(0)) {
			//medan vi väntar, uppdaterar vi animationer, men ingen annan kod körs
			updateScreenAndAnimations();
		}
	}
}

//den här funktionen återställer skärmen,
//användbart om man till exempel sätter fönstret i helskärm och färgen ser "trasig ut"
void render::restoreScreen()
{
	system("cls"); //rensa skärmen
	std::cout << "\033[?25l"; //göm skriv-markören
	updateScreenAndAnimations(); //skriv ut skärmen igen
}
