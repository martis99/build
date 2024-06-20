#include "c.h"
#include "cpp.hpp"

#include <iostream>

extern "C" int asm_args(int argc, char **argv);

int main(int argc, char **argv)
{
	int ret = 0;

	std::cout << "" << std::endl;


	if (asm_args(argc, argv)) {
		ret = 1;
	}

	if (c_args(argc, argv)) {
		ret = 1;
	}

	if (cpp_args(argc, argv)) {
		ret = 1;
	}

	return ret;
}
