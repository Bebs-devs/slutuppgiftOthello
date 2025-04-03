#include "clRender.h" //h�r finns alla deklarationer som anv�nds av huvud-filen
#include <iostream> //cout
#include <vector> //std::vector
#include <string> //std::string
#include <chrono> //animationer, FPS, tid f�r datordrag

//det h�r �r den s� kallade render-motorn
//som sk�ter n�stan all utmatning
//f�r mer information om anv�ndning, se header-filen

using namespace render; //ett custom namespace som finns i header-filen

//Globala konstanter

//ANSI-koder, fr�mst f�r f�rger. Notera att detta inte �r alla ANSI-koder som programmet anv�nder
//RGB signalerar att koderna f�ljer formatet "\033[{3/4}8;2;{r};{g};{b}m"
static const std::string ANSI_BOARD_BG = "\033[48;2;24;140;27m", ANSI_DEFAULT = "\033[0m", 
RGB_WHITE_FG = "\033[38;2;255;255;255m", RGB_BLACK_FG = "\033[38;2;0;0;0m", RGB_BLACK_BG = "\033[48;2;0;0;0m";

//V�rden f�r m�ngd luft spelbr�det
static const int LEFT_PADDING = 2, TOP_PADDING = 1, RIGHT_PADDING = 1, BOTTOM_PADDING = 0, SCOREBOARD_HEIGHT = 2;

//Namn p� spelare och dator som visas i scoreboarden till h�ger om br�det
static const std::string NAME_PLAYER_1 = "Svart", NAME_PLAYER_2 = "Vit", NAME_COMPUTER = "AI";

//symboler f�r utmatning, m�ste vara tv� karakt�r i bredd (bortsedd fr�n Ansi-koder)
static const std::string emptySymbol = "  ", //f�r rutor inte ockuperade
blackSymbol = "\033[30m X", whiteSymbol = "\033[37m O", //f�r rutor ockuperade av svart eller vit
highlightBlackSymbol = "\033[33m X", highlightWhiteSymbol = "\033[33m O", //symboler f�r d�r senaste draget lades
possibleSymbol = "\033[38;5;27m +", //symbol f�r alla rutor som �r m�jliga att l�gga i
selectModifier = "\033[48;5;246m"; //inte en symbol, utan en ANSI-kod som �ndrar hur det ser ut n�r man markerar en ruta

//Globala variabler
//dessa h�ller koll p� vad som visas eller kommer att visas p� sk�rmen
static std::chrono::time_point<std::chrono::system_clock> lastFrameUpdate; //n�r bilden uppdaterades senast
static std::string boardDisplayState[8][8]; //inneh�ller br�det (8x8)
static std::string secondaryDisplayState[12]; //inneh�ller �vrig information p� spelplanen
//titel p� [0], debug p� [9], datorinfo p� [11] exempelvis

static Board* renderedBoardPtr = nullptr; //pekare till det aktiva spelbr�det
static GameSettings* gameSettingsPtr = nullptr; //pekare till aktiva inst�llningar

static std::vector<GameCoordinates> possibleMovesBuffer = {}; //h�r lagras listan �ver m�jliga drag

static GameCoordinates selectedMove = { -1,-1 }; //markerad ruta
static std::string selectedMoveBuffer = ""; //lagrar vad som fanns p� den markerade rutan f�rut

static std::string splashTextStr = ""; //lagrar vilken �verlagringstext som just nu visas p� sk�rmen
static std::string splashTextStyle = "\033[38;2;255;255;255m\033[48;2;0;0;0m"; //hur �verlagringstexten ser ut
static int splashLeftMargin = 7; //var �verlagringstexten b�rjar
static std::chrono::time_point<std::chrono::system_clock> splashExpiryTime; //n�r �verlagringstexten slutar synas

