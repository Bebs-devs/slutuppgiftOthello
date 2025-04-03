#include <vector> //std::vector
#include <string> //std::string
#include "othelloResources.h" //Board, GameSettings, GameCoordinates, ComputerProgress

#pragma once //ser till så att filer inte inkluderas flera gånger av misstag

/*	den här filen innehåller namespacet och de yttre funktioner som hör till APIn för
	render-motorn. Följande funktioner är alltså till för att användas av huvudfilen för 
	utskrift av spelplanen med mera.
	notera att render-motorn inte har ensamrätt på skärm-output.
	tanken med render-motorn är att den lagrar s.k. displaystate hela tiden
	över vad som ska skrivas ut
	sedan finns olika funktioner för att ändra på displaystate när ändringar sker i programmet
	för att se någonting på skärmen anropas funktionen updateScreenAndAnimations,
	denna funktion bör anropas så ofta som möjligt (i rimligt mån) för att animationerna ska se
	flytande ut
	Notera att andra funktioner, såsom updateBoard kallar updateScreenAndAnimations internt,
	dock kan detta undvikas genom att sätta parametern updateScreen till false.
*/

namespace render {
	
	//sets - används för att hämta information om spelet
	void setBoard(Board* board); //ge en pekare till spelbrädet till render-motorn
	void setSettings(GameSettings* settings); //ge en pekare till inställningarna till render-motorn

	//updates - uppdaterar displaystate på olika sätt
	void updateSettings(bool updateScreen = true); //när ändringar i vilka spelare som är AI eller människor sker
	void updateBoard(bool updateScreen = true); //när ändringar sker i brädet
	void updatePossibleMoves(std::vector<GameCoordinates> moves = {}, bool updateScreen = true); //för att visa möjliga drag
	void updateSelectedSquare(GameCoordinates square); //för att markera en ruta
	void updateLastMove(GameCoordinates move, bool updateScreen = true); //för att uppdatera vilket drag som senast skedde
	void updateDebugText(std::string text, bool updateScreen = true); //för att visa en textrad på skärmen
	void updateComputerProgress(ComputerProgress progress); //när ändringar i hur långt datorn har kommit i att välja drag sker
	void updateScreenAndAnimations(); //när ingen ändring har skett men för att uppdatera animationer och skärmen

	//övriga
	void splashText(std::string text, int durationMs = 2000, bool returnWhenFinished = false); //visa en textruta mitt över brädet en viss tid
	void restoreScreen(); //återställ skärmen ifall utskriften ser märklig ut
	void init(); //kallas innan första update-funktionen
}

//för mer information, se cpp-filen
