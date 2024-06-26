#include "libcc.h"

#include <stdio.h>

#if defined(LIBCC_BUILD_DLL)
int dlibcc_args(int argc, char **argv)
{
	if (argc != 2) {
		return 1;
	}

	printf("DLIBC: %s\n", argv[1]);

	return 0;
}
#else
int libcc_args(int argc, char **argv)
{
	if (argc != 2) {
		return 1;
	}

	printf("LIBC: %s\n", argv[1]);

	return 0;
}
#endif
