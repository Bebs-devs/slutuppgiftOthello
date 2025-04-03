#include "othelloResources.h" //här finns mer information


//håller information om spelare,
//huruvida de är datorer
//och isåfall vilken svårighetsgrad de är satta på
GameSettings::GameSettings() {
	player1isComp = false;
	player2isComp = false;
	comp1Difficulty = 0;
	comp2Difficulty = 0;
}

//funktion som kollar om en koordinat ligger på brädet eller inte
//returnerar true om den inte gör det
bool GameCoordinates::isInValid() {
	//index på både rader och kolumner går från 0 till 7
	if (x < 0 || x > 7 || y < 0 || y > 7) return true;
	return false;
}

//innehåller alla relativa koordinater för att förflytta sig ett steg i en viss riktning
//kom ihåg att y-koordinaten kommer först och att {0,0} är uppe vänstra hörnet
const int8 GameCoordinates::translations[8][2] = {
	  {-1,-1},{-1,0 },{-1,1 },{0,-1 },{0,1 },{1,-1 },{1,0 },{1,1 } };
//upp vänster, upp, upp höger, vänster, höger, ner vänster, ner, ner höger (i den ordningen) 

//constructor av board
Board::Board() {
	//den här funktionen körs när ett objekt av typ (klass) Board skapas

	isBlacksTurn = true; //svart börjar alltid enligt reglerna

	//antal svarta brickor = antal vita brickor = 2
	numberOfDiscs[0] = numberOfDiscs[1] = 2;

	//iterera alla rutor och sätt deras status till tom ('a')
	for (int i = 0; i < 8; ++i) {
		for (int j = 0; j < 8; ++j) {
			discs[i][j] = 'a';
		}
	}
	//placera startbrickorna ...
	discs[3][3] = 'c'; //... a a ...
	discs[3][4] = 'b'; //..a c b a..
	discs[4][3] = 'b'; //..a b c a..
	discs[4][4] = 'c'; //... a a ...
	//                       ...
	
	//specifiera de rutor som är bredvid en ruta med en bricka i
	//dessa används för att snabbare hitta möjliga drag
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