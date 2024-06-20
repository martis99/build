#include "cpp.hpp"

#include <iostream>

int cpp_args(int argc, char **argv)
{
	if (argc != 2) {
		return 1;
	}

	std::cout << "CPP: " << argv[1] << std::endl;
	return 0;
}
