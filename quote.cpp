#include "quote.h"
#include <iostream>

int main(int argc, char* argv[]) {
	std::string symbol = argc > 1 ? argv[1] : "";
	std::string exchange = argc > 2 ? argv[2] : "";
	std::cout << "Fetching quote for " << symbol << " on exchange " << exchange << std::endl;
	return 0;
}