// funktion f�r att skriva ut �verlagringstext, anv�nds av print()
// f�ruts�tter att utskrift av splashtext inte redan skett 
// returnerar true om text har skrivits ut
// �ndrar column-variabeln till sista kolumnen utskrift har skett
static bool printSplashText(int & column){
	if (splashTextStr.empty()) return false; //finns det ingen �verlagringstext, kan ingen utskrift ske

	std::string tempSplashTextStr = splashTextStr; //tempor�r variabel eftersom en vi beh�ver att
	//l�ngden p� utskriften �r ett j�mnt antal karakt�rer, d� rutorna p� br�det �r tv� karakt�rer breda
	
	//Index  0 1 2 3 4 5 6 7 8 9 ...
	//Kolumn  0   1   2   3   4 ...
	//Margin 1 2 3 4 5 ...
	//om f�rsta index i n�sta kolumn �r mer �n margin, d� �r det dags att skriva ut
	if (2 * (column + 1) > splashLeftMargin) {
		//om margin �r udda, l�gg till en tom karakt�r i b�rjan som padding
		//anledning till temp-variabel �r s� vi inte l�gger till padding
		//till den faktiska text-variabeln
		if (splashLeftMargin % 2) tempSplashTextStr = ' ' + tempSplashTextStr;

		//p� likande sett l�gger vi till ett mellanrum i slutet om det beh�vs f�r ett j�mnt antal karakt�rer
		if (tempSplashTextStr.length() % 2) tempSplashTextStr += ' ';
		
		//skriv ut texten, med r�tt style, samt �terg� till vanliga stylen efter
		std::cout << splashTextStyle + tempSplashTextStr + ANSI_BOARD_BG;

		//hoppa fram till slutet av �verlagringstexten
		//notera -1, detta �r f�r att print forts�tter ytterligare en kolumn fram
		column += (tempSplashTextStr.length()-1) / 2;
		
		return true; //returnera true, eftersom text skrevs ut
	}

	return false; //returnera false, d� ingen text skrevs ut
}

//funktionen som skriver ut displayState p� sk�rmen
static void print() {
	//notera att string(int i, char c) skapar en string av l�ngd i d�r varje karakt�r �r c

	//h�ller koll p� om �verlagringstext har skrivits ut �n
	bool splashTextWritten = false;

	std::cout << "\033[H"; //g� till "hem-positionen", allts� �versta v�nstra h�rnet

	//skriver ut luft ovanf�r spelplanen
	std::cout << std::string(TOP_PADDING, '\n'); 

	//skriver ut luft till v�nster + titeln
	std::cout << std::string(LEFT_PADDING, ' ') + secondaryDisplayState[0] << '\n';

	//itererar linje 1-8, vilket motsvarar 0-7 i boardDisplayState
	for (int lineIndex = 1; lineIndex < 9; ++lineIndex) {
		//skriver ut luft och �ndrar style till bakgrunden
		std::cout << std::string(LEFT_PADDING, ' ') + ANSI_BOARD_BG;

		//iterera genom kolumner
		for (int column = 0; column < 8; ++column) {
			//�verlagringstext skrivs ut p� raden mellan "po�ng - vit" och *X tur*
			//Scoreboard_height �r index f�r linjen d�r "po�ng - svart" skrivs ut
			//d�rav, om r�tt linje OCH �verlagringstext inte skrivits ut �nnu
			if (lineIndex == SCOREBOARD_HEIGHT+2 && !splashTextWritten) {
				//d� ser vi om vi kan skriva ut n�gon text, kunde vi det
				if (printSplashText(column)) {
					//�ndrar vi variablen till true
					splashTextWritten = true;
					//och g�r till n�sta iteration av kolumnera, som d� kommer addera ett till variablen column
					continue;
				}
			}

			//skrevs ingen text ut nyss, kan vi skriva ut rutan
			//lineIndex - 1 eftersom lineindex g�r mellan 1 och 8
			std::cout << boardDisplayState[lineIndex - 1][column];
		}
		
		//skriver ut luft efter br�det och det som eventuellt fins i secondaryDisplayState p� denna rad samt g�r till n�sta rad
		std::cout << ANSI_DEFAULT + std::string(RIGHT_PADDING, ' ') + secondaryDisplayState[lineIndex] << '\n';
	}

	//nu har br�det skrivits ut, d� skapar vi lite luft under
	std::cout << std::string(BOTTOM_PADDING, '\n');

	//iterera de linnjer som �r kvar (9-11)
	for (int lineIndex = 9; lineIndex < 12; ++lineIndex) {
		//skriv ut luft, information och g� till n�sta rad
		std::cout << std::string(LEFT_PADDING, ' ') + secondaryDisplayState[lineIndex] << '\n';
	}

	//uppdatera tiden f�r senaste bilduppdateringen
	std::chrono::time_point<std::chrono::system_clock> tLastFrameUpdate = lastFrameUpdate;
	lastFrameUpdate = std::chrono::system_clock::now();

	//med hj�lp av f�ljande kod kan man se tiden mellan bildrutor
	//updateDebugText(std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(lastFrameUpdate - tLastFrameUpdate).count()), false);
}

