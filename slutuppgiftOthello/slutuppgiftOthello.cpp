#include <iostream>
#include <locale>

int main() {
	setlocale(LC_ALL, "sv_SE");
	std::cout << "hej ugituiåäöehf";
	return 0;
}