#include "app.h"

#include "dlib.hpp"
#include "lib1.h"
#include "slib.h"

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