// funktion f�r att linj�rt interpolera, allts� ta en punkt mellan, tv� f�rger i ANSI-format
// parametern pointBetween ska vara mellan 0 och 1, d�r 0 blir lika med startf�rg och 1 lika med slutf�rg
static std::string interpolateColor(std::string startColorAnsi, std::string endColorAnsi, float pointBetween) {
	//input m�ste vara antingen i format "\033[48;2;X;X;Xm eller "\033[38;2;X;X;Xm" d�r X �r heltal mellan 0 och 255
	//38 eller 48 avg�r f�rgrund eller bakgrund

	//f�rsta m�ste vi ta ut siffrorna ur ANSI-koden, som dessutom kan vara olika karakt�rer l�nga
	//d�rf�r anv�nder vi substrings f�r att ta alla karakt�rer fram tills separeringskarakt�ren (; eller m)

	std::string beginningCode = startColorAnsi.substr(0,7); //ansi-koden b�rjan, ex "\033[38;2;"
	std::string remainingCode = startColorAnsi.substr(7); //resten av koden, ex "25;255;0m"

	//vi extraherar r�d i startkoden
	int nextSeperatorIndex = remainingCode.find(';'); //var finns n�sta semikolon ex index 2
	int startRed = std::stoi(remainingCode.substr(0, nextSeperatorIndex)); //ta karakt�rerna fram till semikolon, ex "25", konvertera till int, ex 25
	remainingCode = remainingCode.substr(nextSeperatorIndex+1); //resten av koden, ex "255;0m"

	//samma f�r gr�n i startkoden
	nextSeperatorIndex = remainingCode.find(';'); //ex index 3
	int startGreen = std::stoi(remainingCode.substr(0, nextSeperatorIndex)); //ex 255
	remainingCode = remainingCode.substr(nextSeperatorIndex+1); // ex "0m"

	//samma f�r bl�, fast med 'm' ist�llet f�r ';'
	nextSeperatorIndex = remainingCode.find('m'); //ex index 1
	int startBlue = std::stoi(remainingCode.substr(0, nextSeperatorIndex)); //ex 0
	//och d�r �r vi klara med start-koden

	//dags f�r slutkoden
	//exakt samma, men vi �ndrar inte beginningCode, eftersom vi f�ruts�tter att den �r samma
	//�r den inte det �r det f�rsta f�rgkoden som avg�r om det blir f�r- eller bakgrund
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

	//formeln �r interpolation = start + (end - start) * pointBetween
	//vi utf�r formeln separat p� alla tre f�rger
	int red = startRed + (endRed - startRed) * pointBetween;
	int green = startGreen + (endGreen - startGreen) * pointBetween;
	int blue = startBlue + (endBlue - startBlue) * pointBetween;
	
	//sedan s�tter vi ihop det till en ny kod och returnerar den
	return beginningCode + std::to_string(red) + ';' + std::to_string(green) + ';' + std::to_string(blue) + 'm';
}


