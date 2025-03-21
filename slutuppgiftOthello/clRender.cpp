#include "clRender.h"
#include <iostream>
#include <vector>
#include <string>
using namespace render;


static std::string displayState[12]; //index 1-8 för brädet


static void print() {
	std::cout << "\033[H";
	for (int i = 0; i < 12; ++i) {
		std::cout << displayState[i] << '\n';
	}
}

void render::update() {
	
	
}

void render::init() {
	std::cout << "\033[?25l"; //göm skriv-markören
	displayState[0] = " O T H E L L O";
	for (int i = 1; i <= 8; ++i) {
		displayState[i] = "\033[48;5;28m                \033[0m";
	}
	displayState[2] += " 2 - Svart";
	displayState[3] += " 2 - Vit";

	displayState[5] += " *Svarts Tur*";
	displayState[9] = "Funkart";
	displayState[10] = "[---------     ]";
	displayState[11] = "123/13948";
	print();
}