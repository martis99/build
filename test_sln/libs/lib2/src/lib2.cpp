#include "lib2.hpp"

#include "lib3.h"

#include <iostream>

int Printer::print()
{
	std::cout << "lib2" << std::endl;
	if (lib3_print()) {
		return 1;
	}

	return 0;
}