//den h�r funktionen uppdaterar animationer
//observera att den inte kan anropas utifr�n en annan fil eftersom den �r static
//samt att den inte uppdaterar sk�rmen (den anropar inte print)
//i nul�get �r det bara �verlagringstexten som �r animerad, men det �r inte �verdrivet kr�ngligt att l�gga till mer
static void updateAnimations()
{
	using namespace std::chrono; //s� vi slipper skriva std::chrono �verallt

	//konstanter som avg�r hur snabba animationerna �r
	//splashtextspeed �r millisekunder / tv� bokst�ver, splashcolorspeed �r ms p� hela �verg�ngen
	const milliseconds splashTextSpeed = milliseconds(30), splashColorSpeed = milliseconds(300);

	//ALL ANIMATION F�R SPLASH TEXT
	//hur mycket tid det �r kvar tills �verlagringstexten ska vara helt borta
	milliseconds timeToSplashExpiry = duration_cast<milliseconds>(splashExpiryTime - system_clock::now());

	//hur l�ng texten f�r vara. Ex. 10 sekunder kvar, och splashtextspeed �r 1 sekund, till�ten l�ngd �r 10 karakt�rer.
	//n�r det �r 0 sekunder kvar (allts� n�r texten ska vara helt borta) �r till�ten l�ngd 0 karakt�rer.
	int allowedSplashLength = timeToSplashExpiry / splashTextSpeed;

	//har tiden redan g�tt ut, ser vi bara till att texten faktiskt �r tom
	//detta �r s� vi inte beh�ver kr�ngla med negativa v�rden
	if (timeToSplashExpiry < milliseconds(0)) {
		splashTextStr = "";
	}
	//annars kollar vi l�ngden p� texten �verstiger maxgr�nsen
	else if (splashTextStr.length() > allowedSplashLength) {
		//is�fall skjuter vi fram texten ett steg �t h�ger (s� att den h�lls centrerad)
		splashLeftMargin++;

		//och tar bort f�rsta och sista bokstaven
		//vi tar length-2 efter det argumentet �r f�r hur l�ng texten ska vara, inte index
		//som man skulle kunna tro.
		splashTextStr = splashTextStr.substr(1,splashTextStr.length()-2);
	}

	//Dags att animera f�rgen
	//om tiden som �r kvar �r mindre �n tiden f�r hela �verg�ngen samt att tiden inte redan har utg�tt
	if (timeToSplashExpiry < splashColorSpeed && timeToSplashExpiry >= milliseconds(0)) {
		//r�kna ut andelen tid mellan b�rjan och slutet p� �verg�ngen
		//formel: 1 - (tidkvar / tidhela)
		float colorProgress = 1.0f - timeToSplashExpiry.count() / (float)splashColorSpeed.count();

		//stylen �r en kombination av bakgrund (BG) och f�rgrund (FG)
		//notera att i det f�rsta funktionsanropen �r FG och BG kombinerat, men det �r bara
		//den f�rsta som best�mmer om det �r FG eller BG
		splashTextStyle = interpolateColor(RGB_WHITE_FG, ANSI_BOARD_BG, colorProgress) + interpolateColor(RGB_BLACK_BG, ANSI_BOARD_BG, colorProgress);
	}
	
}

//den h�r funktionen �r till att anv�ndas av restan av programmet f�r att uppdatera sk�rmen
//om programmet ex. v�ntar p� input
void render::updateScreenAndAnimations() {
	updateAnimations(); //uppdatera animationer
	print(); //visa animationerna
}

