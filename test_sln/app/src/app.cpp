#include "app.h"

#include "lib1.h"
#include "lib2.hpp"
#include "slib.h"

#pragma comment(lib, "slib.lib")

int App::print()
{
	if (lib1_print()) {
		return 1;
	}

	Printer printer;
	if (printer.print()) {
		return 1;
	}

	if (slib_print()) {
		return 1;
	}

	return 0;
}
