#pragma once //ser till så att filer inte inkluderas flera gånger av misstag
typedef signed char int8; //en omskrivning av signed char som används som ett heltal med ett värde -128 till 127

//den här filen används av både huvudfilen och render-motorn
//den innehåller structs som finns genom hela programmet

//innehåller inställningar för spelarna
struct GameSettings
{
	bool player1isComp, player2isComp; //huruvida spelare 1 respektive spelare 2 är datorer eller inte
	int comp1Difficulty, comp2Difficulty; //förutsatt att spelarna är datorer, lagrar dessa variabler dess svårighetsgrad att besegra

	GameSettings(); //här deklareras att det finns en constructor, som används för att initialisera värdena
};

//ett smart paket som innehåller ett par koordinater som också refereras till en ruta eller ett drag
struct GameCoordinates
{
	static const int8 translations[8][2]; //en statisk konstant lista över relativa koordinater
	//som används vid förflyttning i olika riktningar. Dessa är definierade i cpp-filen.

	int y, x; //själva koordinaterna (dessa kanske skulle kunna vara int8 också men det är oklart om det spelar
		//någon större roll)

	//här är en funktion som kollar om koordinaterna är innanför spelplanen
	//används bland annat i samband med relativa förflyttningar
	bool isInValid();
};

//ett smart paket som innehåller all information om ett spelbräde
//det finns ett sådant för brädet som spelarna spelar på och som visas på skärmen
//dessutom skapas instanser av Board av datorn när den testar olika drag för att undersöka vilket som är bäst
struct Board
{
	bool isBlacksTurn; //brädet håller koll på vems tur det är
	int numberOfDiscs[2]; //{antal svarta brickor, antal vita brickor}
	char discs[8][8]; //a tom, b svart, c vit
	//[y][x] gäller för alla arrayer som håller positioner

	bool adjacents[8][8]; //är true för alla tomma rutor som ligger direkt intill en ockuperat ruta via en av 8 riktningar

	Board(); //här deklareras en constructor, som initialiserar brädet till startpositionen i Othello
};

//en struct för att hålla koll på hur långt datorn har kommit i sitt försök att hitta ett riktigt klurigt drag
//används för animering och debugging
//totalMoves är ännu inte implementerad
struct ComputerProgress
{
	int checkedMoves; //i nuläget: 0 = drag påbörjat, 1 = drag avslutat
	int totalMoves; //inte implementerad
};