//den h�r funktionen m�ste kallas innan �vrig uppdatering av sk�rmen sker
//den initialiserar de variabler som inneh�ller displaystate
void render::init() {

	std::cout << "\033[?25l"; //g�m skriv-mark�ren

	secondaryDisplayState[0] = " O T H E L L O"; //best�m titeln

	//g� igenom hela br�det (8x8) och initialisera varje ruta
	//samt initialisera secondarydisplaystate index 1-8
	for (int lineIndex = 1; lineIndex <= 8; ++lineIndex) {
		secondaryDisplayState[lineIndex] = "";
		for (int column = 0; column < 8; ++column) {
			//notera lineindex-1 eftersom lineindex g�r fr�n 1 till 8
			boardDisplayState[lineIndex - 1][column] = emptySymbol;
		}
	}

	//initialisera po�ngtavlan
	secondaryDisplayState[SCOREBOARD_HEIGHT] = "x - " + NAME_PLAYER_1 + " (" + NAME_COMPUTER + ")";
	secondaryDisplayState[SCOREBOARD_HEIGHT+1] = "x - " + NAME_PLAYER_2 + " (" + NAME_COMPUTER + ")";

	//vi vill inte ha n�gra artifakter (kom ih�g att sk�rmen inte rensas, utan bara skrivs �ver)
	//d�rf�r r�knar vi ut vad det l�ngsta spelarnamnet �r (ex. "svart" vs "vit" blir "svart" allts� 5)
	int longestPlayerName = std::max(NAME_PLAYER_1.length(), NAME_PLAYER_2.length());
	//observera ocks� att index scoreboard height + 2 �r reserverad f�r �verlagringstext
	secondaryDisplayState[SCOREBOARD_HEIGHT+3] = '*' + std::string(longestPlayerName, 'x') + "s tur*";

	//initialisera index 9-11
	secondaryDisplayState[9] = ""; //debug
	secondaryDisplayState[10] = ""; //progress-bar (inte implementerad)
	secondaryDisplayState[11] = ""; //tid f�r dator-drag

	//nu har alla index i secondarydisplaystate (0-11) initialiseras

	//initialisera tid f�r senaste bild-uppdatering
	lastFrameUpdate = std::chrono::system_clock::now();
}

//till f�r att huvud-filen ska koppla spelbr�det med render-motorn
void render::setBoard(Board* board)
{
	renderedBoardPtr = board;
}

//till f�r att huvud-filen ska koppla inst�llningar med render-motorn
void render::setSettings(GameSettings* settings)
{
	gameSettingsPtr = settings;
}

//visar aktuella inst�llningar p� sk�rmen
//parametern updatescreen �r ifall sk�rman ska ritas om
void render::updateSettings(bool updateScreen)
{
	if (gameSettingsPtr == nullptr) { return; } //om inst�llningar inte �r kopplade finns det inget att g�ra

	//S�tter AI/m�nniska-status p� spelare 1 (svart)
	//kom ih�g, formatet �r "x - svart (AI)" alternativ "x - svart", d�rf�r kan vi avg�ra
	//vad som visas enbart genom att kolla p� sista karakt�ren
	//om spelare 1 �r AI och det inte redan st�r att spelare 1 �r AI
	if (gameSettingsPtr->player1isComp && secondaryDisplayState[SCOREBOARD_HEIGHT].back() != ')') {
		secondaryDisplayState[SCOREBOARD_HEIGHT] += '(' + NAME_COMPUTER + ')'; //l�gg helt enkelt till "(AI)"
	}
	//om spelare 1 �r m�nniska och det st�r "(AI)", d� vill vi ta bort "(AI)" 
	else if (!(gameSettingsPtr->player1isComp) && secondaryDisplayState[SCOREBOARD_HEIGHT].back() == ')') {
		//vi ers�tter "(AI)" med "   "
		//vi tar allts� tekniskt sett inte bort n�gon text, eftersom vi inte vill ha n�gra artifakter
		//vi anv�nder NAME_COMPUTER (ex. "AI") och +-2 f�r parenteserna
		secondaryDisplayState[SCOREBOARD_HEIGHT].
			replace(secondaryDisplayState[SCOREBOARD_HEIGHT].end() - NAME_COMPUTER.length() - 2,
			secondaryDisplayState[SCOREBOARD_HEIGHT].end(), NAME_COMPUTER.length() + 2, ' ');
	}

	//om ex. spelare 1 �r AI men det redan st�r AI beh�ver ingen �ndring ske

	//vi g�r exakt samma sak f�r spelare 2 / vit med skillnaden SCOREBOARD_HEIGHT+1 ist�llet f�r bara SCOREBOARD_HEIGHT
	//och player2iscomp ist�llet f�r player1iscomp
	if (gameSettingsPtr->player2isComp && secondaryDisplayState[SCOREBOARD_HEIGHT+1].back() != ')') {
		secondaryDisplayState[SCOREBOARD_HEIGHT+1] += '(' + NAME_COMPUTER + ')';
	}
	else if (!(gameSettingsPtr->player2isComp) && secondaryDisplayState[SCOREBOARD_HEIGHT+1].back() == ')') {
		secondaryDisplayState[SCOREBOARD_HEIGHT+1].
			replace(secondaryDisplayState[SCOREBOARD_HEIGHT+1].end() - NAME_COMPUTER.length() - 2,
				secondaryDisplayState[SCOREBOARD_HEIGHT+1].end(), NAME_COMPUTER.length() + 2, ' ');
	}

	//uppdatera sk�rm (och animationer) ifall updatescreen == true
	if (updateScreen) updateScreenAndAnimations();
}

