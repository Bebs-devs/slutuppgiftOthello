#include <vector> //std::vector
#include <string> //std::string
#include "othelloResources.h" //Board, GameSettings, GameCoordinates, ComputerProgress

#pragma once //ser till s� att filer inte inkluderas flera g�nger av misstag

/*	den h�r filen inneh�ller namespacet och de yttre funktioner som h�r till APIn f�r
	render-motorn. F�ljande funktioner �r allts� till f�r att anv�ndas av huvudfilen f�r 
	utskrift av spelplanen med mera.
	notera att render-motorn inte har ensamr�tt p� sk�rm-output.
	tanken med render-motorn �r att den lagrar s.k. displaystate hela tiden
	�ver vad som ska skrivas ut
	sedan finns olika funktioner f�r att �ndra p� displaystate n�r �ndringar sker i programmet
	f�r att se n�gonting p� sk�rmen anropas funktionen updateScreenAndAnimations,
	denna funktion b�r anropas s� ofta som m�jligt (i rimligt m�n) f�r att animationerna ska se
	flytande ut
	Notera att andra funktioner, s�som updateBoard kallar updateScreenAndAnimations internt,
	dock kan detta undvikas genom att s�tta parametern updateScreen till false.
*/

namespace render {
	
	//sets - anv�nds f�r att h�mta information om spelet
	void setBoard(Board* board); //ge en pekare till spelbr�det till render-motorn
	void setSettings(GameSettings* settings); //ge en pekare till inst�llningarna till render-motorn

	//updates - uppdaterar displaystate p� olika s�tt
	void updateSettings(bool updateScreen = true); //n�r �ndringar i vilka spelare som �r AI eller m�nniskor sker
	void updateBoard(bool updateScreen = true); //n�r �ndringar sker i br�det
	void updatePossibleMoves(std::vector<GameCoordinates> moves = {}, bool updateScreen = true); //f�r att visa m�jliga drag
	void updateSelectedSquare(GameCoordinates square); //f�r att markera en ruta
	void updateLastMove(GameCoordinates move, bool updateScreen = true); //f�r att uppdatera vilket drag som senast skedde
	void updateDebugText(std::string text, bool updateScreen = true); //f�r att visa en textrad p� sk�rmen
	void updateComputerProgress(ComputerProgress progress); //n�r �ndringar i hur l�ngt datorn har kommit i att v�lja drag sker
	void updateScreenAndAnimations(); //n�r ingen �ndring har skett men f�r att uppdatera animationer och sk�rmen

	//�vriga
	void splashText(std::string text, int durationMs = 2000, bool returnWhenFinished = false); //visa en textruta mitt �ver br�det en viss tid
	void restoreScreen(); //�terst�ll sk�rmen ifall utskriften ser m�rklig ut
	void init(); //kallas innan f�rsta update-funktionen
}

//f�r mer information, se cpp-filen
