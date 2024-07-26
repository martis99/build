#include "liblib.h"

#include <stdio.h>

#if defined(LIBLIB_BUILD_DLL)
int dliblib_args(int argc, char **argv)
{
	if (argc != 2) {
		return 1;
	}

	printf("DLIBLIB: %s\n", argv[1]);

	return 0;
}
#else
int liblib_args(int argc, char **argv)
{
	if (argc != 2) {
		return 1;
	}

	printf("LIBLIB: %s\n", argv[1]);

	return 0;
}
#endif