//den h�r funktionen uppdaterar br�det symboler
//baserat p� vilka rutor som �r ockuperade
//den uppdaterar ocks� po�ngtavlan och vems tur det �r
void render::updateBoard(bool updateScreen)
{
	if (renderedBoardPtr == nullptr) { return; } //avsluta funktionen om inget br�de har kopplats

	//iterera alla rutor (8x8)
	for (int row = 0; row < 8; ++row) {
		for (int col = 0; col < 8; ++col) {
			//switcha status p� br�det som �r en char, d�r 'a' = tom, 'b' = 
			switch (renderedBoardPtr->discs[row][col])
			{
			case 'a': //om tom s�tt displaystate p� koordinaten till emptysymbol
				boardDisplayState[row][col] = emptySymbol;
				break;
			case 'b': //om svart s�tt displaystate p� koordinaten till blacksymbol
				boardDisplayState[row][col] = blackSymbol;
				break;
			case 'c': //om vit s�tt displaystate p� koordianten till blacksymbol
				boardDisplayState[row][col] = whiteSymbol;
				break;		
			default: //om status p� ruta �r n�gonting annat har ett fel intr�ffat
				//detta borde dock aldrig h�nda dock
				std::cout << "ERROR - ett fel har intr�ffat och br�det kan inte uppdateras";
				return;
			}
		}
	}

	//Dags att uppdatera po�ngst�llningen
	//formatet �r XX - svart (AI) d�r XX kan vara 1 eller 2 karakt�rer

	//vi lagrar all text efter XX i en tempor�r variabel
	//eftersom XX kan vara eller tv� karakt�rer kollar vi f�rsta karakt�ren p� index 1
	//�r index 1 ett mellanrum kan vi konstatera att XX �r en karakt�r, annars tv�.
	std::string tPlayerInfo = secondaryDisplayState[SCOREBOARD_HEIGHT].substr(secondaryDisplayState[SCOREBOARD_HEIGHT][1] == ' ' ? 1 : 2);

	//konvertera antal brickor svart har till en string och l�gg till temp-variabeln, detta blir d� det nya v�rdet i displaystaten
	secondaryDisplayState[SCOREBOARD_HEIGHT] = std::to_string(renderedBoardPtr->numberOfDiscs[0]) + tPlayerInfo;

	//g�r samma sak f�r vit
	tPlayerInfo = secondaryDisplayState[SCOREBOARD_HEIGHT+1].substr(secondaryDisplayState[SCOREBOARD_HEIGHT+1][1] == ' ' ? 1 : 2);
	secondaryDisplayState[SCOREBOARD_HEIGHT+1] = std::to_string(renderedBoardPtr->numberOfDiscs[1]) + tPlayerInfo;

	//uppdatera vems tur det �r
	secondaryDisplayState[SCOREBOARD_HEIGHT + 3] = '*'  
		+(renderedBoardPtr->isBlacksTurn ? NAME_PLAYER_1 : NAME_PLAYER_2)  //ex svart eller vit
		+ "s tur*" 
		+ std::string(abs( int(NAME_PLAYER_2.length()-NAME_PLAYER_1.length())), ' '); //oavsett vad m�ste stringen alltid
	//	vara lika l�ng (f�r att undvika artifakter). D�rf�r l�gger vi till ett antal mellanrum som motsvarar skillnaden 
	// mellan namn p� spelare 1 och 2

	possibleMovesBuffer.clear(); //Eftersom vi skriver �ver hela br�det n�r vi anropar updateBoard()
	//kommer ocks� alla m�jliga drag f�rsvinna, d�rf�r rensar vi buffern.

	//uppdatera sk�rm och animationer om parametern updatescreen == true
	if (updateScreen) updateScreenAndAnimations();
}

