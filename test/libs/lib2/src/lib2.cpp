#include "lib2.hpp"

#include "lib3.h"

#include <iostream>

void Printer::print()
{
	std::cout << "lib2" << std::endl;
	lib3_print();
}
