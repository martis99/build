#include "app.h"

#include <iostream>

int main(int argc, char *argv[])
{
	std::cout << "args: ";
	for (int i = 0; i < argc; i++) {
		std::cout << argv[i] << " ";
	}
	std::cout << std::endl;

	App app;

	app.print();

	return 0;
}