//f�rsvinner vid uppdatering av br�det 
//*h�r skulle man kunna l�gga till animationer*
//den h�r funktionen uppdatera de drag som visas p� sk�rmen som m�jliga drag
void render::updatePossibleMoves(std::vector<GameCoordinates> moves, bool updateScreen)
{
	//f�rsta rensar vi de gamla m�jliga dragen (om s�dana finns)
	for (GameCoordinates coords : possibleMovesBuffer) 
		boardDisplayState[coords.y][coords.x] = emptySymbol;

	//sedan uppdaterar vi de nya m�jliga dragen
	for (GameCoordinates coords : moves)
		boardDisplayState[coords.y][coords.x] = possibleSymbol;

	//uppdatera buffern
	possibleMovesBuffer = moves;

	//uppdatera sk�rm och animationer om updatescreen == true
	if (updateScreen) updateScreenAndAnimations();
}

//f�rsvinner vid uppdatering av br�det
//den h�r funktionen updaterar vilken ruta som visas som senast gjorda drag
void render::updateSelectedSquare(GameCoordinates square)
{
	//�terst�ll rutan som f�rut var markerad. Buffen lagrar vilken information som var d�r f�rut.
	if (!selectedMoveBuffer.empty()) boardDisplayState[selectedMove.y][selectedMove.x] = selectedMoveBuffer;

	//om den nya valda rutan �r utanf�r spelplanen (ex. {-1,-1}) s�tter vi buffern till tom
	if (square.isInValid()) {
		selectedMoveBuffer = "";
	}
	//annars uppdaterar vi den markerade rutan
	else {
		selectedMove = square; //kom ih�g vilken ruta som �r markerad f�r n�sta g�ng funktionen anropas

		selectedMoveBuffer = boardDisplayState[selectedMove.y][selectedMove.x]; //uppdatera buffern

		//uppdatera rutan som �r markerad med style f�r markerad ruta + buffern + �terst�ll all style + br�dbakgrundsstyle
		boardDisplayState[selectedMove.y][selectedMove.x] = selectModifier + selectedMoveBuffer + ANSI_DEFAULT + ANSI_BOARD_BG;
	}


	//uppdatera sk�rm och animationer
	updateScreenAndAnimations();
}

//f�rsvinner vid uppdatering av br�det
//den h�r funktioner uppdaterar den ruta d�r ett drag gjordes senast
void render::updateLastMove(GameCoordinates move, bool updateScreen)
{	
	//uppdatera displaystate till highlight-symbolen f�r den f�rg som lade draget,
	//om det �r svarts tur nu, s� var det f�rra draget vit
	boardDisplayState[move.y][move.x] = renderedBoardPtr->isBlacksTurn ? highlightWhiteSymbol : highlightBlackSymbol;
	
	//uppdatera sk�rm och animationer om updatescreen == true
	if (updateScreen) updateScreenAndAnimations();
}

