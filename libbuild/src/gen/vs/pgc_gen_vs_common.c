#include "pgc_gen_vs_common.h"

str_t arch_to_plat(str_t arch)
{
	if (str_eq(arch, STR("i386"))) {
		return STR("Win32");
	} else if (str_eq(arch, STR("x86_64"))) {
		return STR("x64");
	} else {
		return arch;
	}
}
