#include "app.h"

#include "lib1.h"
#include "lib2.hpp"
#include "slib.h"

#pragma comment(lib, "slib.lib")

void App::print()
{
	lib1_print();

	Printer printer;
	printer.print();

	slib_print();
}
