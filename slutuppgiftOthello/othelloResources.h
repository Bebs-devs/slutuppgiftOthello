#pragma once
typedef signed char int8;

struct GameSettings
{
	bool player1iscomp, player2iscomp;
	int comp1Difficulty, comp2Difficulty;

	GameSettings();
};

struct GameCoordinates
{
	static const int8 translations[8][2];
	int y, x;

	bool isInValid();
};


struct Board
{
	bool isBlacksTurn;
	int numberOfDiscs[2]; //{antal svarta brickor, antal vita brickor}
	char discs[8][8]; //a tom, b svart, c vit
	//[y][x] gäller för alla arrayer som håller positioner
	bool adjacents[8][8]; //är ruta bredvid en ruta med bricka i.

	Board();
};

struct ComputerProgress
{
	int checkedMoves;
	int totalMoves;
};