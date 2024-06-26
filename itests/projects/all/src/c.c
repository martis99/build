#include "c.h"

#include <stdio.h>

int c_args(int argc, char **argv)
{
	if (argc != 2) {
		return 1;
	}

	printf("C: %s\n", argv[1]);
	return 0;
}
