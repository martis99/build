#include "libccpp.hpp"

#include <iostream>

#if defined(LIBCCPP_BUILD_DLL)
int dlibccpp_args(int argc, char **argv)
{
	if (argc != 2) {
		return 1;
	}

	std::cout << "DLIBCPP: " << argv[1] << std::endl;
	return 0;
}
#else
int libccpp_args(int argc, char **argv)
{
	if (argc != 2) {
		return 1;
	}

	std::cout << "LIBCPP: " << argv[1] << std::endl;
	return 0;
}
#endif
