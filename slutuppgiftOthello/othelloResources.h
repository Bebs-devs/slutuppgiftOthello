#pragma once //ser till s� att filer inte inkluderas flera g�nger av misstag
typedef signed char int8; //en omskrivning av signed char som anv�nds som ett heltal med ett v�rde -128 till 127

//den h�r filen anv�nds av b�de huvudfilen och render-motorn
//den inneh�ller structs som finns genom hela programmet

//inneh�ller inst�llningar f�r spelarna
struct GameSettings
{
	bool player1isComp, player2isComp; //huruvida spelare 1 respektive spelare 2 �r datorer eller inte
	int comp1Difficulty, comp2Difficulty; //f�rutsatt att spelarna �r datorer, lagrar dessa variabler dess sv�righetsgrad att besegra

	GameSettings(); //h�r deklareras att det finns en constructor, som anv�nds f�r att initialisera v�rdena
};

//ett smart paket som inneh�ller ett par koordinater som ocks� refereras till en ruta eller ett drag
struct GameCoordinates
{
	static const int8 translations[8][2]; //en statisk konstant lista �ver relativa koordinater
	//som anv�nds vid f�rflyttning i olika riktningar. Dessa �r definierade i cpp-filen.

	int y, x; //sj�lva koordinaterna (dessa kanske skulle kunna vara int8 ocks� men det �r oklart om det spelar
		//n�gon st�rre roll)

	//h�r �r en funktion som kollar om koordinaterna �r innanf�r spelplanen
	//anv�nds bland annat i samband med relativa f�rflyttningar
	bool isInValid();
};

//ett smart paket som inneh�ller all information om ett spelbr�de
//det finns ett s�dant f�r br�det som spelarna spelar p� och som visas p� sk�rmen
//dessutom skapas instanser av Board av datorn n�r den testar olika drag f�r att unders�ka vilket som �r b�st
struct Board
{
	bool isBlacksTurn; //br�det h�ller koll p� vems tur det �r
	int numberOfDiscs[2]; //{antal svarta brickor, antal vita brickor}
	char discs[8][8]; //a tom, b svart, c vit
	//[y][x] g�ller f�r alla arrayer som h�ller positioner

	bool adjacents[8][8]; //�r true f�r alla tomma rutor som ligger direkt intill en ockuperat ruta via en av 8 riktningar

	Board(); //h�r deklareras en constructor, som initialiserar br�det till startpositionen i Othello
};

//en struct f�r att h�lla koll p� hur l�ngt datorn har kommit i sitt f�rs�k att hitta ett riktigt klurigt drag
//anv�nds f�r animering och debugging
//totalMoves �r �nnu inte implementerad
struct ComputerProgress
{
	int checkedMoves; //i nul�get: 0 = drag p�b�rjat, 1 = drag avslutat
	int totalMoves; //inte implementerad
};