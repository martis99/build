#include "lib.h"

#include "liblib.h"

#include <stdio.h>

#if defined(LIB_BUILD_DLL)
int dlib_args(int argc, char **argv)
{
	if (argc != 2) {
		return 1;
	}

	int ret = 0;

	if (liblib_args(argc, argv)) {
		ret = 1;
	}

	if (dliblib_args(argc, argv)) {
		ret = 1;
	}

	printf("DLIB: %s\n", argv[1]);

	return ret;
}
#else
int lib_args(int argc, char **argv)
{
	if (argc != 2) {
		return 1;
	}

	int ret = 0;

	if (extlib_args(argc, argv)) {
		ret = 1;
	}

	if (dextlib_args(argc, argv)) {
		ret = 1;
	}

	if (liblib_args(argc, argv)) {
		ret = 1;
	}

	if (dliblib_args(argc, argv)) {
		ret = 1;
	}

	printf("LIB: %s\n", argv[1]);

	return ret;
}
#endif
