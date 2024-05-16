#include "dlib.hpp"

#include "lib3.h"

#include <iostream>

int Printer::print()
{
	std::cout << "dlib" << std::endl;
	if (lib3_print()) {
		return 1;
	}

	return 0;
}
