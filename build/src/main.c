#include "sln.h"
#include "utils.h"

#include "args.h"
#include "types.h"

#include "mem.h"
#include "defines.h"

#include <stdio.h>
#include <stdlib.h>

int G_DBG = 0;
int G_SUC = 0;
int G_INF = 0;
int G_WRN = 1;
int G_ERR = 1;
int G_MSG = 1;

typedef enum gen_e {
	GEN_NONE,
	GEN_CMAKE,
	GEN_MAKE,
	GEN_VS,

	__GEN_MAX,
} gen_t;

static inline gen_t gen_enum(char c) {
	switch (c) {
	case 'N':
		return GEN_NONE;
	case 'C':
		return GEN_CMAKE;
	case 'M':
		return GEN_MAKE;
	case 'V':
		return GEN_VS;
	default:
		return __GEN_MAX;
	}
}

static int handle_gen(const char *param, void *ret) {
	if (param[1] != '\0' || (*(gen_t *)ret = gen_enum(param[0])) == __GEN_MAX) {
		return 1;
	}
	return 0;
}

typedef enum arg_enum_e {
	ARG_S,
	ARG_B,
	ARG_G,
	ARG_D,
	ARG_C,
	ARG_I,
	ARG_W,
	ARG_E,
	ARG_M
} arg_enum_t;

typedef int(*gen_fn)(sln_t *sln, const path_t *path);

int main(int argc, char *argv[]) {
	size_t mem = 0;
	mem_init(&mem);

	const char *name = "build";
	const char *description = "Build project";

	arg_t args[] = {
	[ARG_S] = {'S', "solution", PARAM_STR, "<dir>", "Specify a solution directory"},
	[ARG_B] = {'B', "build", PARAM_STR, "<dir>", "Specify a build directory"},
	[ARG_G] = {'G', "generator", PARAM_MODE, "<name>", "Specify a build system generator", handle_gen},
	[ARG_D] = {'D', "debug", PARAM_SWITCH, "<switch>", "Turn on/off debug messages"},
	[ARG_C] = {'C', "succes", PARAM_SWITCH, "<switch>", "Turn on/off success messages"},
	[ARG_I] = {'I', "info", PARAM_SWITCH, "<switch>", "Turn on/off info messages"},
	[ARG_W] = {'W', "warning", PARAM_SWITCH, "<switch>", "Turn on/off warning messages"},
	[ARG_E] = {'E', "error", PARAM_SWITCH, "<switch>", "Turn on/off error messages"},
	[ARG_M] = {'M', "message", PARAM_SWITCH, "<switch>", "Turn on/off messages"},
	};

	mode_t gen_modes[] = {
		[GEN_NONE] = {.c = 'N', .desc = "None"},
		[GEN_CMAKE] = {.c = 'C', .desc = "CMake"},
		[GEN_MAKE] = {.c = 'M', .desc = "Make"},
		[GEN_VS] = {.c = 'V', .desc = "Visual Studio 17 2022"},
	};

	mode_desc_t modes[] = {
		{.name = "Generators", .modes = gen_modes, .len = __GEN_MAX},
	};

	gen_fn gen_fns[] = {
		[GEN_CMAKE] = sln_gen_cmake,
		[GEN_MAKE] = sln_gen_make,
		[GEN_VS] = sln_gen_vs,
	};

	char *solution = NULL;
	char *build = NULL;
	gen_t gen = GEN_NONE;

	void *params[] = {
	[ARG_S] = &solution,
	[ARG_B] = &build,
	[ARG_G] = &gen,
	[ARG_D] = &G_DBG,
	[ARG_C] = &G_SUC,
	[ARG_I] = &G_INF,
	[ARG_W] = &G_WRN,
	[ARG_E] = &G_ERR,
	[ARG_M] = &G_MSG,
	};


	if (args_handle(name, description, args, sizeof(args), modes, sizeof(modes), argc, argv, params)) {
		return 1;
	}

	if (solution == NULL) {
		solution = "";
	}

	build = build ? build : solution;

	path_t sln_dir = { 0 };
	if (path_init(&sln_dir, solution, cstr_len(solution))) {
		return 1;
	}

	int ret = 0;
	sln_t sln = { 0 };

	ret += sln_read(&sln, &sln_dir);

	if (ret != 0) {
		return ret;
	}
	sln_print(&sln);

	path_t build_dir = { 0 };
	if (path_init(&build_dir, build, cstr_len(build))) {
		return 1;
	}

	ret += gen_fns[gen](&sln, &build_dir);

	INF("mem: %zd", mem);

	sln_free(&sln);

	INF("mem: %zd", mem);
	return ret;
}