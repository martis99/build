#include "c.h"
#include "cpp.hpp"

#include "lib.h"
#include "libasm.h"
#include "libcc.h"
#include "libccpp.hpp"

#include <iostream>

extern "C" int asm_args(int argc, char **argv);

int main(int argc, char **argv)
{
	int ret = 0;

	std::cout << "" << std::endl;

	if (asm_args(argc, argv)) {
		ret = 1;
	}

	if (lib_args(argc, argv)) {
		ret = 1;
	}

	if (dlib_args(argc, argv)) {
		ret = 1;
	}

	if (libasm_args(argc, argv)) {
		ret = 1;
	}

	if (dlibasm_args(argc, argv)) {
		ret = 1;
	}

	if (c_args(argc, argv)) {
		ret = 1;
	}

	if (libcc_args(argc, argv)) {
		ret = 1;
	}

	if (dlibcc_args(argc, argv)) {
		ret = 1;
	}

	if (cpp_args(argc, argv)) {
		ret = 1;
	}

	if (libccpp_args(argc, argv)) {
		ret = 1;
	}

	if (dlibccpp_args(argc, argv)) {
		ret = 1;
	}

	return ret;
}