//den h�r funktionen kan anv�ndas f�r att enkelt visa information under programmets k�rtid
void render::updateDebugText(std::string text, bool updateScreen)
{
	//kom ih�g, vi vill inte ha n�gra artifakter, d�rf�r kan vi inte korta ner stringen
	//d�rf�r kollar vi skillnaden p� den gamla texten (den som st�r d�r nu) och den nya texten (den vi vill s�tta d�r)
	int differenceOldNewLength = secondaryDisplayState[9].length() - text.length();

	//uppdatera texten i displaystate
	secondaryDisplayState[9] = text;

	//om skillnaden var positiv, allts� att den gamla texten var l�ngre �n den nya texten
	//kompensera f�r mellanskillanden genom att l�gga till s� m�nga mellanrum
	if (differenceOldNewLength > 0) secondaryDisplayState[9] += std::string(differenceOldNewLength, ' ');

	//uppdatera sk�rm och animationer om updatescreen == true
	if (updateScreen) updateScreenAndAnimations();
}

//den h�r funktionen anv�nds f�r att vissa hur l�ngt datorn har kommit i sitt f�rs�k att hitta ett drag
//i nul�get �r det enda implementerade att m�ta hur l�ng tid ett drag tar att g�ra.
void render::updateComputerProgress(ComputerProgress progress)
{
	using namespace std::chrono; //s� vi slipper skriva std::chrono

	static time_point<system_clock> start; //tidpunkt d� draget p�b�rjades. Static s� den bibeh�lls mellan funktionsanrop

	//checkedmoves == 0 indikerar att draget nyss p�b�rjades
	if (progress.checkedMoves == 0) start = system_clock::now(); //s�tt start-variabeln

	//checkedmoves == 1 indikerar att draget nyss avslutades
	if (progress.checkedMoves == 1) 
		//r�kna ut hur l�ng tid draget tog och l�gg till det i displaystate med lite extra luft f�r att undvika artifakter
		secondaryDisplayState[11] = "senaste AI-draget tog: " + std::to_string(duration_cast<milliseconds>(system_clock::now() - start).count()) + " ms         ";
	
	//uppdatera sk�rm och animationer
	updateScreenAndAnimations();
}

//skapa en �verlagringstext som ska visas i ett visst antal millisekunder
//returnwhenfinished indikerar om funktionen ska v�nta tills texten utg�tt eller returnera direkt
void render::splashText(std::string text, int durationMs, bool returnWhenFinished)
{
	using namespace std::chrono; //s� vi slipper skriva std::chrono

	//standardstylen f�r �verlagringstext
	splashTextStyle = RGB_WHITE_FG + RGB_BLACK_BG;

	//uppdatera texten
	splashTextStr = text;

	//om texten �r kortare �n bredden p� br�det, s�tt margin s� att texten blir centrerad runt br�det
	if (splashTextStr.length() < 16) splashLeftMargin = (16 - splashTextStr.length()) / 2;
	//annars l�t den vara 0
	else splashLeftMargin = 0;

	//uppdatera utg�ngstiden genom att l�gga p� varaktigheten p� tidpunkten just nu
	splashExpiryTime = system_clock::now() + milliseconds(durationMs);

	//uppdatera sk�rm och animationer
	updateScreenAndAnimations();

	if (returnWhenFinished) {
		//v�nta tills texten har g�tt ut
		while (duration_cast<milliseconds>(splashExpiryTime - system_clock::now()) > milliseconds(0)) {
			//medan vi v�ntar, uppdaterar vi animationer, men ingen annan kod k�rs
			updateScreenAndAnimations();
		}
	}
}

//den h�r funktionen �terst�ller sk�rmen,
//anv�ndbart om man till exempel s�tter f�nstret i helsk�rm och f�rgen ser "trasig ut"
void render::restoreScreen()
{
	system("cls"); //rensa sk�rmen
	std::cout << "\033[?25l"; //g�m skriv-mark�ren
	updateScreenAndAnimations(); //skriv ut sk�rmen igen
}
