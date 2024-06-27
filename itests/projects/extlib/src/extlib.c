#include "extlib.h"

#include <stdio.h>

#if defined(EXTLIB_BUILD_DLL)
int dextlib_args(int argc, char **argv)
{
	if (argc != 2) {
		return 1;
	}

	printf("DEXTLIB: %s\n", argv[1]);

	return 0;
}
#else
int extlib_args(int argc, char **argv)
{
	if (argc != 2) {
		return 1;
	}

	printf("EXTLIB: %s\n", argv[1]);

	return 0;
}
#endif
