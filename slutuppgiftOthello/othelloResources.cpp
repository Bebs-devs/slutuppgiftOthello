#include "othelloResources.h" //h�r finns mer information


//h�ller information om spelare,
//huruvida de �r datorer
//och is�fall vilken sv�righetsgrad de �r satta p�
GameSettings::GameSettings() {
	player1isComp = false;
	player2isComp = false;
	comp1Difficulty = 0;
	comp2Difficulty = 0;
}

//funktion som kollar om en koordinat ligger p� br�det eller inte
//returnerar true om den inte g�r det
bool GameCoordinates::isInValid() {
	//index p� b�de rader och kolumner g�r fr�n 0 till 7
	if (x < 0 || x > 7 || y < 0 || y > 7) return true;
	return false;
}

//inneh�ller alla relativa koordinater f�r att f�rflytta sig ett steg i en viss riktning
//kom ih�g att y-koordinaten kommer f�rst och att {0,0} �r uppe v�nstra h�rnet
const int8 GameCoordinates::translations[8][2] = {
	  {-1,-1},{-1,0 },{-1,1 },{0,-1 },{0,1 },{1,-1 },{1,0 },{1,1 } };
//upp v�nster, upp, upp h�ger, v�nster, h�ger, ner v�nster, ner, ner h�ger (i den ordningen) 

//constructor av board
Board::Board() {
	//den h�r funktionen k�rs n�r ett objekt av typ (klass) Board skapas

	isBlacksTurn = true; //svart b�rjar alltid enligt reglerna

	//antal svarta brickor = antal vita brickor = 2
	numberOfDiscs[0] = numberOfDiscs[1] = 2;

	//iterera alla rutor och s�tt deras status till tom ('a')
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
	
	//specifiera de rutor som �r bredvid en ruta med en bricka i
	//dessa anv�nds f�r att snabbare hitta m�jliga drag
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