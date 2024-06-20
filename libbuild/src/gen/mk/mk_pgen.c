#include "gen/mk/mk_pgen.h"

#include "make.h"

typedef struct mk_pgen_header_data_s {
	str_t dir;
	mk_pgen_header_ext_t exts;
} mk_pgen_header_data_t;

typedef struct mk_pgen_src_data_s {
	str_t dir;
	mk_pgen_src_ext_t exts;
} mk_pgen_src_data_t;

typedef struct mk_pgen_file_data_s {
	str_t path;
	mk_pgen_file_ext_t ext;
} mk_pgen_file_data_t;

mk_pgen_t *mk_pgen_init(mk_pgen_t *gen)
{
	if (gen == NULL) {
		return NULL;
	}

	if (arr_init(&gen->configs, 2, sizeof(str_t)) == NULL) {
		return NULL;
	}

	if (arr_init(&gen->headers, 1, sizeof(mk_pgen_header_data_t)) == NULL) {
		return NULL;
	}

	if (arr_init(&gen->srcs, 1, sizeof(mk_pgen_src_data_t)) == NULL) {
		return NULL;
	}

	gen->includes = strz(64);

	for (mk_pgen_src_ext_t s = 0; s < __MK_SRC_MAX; s++) {
		gen->flags[s] = strz(16);
	}

	for (mk_pgen_intdir_type_t i = 0; i < __MK_INTDIR_MAX; i++) {
		gen->defines[i] = strz(16);
	}

	gen->ldflags = strz(16);

	if (arr_init(&gen->files, 2, sizeof(mk_pgen_file_data_t)) == NULL) {
		return NULL;
	}

	if (arr_init(&gen->requires, 1, sizeof(str_t)) == NULL) {
		return NULL;
	}

	return gen;
}

void mk_pgen_free(mk_pgen_t *gen)
{
	if (gen == NULL) {
		return;
	}

	str_free(&gen->outdir);

	for (mk_pgen_intdir_type_t i = 0; i < __MK_INTDIR_MAX; i++) {
		str_free(&gen->intdir[i]);
	}

	str_free(&gen->covdir);

	str_t *config;
	arr_foreach(&gen->configs, config)
	{
		str_free(config);
	}
	arr_free(&gen->configs);

	mk_pgen_header_data_t *header;
	arr_foreach(&gen->headers, header)
	{
		str_free(&header->dir);
	}
	arr_free(&gen->headers);

	mk_pgen_src_data_t *src;
	arr_foreach(&gen->srcs, src)
	{
		str_free(&src->dir);
	}
	arr_free(&gen->srcs);

	str_free(&gen->args);
	str_free(&gen->includes);

	for (mk_pgen_src_ext_t s = 0; s < __MK_SRC_MAX; s++) {
		str_free(&gen->flags[s]);
	}

	for (mk_pgen_intdir_type_t i = 0; i < __MK_INTDIR_MAX; i++) {
		str_free(&gen->defines[i]);
	}

	str_free(&gen->ldflags);

	for (mk_pgen_build_type_t b = 0; b < __MK_BUILD_MAX; b++) {
		str_free(&gen->run[b]);
	}

	for (mk_pgen_build_type_t b = 0; b < __MK_BUILD_MAX; b++) {
		str_free(&gen->run_debug[b]);
	}

	for (mk_pgen_build_type_t b = 0; b < __MK_BUILD_MAX; b++) {
		str_free(&gen->artifact[b]);
	}

	mk_pgen_file_data_t *file;
	arr_foreach(&gen->files, file)
	{
		str_free(&file->path);
	}
	arr_free(&gen->files);

	str_free(&gen->size);

	str_free(&gen->url);
	str_free(&gen->name);
	str_free(&gen->format);

	str_t *require;
	arr_foreach(&gen->requires, require)
	{
		str_free(require);
	}
	arr_free(&gen->requires);

	str_free(&gen->config);
	str_free(&gen->targets);
}

//TODO: Add ability to set config name independent from config settings
uint mk_pgen_add_config(mk_pgen_t *gen, str_t config)
{
	if (gen == NULL) {
		return MK_HEADER_END;
	}

	uint id = arr_add(&gen->configs);

	str_t *str = arr_get(&gen->configs, id);
	if (str == NULL) {
		return MK_CONFIG_END;
	}

	*str = config;

	return id;
}

uint mk_pgen_add_header(mk_pgen_t *gen, str_t dir, int exts)
{
	if (gen == NULL) {
		return MK_HEADER_END;
	}

	uint id = arr_add(&gen->headers);

	mk_pgen_header_data_t *header = arr_get(&gen->headers, id);
	if (header == NULL) {
		return MK_HEADER_END;
	}

	*header = (mk_pgen_header_data_t){
		.dir  = dir,
		.exts = exts,
	};

	return id;
}

uint mk_pgen_add_src(mk_pgen_t *gen, str_t dir, int exts)
{
	if (gen == NULL) {
		return MK_SRC_END;
	}

	uint id = arr_add(&gen->srcs);

	mk_pgen_src_data_t *src = arr_get(&gen->srcs, id);
	if (src == NULL) {
		return MK_SRC_END;
	}

	*src = (mk_pgen_src_data_t){
		.dir  = dir,
		.exts = exts,
	};

	return id;
}

void mk_pgen_add_include(mk_pgen_t *gen, str_t dir)
{
	if (gen == NULL) {
		return;
	}

	str_cat(&gen->includes, gen->includes.len > 0 ? STR(" -I") : STR("-I"));
	str_cat(&gen->includes, dir);
}

void mk_pgen_add_flag(mk_pgen_t *gen, str_t flag, int exts)
{
	if (gen == NULL) {
		return;
	}

	for (mk_pgen_src_ext_t s = 0; s < __MK_SRC_MAX; s++) {
		if ((exts & (1 << s)) == 0) {
			continue;
		}
		if (gen->flags[s].len > 0) {
			str_cat(&gen->flags[s], STR(" "));
		}
		str_cat(&gen->flags[s], flag);
	}
}

void mk_pgen_add_define(mk_pgen_t *gen, str_t define, int intdirs)
{
	if (gen == NULL) {
		return;
	}

	for (mk_pgen_intdir_type_t i = 0; i < __MK_INTDIR_MAX; i++) {
		if ((intdirs & (1 << i)) == 0) {
			continue;
		}

		str_cat(&gen->defines[i], gen->defines[i].len > 0 ? STR(" -D") : STR("-D"));
		str_cat(&gen->defines[i], define);
	}
}

void mk_pgen_add_ldflag(mk_pgen_t *gen, str_t ldflag)
{
	if (gen == NULL) {
		return;
	}

	if (gen->ldflags.len > 0) {
		str_cat(&gen->ldflags, STR(" "));
	}
	str_cat(&gen->ldflags, ldflag);
}

void mk_pgen_add_slib(mk_pgen_t *gen, str_t lib)
{
	if (gen == NULL) {
		return;
	}

	str_cat(&gen->ldflags, gen->ldflags.len > 0 ? STR(" -l:") : STR("-l:"));
	str_cat(&gen->ldflags, lib);
	str_cat(&gen->ldflags, STR(".a"));
}

void mk_pgen_add_dlib(mk_pgen_t *gen, str_t lib)
{
	if (gen == NULL) {
		return;
	}

	str_cat(&gen->ldflags, gen->ldflags.len > 0 ? STR(" -l:") : STR("-l:"));
	str_cat(&gen->ldflags, lib);
	str_cat(&gen->ldflags, STR(".so"));
}

void mk_pgen_add_slib_dir(mk_pgen_t *gen, str_t lib)
{
	if (gen == NULL) {
		return;
	}

	str_cat(&gen->ldflags, gen->ldflags.len > 0 ? STR(" -L") : STR("-L"));
	str_cat(&gen->ldflags, lib);
}

void mk_pgen_add_dlib_dir(mk_pgen_t *gen, str_t lib)
{
	if (gen == NULL) {
		return;
	}

	str_cat(&gen->ldflags, gen->ldflags.len > 0 ? STR(" -Wl,-rpath,") : STR("-Wl,-rpath,"));
	str_cat(&gen->ldflags, lib);
}

void mk_pgen_set_run(mk_pgen_t *gen, str_t run, int build)
{
	if (gen == NULL) {
		return;
	}

	for (mk_pgen_build_type_t b = 0; b < __MK_BUILD_MAX; b++) {
		if ((build & (1 << b)) == 0) {
			continue;
		}

		gen->run[b] = run;
	}
}

void mk_pgen_set_run_debug(mk_pgen_t *gen, str_t run, int build)
{
	if (gen == NULL) {
		return;
	}

	for (mk_pgen_build_type_t b = 0; b < __MK_BUILD_MAX; b++) {
		if ((build & (1 << b)) == 0) {
			continue;
		}

		gen->run_debug[b] = run;
	}
}

uint mk_pgen_add_file(mk_pgen_t *gen, str_t path, int ext)
{
	if (gen == NULL) {
		return MK_FILE_END;
	}

	uint id = arr_add(&gen->files);

	mk_pgen_file_data_t *file = arr_get(&gen->files, id);
	if (file == NULL) {
		return MK_FILE_END;
	}

	*file = (mk_pgen_file_data_t){
		.path = path,
		.ext  = ext,
	};

	return id;
}

uint mk_pgen_add_require(mk_pgen_t *gen, str_t require)
{
	if (gen == NULL) {
		return MK_REQUIRE_END;
	}

	uint id = arr_add(&gen->requires);

	str_t *data = arr_get(&gen->requires, id);
	if (data == NULL) {
		return MK_REQUIRE_END;
	}

	*data = require;

	return id;
}

static int is_config(const mk_pgen_t *gen, str_t config)
{
	const str_t *conf;
	arr_foreach(&gen->configs, conf)
	{
		if (str_eq(*conf, config)) {
			return 1;
		}
	}

	return 0;
}

make_t *mk_pgen_local(const mk_pgen_t *gen, make_t *make)
{
	const make_var_t arch	  = make_create_var_ext(make, STR("ARCH"), MAKE_VAR_INST);
	const make_var_t config	  = make_create_var_ext(make, STR("CONFIG"), MAKE_VAR_INST);
	const make_var_t coverage = make_create_var_ext(make, STR("COVERAGE"), MAKE_VAR_INST);
	const make_var_t show	  = make_create_var_ext(make, STR("SHOW"), MAKE_VAR_INST);

	int is_vars = 0;

	make_var_t outdir = MAKE_END;
	if (gen->outdir.data) {
		is_vars = 1;

		outdir = make_add_act(make, make_create_var(make, STR("OUTDIR"), MAKE_VAR_INST));
		make_var_add_val(make, outdir, MSTR(str_cpy(gen->outdir)));
	}

	make_var_t intdir[] = {
		[MK_INTDIR_OBJECT] = MAKE_END,
		[MK_INTDIR_STATIC] = MAKE_END,
		[MK_INTDIR_SHARED] = MAKE_END,
	};

	static struct {
		str_t name;
		str_t defines;
		const char *flags;
	} intdir_c[] = {
		[MK_INTDIR_OBJECT] = { STRS("INTDIR"), STRS("DEFINES"), "" },
		[MK_INTDIR_STATIC] = { STRS("INTDIR_S"), STRS("DEFINES_S"), "" },
		[MK_INTDIR_SHARED] = { STRS("INTDIR_D"), STRS("DEFINES_D"), " -fPIC" },
	};

	for (mk_pgen_intdir_type_t i = 0; i < __MK_INTDIR_MAX; i++) {
		if (gen->intdir[i].data == NULL) {
			continue;
		}

		is_vars = 1;

		intdir[i] = make_add_act(make, make_create_var(make, intdir_c[i].name, MAKE_VAR_INST));
		make_var_add_val(make, intdir[i], MSTR(str_cpy(gen->intdir[i])));
	}

	make_var_t covdir = MAKE_END;
	if (gen->covdir.data) {
		is_vars = 1;

		covdir = make_add_act(make, make_create_var(make, STR("COVDIR"), MAKE_VAR_INST));
		make_var_add_val(make, covdir, MSTR(str_cpy(gen->covdir)));
	}

	static const char *header_ext[] = {
		[MK_EXT_INC] = "inc",
		[MK_EXT_H]   = "h",
		[MK_EXT_HPP] = "hpp",
	};

	make_var_t headers = MAKE_END;
	const mk_pgen_header_data_t *header;
	arr_foreach(&gen->headers, header)
	{
		for (mk_pgen_header_ext_t ext = 0; ext < __MK_HEADER_MAX; ext++) {
			if ((header->exts & (1 << ext)) == 0) {
				continue;
			}

			is_vars = 1;

			headers = make_add_act(make, make_create_var(make, STR("HEADERS"), headers == MAKE_END ? MAKE_VAR_INST : MAKE_VAR_APP));
			make_var_add_val(make, headers, MSTR(strf("$(shell find %.*s -name '*.%s')", header->dir.len, header->dir.data, header_ext[ext])));
		}
	}

	// clang-format off
	make_var_t target[] = {
		[MK_BUILD_EXE]	  = MAKE_END,
		[MK_BUILD_STATIC] = MAKE_END,
		[MK_BUILD_SHARED] = MAKE_END,
		[MK_BUILD_BIN]	  = MAKE_END,
		[MK_BUILD_ELF]	  = MAKE_END,
		[MK_BUILD_FAT12]  = MAKE_END,
	};

	static struct {
		str_t name;
		const char *ext;
		str_t act;
		mk_pgen_intdir_type_t intdir;
		str_t run;
		str_t artifact;
	} target_c[] = {
		[MK_BUILD_EXE]	  = { STRS("TARGET"),	    "",     STRS("compile"), MK_INTDIR_OBJECT, STRS("run"),	  STRS("artifact")	 },
		[MK_BUILD_STATIC] = { STRS("TARGET_S"),     ".a",   STRS("static"),  MK_INTDIR_STATIC, STRS("run_s"),	  STRS("artifact_s")	 },
		[MK_BUILD_SHARED] = { STRS("TARGET_D"),     ".so",  STRS("shared"),  MK_INTDIR_SHARED, STRS("run_d"),	  STRS("artifact_d")	 },
		[MK_BUILD_BIN]	  = { STRS("TARGET_BIN"),   ".bin", STRS("bin"),     MK_INTDIR_OBJECT,  STRS("run_bin"),   STRS("artifact_bin")	 }, 
		[MK_BUILD_ELF]	  = { STRS("TARGET_ELF"),   ".elf", STRS("elf"),     MK_INTDIR_OBJECT, STRS("run_elf"),   STRS("artifact_elf")	 }, 
		[MK_BUILD_FAT12]  = { STRS("TARGET_FAT12"), ".img", STRS("fat12"),   __MK_INTDIR_MAX,  STRS("run_fat12"), STRS("artifact_fat12") },
	};
	// clang-format on

	for (mk_pgen_build_type_t b = 0; b < __MK_BUILD_MAX; b++) {
		if (outdir == MAKE_END || gen->name.data == NULL || (gen->builds & (1 << b)) == 0) {
			continue;
		}

		is_vars = 1;

		target[b] = make_add_act(make, make_create_var(make, target_c[b].name, MAKE_VAR_INST));
		make_var_add_val(make, target[b], MSTR(strf("$(OUTDIR)%.*s%s", gen->name.len, gen->name.data, target_c[b].ext)));
	}

	make_var_t args = MAKE_END;
	if (target[MK_BUILD_EXE] != MAKE_END) {
		args = make_add_act(make, make_create_var(make, STR("ARGS"), MAKE_VAR_INST));
		if (gen->args.len > 0) {
			make_var_add_val(make, args, MSTR(str_cpy(gen->args)));
		}
	}

	// clang-format off
	static struct {
		str_t name;
		str_t flags;
		const char *ext;
		int cov;
	} src_c[] = {
		[MK_EXT_ASM] = { STRS("SRC_NASM"), STRS("ASFLAGS"),  "asm", 0 },
		[MK_EXT_S]   = { STRS("SRC_ASM"),  STRS("ASFLAGS"),  "S",   0 },
		[MK_EXT_C]   = { STRS("SRC_C"),    STRS("CFLAGS"),   "c",   1 },
		[MK_EXT_CPP] = { STRS("SRC_CPP"),  STRS("CXXFLAGS"), "cpp", 1 },
	};
	// clang-format on

	make_var_t src[] = {
		[MK_EXT_ASM] = MAKE_END,
		[MK_EXT_S]   = MAKE_END,
		[MK_EXT_C]   = MAKE_END,
		[MK_EXT_CPP] = MAKE_END,
	};

	for (mk_pgen_src_ext_t s = 0; s < __MK_SRC_MAX; s++) {
		const mk_pgen_src_data_t *data;
		arr_foreach(&gen->srcs, data)
		{
			if ((data->exts & (1 << s)) == 0) {
				continue;
			}

			is_vars = 1;

			src[s] = make_add_act(make, make_create_var(make, src_c[s].name, src[s] == MAKE_END ? MAKE_VAR_INST : MAKE_VAR_APP));
			make_var_add_val(make, src[s], MSTR(strf("$(shell find %.*s -name '*.%s')", data->dir.len, data->dir.data, src_c[s].ext)));
		}
	}

	make_var_t obj[][__MK_SRC_MAX] = {
		[MK_INTDIR_OBJECT] = {
			[MK_EXT_ASM] = MAKE_END,
			[MK_EXT_S]   = MAKE_END,
			[MK_EXT_C]   = MAKE_END,
			[MK_EXT_CPP] = MAKE_END,
		},
		[MK_INTDIR_STATIC] = {
			[MK_EXT_ASM] = MAKE_END,
			[MK_EXT_S]   = MAKE_END,
			[MK_EXT_C]   = MAKE_END,
			[MK_EXT_CPP] = MAKE_END,
		},
		[MK_INTDIR_SHARED] = {
			[MK_EXT_ASM] = MAKE_END,
			[MK_EXT_S]   = MAKE_END,
			[MK_EXT_C]   = MAKE_END,
			[MK_EXT_CPP] = MAKE_END,
		},
	};

	make_var_t obj_ext[] = {
		[MK_EXT_ASM] = MAKE_END,
		[MK_EXT_S]   = MAKE_END,
		[MK_EXT_C]   = MAKE_END,
		[MK_EXT_CPP] = MAKE_END,
	};

	static struct {
		str_t name;
	} obj_ext_c[][__MK_SRC_MAX] = {
		[MK_INTDIR_OBJECT] = {
			[MK_EXT_ASM] = { STRS("OBJ_ASM") },
			[MK_EXT_S]   = { STRS("OBJ_S")   },
			[MK_EXT_C]   = { STRS("OBJ_C")   },
			[MK_EXT_CPP] = { STRS("OBJ_CPP") },
		},
		[MK_INTDIR_STATIC] = {
			[MK_EXT_ASM] = { STRS("OBJ_ASM_S") },
			[MK_EXT_S]   = { STRS("OBJ_S_S")   },
			[MK_EXT_C]   = { STRS("OBJ_C_S")   },
			[MK_EXT_CPP] = { STRS("OBJ_CPP_S") },
		},
		[MK_INTDIR_SHARED] = {
			[MK_EXT_ASM] = { STRS("OBJ_ASM_D") },
			[MK_EXT_S]   = { STRS("OBJ_S_D")   },
			[MK_EXT_C]   = { STRS("OBJ_C_D")   },
			[MK_EXT_CPP] = { STRS("OBJ_CPP_D") },
		}, 
	};

	int is_obj = 0;

	for (mk_pgen_intdir_type_t i = 0; i < __MK_INTDIR_MAX; i++) {
		if (intdir[i] == MAKE_END) {
			continue;
		}

		for (mk_pgen_src_ext_t s = 0; s < __MK_SRC_MAX; s++) {
			if (!obj_ext_c[i][s].name.data || src[s] == MAKE_END) {
				continue;
			}

			is_vars = 1;
			is_obj	= 1;

			obj_ext[s] = obj[i][s] = make_add_act(make, make_create_var(make, obj_ext_c[i][s].name, MAKE_VAR_INST));
			make_var_add_val(make, obj[i][s], MSTR(strf("$(patsubst %%.%s, $(%s)%%.o, $(%s))", src_c[s].ext, intdir_c[i].name.data, src_c[s].name.data)));
		}
	}

	make_var_t cov = MAKE_END;
	for (mk_pgen_src_ext_t s = 0; s < __MK_SRC_MAX; s++) {
		if (covdir == MAKE_END || src[s] == MAKE_END || src_c[s].cov == 0) {
			continue;
		}

		is_vars = 1;

		cov = make_add_act(make, make_create_var(make, STR("COV"), cov == MAKE_END ? MAKE_VAR_INST : MAKE_VAR_APP));
		make_var_add_val(make, cov, MSTR(strf("$(patsubst %%.%s, $(COVDIR)%%.gcno, $(%s))", src_c[s].ext, src_c[s].name.data)));
		cov = make_add_act(make, make_create_var(make, STR("COV"), MAKE_VAR_APP));
		make_var_add_val(make, cov, MSTR(strf("$(patsubst %%.%s, $(COVDIR)%%.gcda, $(%s))", src_c[s].ext, src_c[s].name.data)));
	}

	make_var_t repdir = MAKE_END;
	make_var_t lcov	  = MAKE_END;
	if (target[MK_BUILD_EXE] != MAKE_END && covdir != MAKE_END) {
		is_vars = 1;

		repdir = make_add_act(make, make_create_var(make, STR("REPDIR"), MAKE_VAR_INST));
		make_var_add_val(make, repdir, MSTR(STR("$(COVDIR)coverage-report/")));
		lcov = make_add_act(make, make_create_var(make, STR("LCOV"), MAKE_VAR_INST));
		make_var_add_val(make, lcov, MSTR(STR("$(COVDIR)lcov.info")));

		cov = make_add_act(make, make_create_var(make, STR("COV"), cov == MAKE_END ? MAKE_VAR_INST : MAKE_VAR_APP));
		make_var_add_val(make, cov, MVAR(lcov));
		make_var_add_val(make, cov, MVAR(repdir));
	}

	if (is_vars) {
		make_add_act(make, make_create_empty(make));
	}

	int is_flags = 0;

	make_var_t includes = MAKE_END;
	if (is_obj && gen->includes.len > 0) {
		is_flags = 1;

		includes = make_add_act(make, make_create_var(make, STR("INCLUDES"), MAKE_VAR_INST));
		make_var_add_val(make, includes, MSTR(str_cpy(gen->includes)));
	}

	make_var_t flags[] = {
		[MK_EXT_ASM] = MAKE_END,
		[MK_EXT_S]   = MAKE_END,
		[MK_EXT_C]   = MAKE_END,
		[MK_EXT_CPP] = MAKE_END,
	};

	for (mk_pgen_src_ext_t s = 0; s < __MK_SRC_MAX; s++) {
		if (obj_ext[s] == MAKE_END || gen->flags[s].len == 0) {
			continue;
		}

		is_flags = 1;

		flags[s] = make_add_act(make, make_create_var(make, src_c[s].flags, MAKE_VAR_INST));
		make_var_add_val(make, flags[s], MSTR(str_cpy(gen->flags[s])));
	}

	make_var_t defines[] = {
		[MK_INTDIR_OBJECT] = MAKE_END,
		[MK_INTDIR_STATIC] = MAKE_END,
		[MK_INTDIR_SHARED] = MAKE_END,
	};

	for (mk_pgen_intdir_type_t i = 0; i < __MK_INTDIR_MAX; i++) {
		if (target[i] == MAKE_END || gen->defines[i].len == 0) {
			continue;
		}

		is_flags = 1;

		defines[i] = make_add_act(make, make_create_var(make, intdir_c[i].defines, MAKE_VAR_INST));
		make_var_add_val(make, defines[i], MSTR(str_cpy(gen->defines[i])));
	}

	make_var_t ldflags = MAKE_END;
	if (is_obj) {
		is_flags = 1;

		ldflags = make_add_act(make, make_create_var(make, STR("LDFLAGS"), MAKE_VAR_INST));
		if (gen->ldflags.len > 0) {
			make_var_add_val(make, ldflags, MSTR(str_cpy(gen->ldflags)));
		}
	}

	make_var_t nasm_config_flags = MAKE_END;
	if (obj_ext[MK_EXT_ASM] != MAKE_END) {
		is_flags = 1;

		nasm_config_flags = make_add_act(make, make_create_var(make, STR("ASM_CONFIG_FLAGS"), MAKE_VAR_INST));
	}

	make_var_t gcc_config_flags = MAKE_END;
	if (obj_ext[MK_EXT_S] != MAKE_END || obj_ext[MK_EXT_C] != MAKE_END || obj_ext[MK_EXT_CPP] != MAKE_END) {
		is_flags = 1;

		gcc_config_flags = make_add_act(make, make_create_var(make, STR("GCC_CONFIG_FLAGS"), MAKE_VAR_INST));
	}

	if (is_flags) {
		make_add_act(make, make_create_empty(make));
	}

	const make_var_t rm = make_add_act(make, make_create_var(make, STR("RM"), MAKE_VAR_APP));
	make_var_add_val(make, rm, MSTR(STR("-r")));

	make_add_act(make, make_create_empty(make));

	if (is_obj) {
		const make_if_t if_x86_64   = make_add_act(make, make_create_if(make, MVAR(arch), MSTR(STR("x86_64"))));
		const make_var_t bit_x86_64 = make_if_add_true_act(make, if_x86_64, make_create_var(make, STR("BITS"), MAKE_VAR_INST));
		make_var_add_val(make, bit_x86_64, MSTR(STR("64")));

		const make_if_t if_i386	   = make_add_act(make, make_create_if(make, MVAR(arch), MSTR(STR("i386"))));
		const make_var_t bits_i386 = make_if_add_true_act(make, if_i386, make_create_var(make, STR("BITS"), MAKE_VAR_INST));
		make_var_add_val(make, bits_i386, MSTR(STR("32")));

		make_add_act(make, make_create_empty(make));
	}

	int is_debug = is_config(gen, STR("Debug"));

	if (is_debug) {
		const make_if_t if_config = make_add_act(make, make_create_if(make, MVAR(config), MSTR(STR("Debug"))));
		if (nasm_config_flags != MAKE_END) {
			const make_var_t config_flags_true = make_if_add_true_act(make, if_config, make_create_var(make, STR("NASM_CONFIG_FLAGS"), MAKE_VAR_APP));
			make_var_add_val(make, config_flags_true, MSTR(STR("-g -F dwarf")));
		}
		if (gcc_config_flags != MAKE_END) {
			const make_var_t config_flags_true = make_if_add_true_act(make, if_config, make_create_var(make, STR("GCC_CONFIG_FLAGS"), MAKE_VAR_APP));
			make_var_add_val(make, config_flags_true, MSTR(STR("-ggdb3 -O0")));
		}
		make_add_act(make, make_create_empty(make));
	}

	if (cov != MAKE_END) {
		const make_if_t if_cov = make_add_act(make, make_create_if(make, MVAR(coverage), MSTR(STR("true"))));
		if (nasm_config_flags != MAKE_END) {
			const make_var_t cov_flags_true = make_if_add_true_act(make, if_cov, make_create_var(make, STR("NASM_CONFIG_FLAGS"), MAKE_VAR_APP));
			make_var_add_val(make, cov_flags_true, MSTR(STR("--coverage -fprofile-abs-path")));
		}
		if (gcc_config_flags != MAKE_END) {
			const make_var_t cov_flags_true = make_if_add_true_act(make, if_cov, make_create_var(make, STR("GGC_CONFIG_FLAGS"), MAKE_VAR_APP));
			make_var_add_val(make, cov_flags_true, MSTR(STR("--coverage -fprofile-abs-path")));
		}
		make_add_act(make, make_create_empty(make));
	}

	const make_rule_t all = make_add_act(make, make_create_rule(make, MRULE(MSTR(STR("all"))), 0));
	for (mk_pgen_build_type_t b = 0; b < __MK_BUILD_MAX; b++) {
		if (target[b] == MAKE_END) {
			continue;
		}

		make_rule_add_depend(make, all, MRULE(MSTR(target_c[b].act)));
	}

	const make_rule_t check = make_add_act(make, make_create_rule(make, MRULE(MSTR(STR("check"))), 0));

	if (obj_ext[MK_EXT_ASM] != MAKE_END) {
		const make_if_t if_nasm = make_rule_add_act(make, check, make_create_if(make, MSTR(str_null()), MSTR(STR("$(shell which nasm)"))));
		make_if_add_true_act(make, if_nasm, make_create_cmd(make, MCMD(STR("sudo apt install nasm -y"))));
	}

	if (obj_ext[MK_EXT_S] != MAKE_END || obj_ext[MK_EXT_C] != MAKE_END || obj_ext[MK_EXT_CPP] != MAKE_END) {
		const make_if_t if_gcc = make_rule_add_act(make, check, make_create_if(make, MSTR(str_null()), MSTR(STR("$(shell which gcc)"))));
		make_if_add_true_act(make, if_gcc, make_create_cmd(make, MCMD(STR("sudo apt install gcc -y"))));
	}

	if (target[MK_BUILD_FAT12] != MAKE_END) {
		const make_if_t if_mcopy = make_rule_add_act(make, check, make_create_if(make, MSTR(str_null()), MSTR(STR("$(shell which mcopy)"))));
		make_if_add_true_act(make, if_mcopy, make_create_cmd(make, MCMD(STR("sudo apt install mtools -y"))));
	}

	if (lcov != MAKE_END) {
		const make_if_t if_coverage = make_rule_add_act(make, check, make_create_if(make, MVAR(coverage), MSTR(STR("true"))));
		const make_if_t if_lcov	    = make_if_add_true_act(make, if_coverage, make_create_if(make, MSTR(str_null()), MSTR(STR("$(shell which lcov)"))));
		make_if_add_true_act(make, if_lcov, make_create_cmd(make, MCMD(STR("sudo apt install lcov -y"))));
	}

	for (mk_pgen_build_type_t b = 0; b < __MK_BUILD_MAX; b++) {
		if (target[b] != MAKE_END) {
			const make_rule_t compile = make_add_act(make, make_create_rule(make, MRULE(MSTR(target_c[b].act)), 0));
			make_rule_add_depend(make, compile, MRULE(MSTR(STR("check"))));
			make_rule_add_depend(make, compile, MRULE(MVAR(target[b])));
		}
	}

	if (target[MK_BUILD_EXE] != MAKE_END && ldflags != MAKE_END) {
		make_var_t rtarget = make_add_act(make, make_create_rule(make, MRULE(MVAR(target[MK_BUILD_EXE])), 1));
		for (mk_pgen_src_ext_t s = 0; s < __MK_SRC_MAX; s++) {
			if (obj[target_c[MK_BUILD_EXE].intdir][s] == MAKE_END) {
				continue;
			}

			make_rule_add_depend(make, rtarget, MRULE(MVAR(obj[target_c[MK_BUILD_EXE].intdir][s])));
		}

		make_rule_add_act(make, rtarget, make_create_cmd(make, MCMD(STR("@mkdir -p $(@D)"))));
		make_rule_add_act(make, rtarget, make_create_cmd(make, MCMD(strf("@$(TCC) -m$(BITS) $^ $(LDFLAGS) -o $@"))));
	}

	if (target[MK_BUILD_STATIC] != MAKE_END) {
		make_var_t rtarget = make_add_act(make, make_create_rule(make, MRULE(MVAR(target[MK_BUILD_STATIC])), 1));
		for (mk_pgen_src_ext_t s = 0; s < __MK_SRC_MAX; s++) {
			if (obj[target_c[MK_BUILD_STATIC].intdir][s] == MAKE_END) {
				continue;
			}

			make_rule_add_depend(make, rtarget, MRULE(MVAR(obj[target_c[MK_BUILD_STATIC].intdir][s])));
		}

		make_rule_add_act(make, rtarget, make_create_cmd(make, MCMD(STR("@mkdir -p $(@D)"))));
		make_rule_add_act(make, rtarget, make_create_cmd(make, MCMD(STR("@ar rcs $@ $^"))));
	}

	if (target[MK_BUILD_SHARED] != MAKE_END && ldflags != MAKE_END) {
		make_var_t rtarget = make_add_act(make, make_create_rule(make, MRULE(MVAR(target[MK_BUILD_SHARED])), 1));
		for (mk_pgen_src_ext_t s = 0; s < __MK_SRC_MAX; s++) {
			if (obj[target_c[MK_BUILD_SHARED].intdir][s] == MAKE_END) {
				continue;
			}

			make_rule_add_depend(make, rtarget, MRULE(MVAR(obj[target_c[MK_BUILD_SHARED].intdir][s])));
		}

		make_rule_add_act(make, rtarget, make_create_cmd(make, MCMD(STR("@mkdir -p $(@D)"))));
		make_rule_add_act(make, rtarget, make_create_cmd(make, MCMD(STR("@$(TCC) -shared $(LDFLAGS) $^ -o $@"))));
	}

	if (target[MK_BUILD_BIN] != MAKE_END) {
		make_var_t rtarget = make_add_act(make, make_create_rule(make, MRULE(MVAR(target[MK_BUILD_BIN])), 1));
		make_rule_add_act(make, rtarget, make_create_cmd(make, MCMD(STR("@mkdir -p $(@D)"))));

		if (ldflags != MAKE_END) {
			for (mk_pgen_src_ext_t s = 0; s < __MK_SRC_MAX; s++) {
				if (obj[target_c[MK_BUILD_BIN].intdir][s] == MAKE_END) {
					continue;
				}

				make_rule_add_depend(make, rtarget, MRULE(MVAR(obj[target_c[MK_BUILD_BIN].intdir][s])));
			}

			make_rule_add_act(make, rtarget, make_create_cmd(make, MCMD(STR("@$(TLD) -Tlinker.ld --oformat binary $(LDFLAGS) $^ -o $@"))));
		} else {
			const mk_pgen_file_data_t *file;
			arr_foreach(&gen->files, file)
			{
				switch (file->ext) {
				case MK_EXT_BIN:
					make_rule_add_act(make, rtarget,
							  make_create_cmd(make, MCMD(strf("@dd if=%.*s status=none >> $@", file->path.len, file->path.data))));
					break;

				case MK_EXT_ELF:
					make_rule_add_act(make, rtarget,
							  make_create_cmd(make, MCMD(strf("@objcopy -O binary -j .text %.*s %.*s.bin", file->path.len, file->path.data,
											  file->path.len, file->path.data))));
					make_rule_add_act(make, rtarget,
							  make_create_cmd(make, MCMD(strf("@dd if=%.*s.bin status=none >> $@", file->path.len, file->path.data))));
					break;
				}
			}

			if (gen->size.data) {
				make_rule_add_act(make, rtarget,
						  make_create_cmd(make, MCMD(strf("@dd if=/dev/zero bs=1 count=%.*s status=none >> $@", gen->size.len, gen->size.data))));
			}
		}
	}

	if (target[MK_BUILD_ELF] != MAKE_END && ldflags != MAKE_END) {
		make_var_t rtarget = make_add_act(make, make_create_rule(make, MRULE(MVAR(target[MK_BUILD_ELF])), 1));
		for (mk_pgen_src_ext_t s = 0; s < __MK_SRC_MAX; s++) {
			if (obj[target_c[MK_BUILD_ELF].intdir][s] == MAKE_END) {
				continue;
			}

			make_rule_add_depend(make, rtarget, MRULE(MVAR(obj[target_c[MK_BUILD_ELF].intdir][s])));
		}

		make_rule_add_act(make, rtarget, make_create_cmd(make, MCMD(STR("@mkdir -p $(@D)"))));
		make_rule_add_act(make, rtarget, make_create_cmd(make, MCMD(STR("@$(TLD) -Tlinker.ld $(LDFLAGS) $^ -o $@"))));
	}

	if (target[MK_BUILD_FAT12] != MAKE_END) {
		make_var_t rtarget = make_add_act(make, make_create_rule(make, MRULE(MVAR(target[MK_BUILD_FAT12])), 1));
		make_rule_add_act(make, rtarget, make_create_cmd(make, MCMD(STR("@mkdir -p $(@D)"))));

		//create empty 1.44MB image (block size = 512, block count = 2880)
		make_rule_add_act(make, rtarget, make_create_cmd(make, MCMD(STR("@dd if=/dev/zero of=$@ bs=512 count=2880 status=none"))));
		//create file system
		make_rule_add_act(make, rtarget, make_create_cmd(make, MCMD(STR("@mkfs.fat -F12 -n \"NBOS\" $@"))));
		//put first binary to the first sector of the disk
		make_rule_add_act(make, rtarget, make_create_cmd(make, MCMD(STR("@dd if=$< of=$@ conv=notrunc status=none"))));
		//copy files to the image
		make_rule_add_act(make, rtarget, make_create_cmd(make, MCMD(STR("@mcopy -i $@ $(word 2,$^) \"::$(shell basename $(word 2,$^))\""))));
	}

	for (mk_pgen_intdir_type_t i = 0; i < __MK_INTDIR_MAX; i++) {
		if (obj[i][MK_EXT_ASM] != MAKE_END) {
			const make_rule_t int_o = make_add_act(make, make_create_rule(make, MRULE(MSTR(strf("$(%s)%%.o", intdir_c[i].name.data))), 1));
			make_rule_add_depend(make, int_o, MRULE(MSTR(STR("%.asm"))));
			make_rule_add_act(make, int_o, make_create_cmd(make, MCMD(STR("@mkdir -p $(@D)"))));
			make_rule_add_act(make, int_o,
					  make_create_cmd(make, MCMD(strf("@nasm -felf$(BITS) $(INCLUDES) $(NASM_CONFIG_FLAGS) $(ASFLAGS) $(%s)%s $< -o $@",
									  intdir_c[i].defines.data, intdir_c[i].flags))));
		}
	}

	for (mk_pgen_intdir_type_t i = 0; i < __MK_INTDIR_MAX; i++) {
		if (obj[i][MK_EXT_S] != MAKE_END) {
			const make_rule_t int_o = make_add_act(make, make_create_rule(make, MRULE(MSTR(strf("$(%s)%%.o", intdir_c[i].name.data))), 1));
			make_rule_add_depend(make, int_o, MRULE(MSTR(STR("%.S"))));
			make_rule_add_act(make, int_o, make_create_cmd(make, MCMD(STR("@mkdir -p $(@D)"))));
			make_rule_add_act(make, int_o,
					  make_create_cmd(make, MCMD(strf("@$(TCC) -c -m$(BITS) $(INCLUDES) $(GCC_CONFIG_FLAGS) $(ASFLAGS) $(%s)%s $< -o $@",
									  intdir_c[i].defines.data, intdir_c[i].flags))));
		}
	}

	for (mk_pgen_intdir_type_t i = 0; i < __MK_INTDIR_MAX; i++) {
		if (obj[i][MK_EXT_C] != MAKE_END) {
			const make_rule_t int_o = make_add_act(make, make_create_rule(make, MRULE(MSTR(strf("$(%s)%%.o", intdir_c[i].name.data))), 1));
			make_rule_add_depend(make, int_o, MRULE(MSTR(STR("%.c"))));
			make_rule_add_act(make, int_o, make_create_cmd(make, MCMD(STR("@mkdir -p $(@D)"))));
			make_rule_add_act(make, int_o,
					  make_create_cmd(make, MCMD(strf("@$(TCC) -c -m$(BITS) $(INCLUDES) $(GCC_CONFIG_FLAGS) $(CFLAGS) $(%s)%s $< -o $@",
									  intdir_c[i].defines.data, intdir_c[i].flags))));
		}
	}

	for (mk_pgen_intdir_type_t i = 0; i < __MK_INTDIR_MAX; i++) {
		if (obj[i][MK_EXT_CPP] != MAKE_END) {
			const make_rule_t int_o = make_add_act(make, make_create_rule(make, MRULE(MSTR(strf("$(%s)%%.o", intdir_c[i].name.data))), 1));
			make_rule_add_depend(make, int_o, MRULE(MSTR(STR("%.cpp"))));
			make_rule_add_act(make, int_o, make_create_cmd(make, MCMD(STR("@mkdir -p $(@D)"))));
			make_rule_add_act(make, int_o,
					  make_create_cmd(make, MCMD(strf("@$(TCC) -c -m$(BITS) $(INCLUDES) $(GCC_CONFIG_FLAGS) $(CXXFLAGS) $(%s)%s $< -o $@",
									  intdir_c[i].defines.data, intdir_c[i].flags))));
		}
	}

	// clang-format off
	make_var_t run[] = {
		[MK_BUILD_EXE]	  = MAKE_END,
		[MK_BUILD_STATIC] = MAKE_END,
		[MK_BUILD_SHARED] = MAKE_END,
		[MK_BUILD_BIN]	  = MAKE_END,
		[MK_BUILD_ELF]	  = MAKE_END,
		[MK_BUILD_FAT12]  = MAKE_END,
	};
	// clang-format on

	for (mk_pgen_build_type_t b = 0; b < __MK_BUILD_MAX; b++) {
		if (target[b] == MAKE_END) {
			continue;
		}

		if ((is_debug && gen->run_debug[b].data) || gen->run[b].data || b == MK_BUILD_EXE) {
			run[b] = make_add_act(make, make_create_rule(make, MRULE(MSTR(STR("run"))), 0));
			make_rule_add_depend(make, run[b], MRULE(MSTR(STR("check"))));
			make_rule_add_depend(make, run[b], MRULE(MVAR(target[b])));
		}

		if (is_debug && gen->run_debug[b].data) {
			const make_if_t if_config = make_rule_add_act(make, run[b], make_create_if(make, MVAR(config), MSTR(STR("Debug"))));
			make_if_add_true_act(make, if_config, make_create_cmd(make, MCMD(str_cpy(gen->run_debug[b]))));
			if (gen->run[b].data) {
				make_if_add_false_act(make, if_config, make_create_cmd(make, MCMD(str_cpy(gen->run[b]))));
			} else if (b == MK_BUILD_EXE) {
				make_if_add_false_act(make, if_config, make_create_cmd(make, MCMD(STR("@$(TARGET) $(ARGS)"))));
			}
		} else if (gen->run[b].data) {
			make_rule_add_act(make, run[b], make_create_cmd(make, MCMD(str_cpy(gen->run[b]))));
		} else if (b == MK_BUILD_EXE) {
			make_rule_add_act(make, run[b], make_create_cmd(make, MCMD(STR("@$(TARGET) $(ARGS)"))));
		}

		if (run[b] == MAKE_END || repdir == MAKE_END || lcov == MAKE_END) {
			continue;
		}

		const make_if_t if_coverage = make_rule_add_act(make, run[b], make_create_if(make, MVAR(coverage), MSTR(STR("true"))));
		make_if_add_true_act(make, if_coverage, make_create_cmd(make, MCMD(STR("@lcov -q -c -d $(SLNDIR) -o $(LCOV)"))));
		const make_if_t if_show = make_if_add_true_act(make, if_coverage, make_create_if(make, MVAR(show), MSTR(STR("true"))));
		make_if_add_true_act(make, if_show, make_create_cmd(make, MCMD(STR("@genhtml -q $(LCOV) -o $(REPDIR)"))));
		make_if_add_true_act(make, if_show, make_create_cmd(make, MCMD(STR("@open $(REPDIR)index.html"))));
	}

	// clang-format off
	make_var_t artifact[] = {
		[MK_BUILD_EXE]	  = MAKE_END,
		[MK_BUILD_STATIC] = MAKE_END,
		[MK_BUILD_SHARED] = MAKE_END,
		[MK_BUILD_BIN]	  = MAKE_END,
		[MK_BUILD_ELF]	  = MAKE_END,
		[MK_BUILD_FAT12]  = MAKE_END,
	};

	// clang-format on
	for (mk_pgen_build_type_t b = 0; b < __MK_BUILD_MAX; b++) {
		if (target[b] == MAKE_END || gen->artifact[b].data == NULL) {
			continue;
		}

		artifact[b] = make_add_act(make, make_create_rule(make, MRULE(MSTR(target_c[b].artifact)), 0));
		make_rule_add_depend(make, artifact[b], MRULE(MSTR(STR("check"))));
		make_rule_add_depend(make, artifact[b], MRULE(MVAR(target[b])));
		make_rule_add_act(make, artifact[b], make_create_cmd(make, MCMD(STR("@mkdir -p $(SLNDIR)tmp/artifact/"))));
		make_rule_add_act(make, artifact[b],
				  make_create_cmd(make, MCMD(strf("@cp $(TARGET) $(SLNDIR)tmp/artifact/%.*s", gen->artifact[b].len, gen->artifact[b].data))));
	}

	const make_rule_t clean = make_add_act(make, make_create_rule(make, MRULE(MSTR(STR("clean"))), 0));

	str_t cleans = strz(128);
	str_cat(&cleans, STR("@$(RM)"));

	for (mk_pgen_build_type_t b = 0; b < __MK_BUILD_MAX; b++) {
		if (target[b] == MAKE_END) {
			continue;
		}
		str_cat(&cleans, STR(" $("));
		str_cat(&cleans, target_c[b].name);
		str_cat(&cleans, STR(")"));
	}

	for (mk_pgen_build_type_t b = 0; b < __MK_BUILD_MAX; b++) {
		if (artifact[b] == MAKE_END) {
			continue;
		}
		str_cat(&cleans, STR(" $(SLNDIR)tmp/artifact/"));
		str_cat(&cleans, gen->artifact[b]);
	}

	for (mk_pgen_intdir_type_t i = 0; i < __MK_INTDIR_MAX; i++) {
		for (mk_pgen_src_ext_t s = 0; s < __MK_SRC_MAX; s++) {
			if (obj[i][s] == MAKE_END) {
				continue;
			}

			str_cat(&cleans, STR(" $("));
			str_cat(&cleans, obj_ext_c[i][s].name);
			str_cat(&cleans, STR(")"));
		}
	}

	if (cov != MAKE_END) {
		str_cat(&cleans, STR(" $(COV)"));
	}

	make_rule_add_act(make, clean, make_create_cmd(make, MCMD(cleans)));

	return make;
}

static make_t *mk_pgen_remote(const mk_pgen_t *gen, make_t *make)
{
	make_var_t outdir = MAKE_END;
	if (gen->outdir.data) {
		outdir = make_add_act(make, make_create_var(make, STR("OUTDIR"), MAKE_VAR_INST));
		make_var_add_val(make, outdir, MSTR(str_cpy(gen->outdir)));
	}

	make_var_t url = MAKE_END;
	if (gen->url.data) {
		url = make_add_act(make, make_create_var(make, STR("URL"), MAKE_VAR_INST));
		make_var_add_val(make, url, MSTR(str_cpy(gen->url)));
	}

	make_var_t name = MAKE_END;
	if (gen->name.data) {
		name = make_add_act(make, make_create_var(make, STR("NAME"), MAKE_VAR_INST));
		make_var_add_val(make, name, MSTR(str_cpy(gen->name)));
	}

	make_var_t format = MAKE_END;
	if (gen->format.data) {
		format = make_add_act(make, make_create_var(make, STR("FORMAT"), MAKE_VAR_INST));
		make_var_add_val(make, format, MSTR(str_cpy(gen->format)));
	}

	make_var_t file = MAKE_END;
	if (name != MAKE_END && format != MAKE_END) {
		file = make_add_act(make, make_create_var(make, STR("FILE"), MAKE_VAR_INST));
		make_var_add_val(make, file, MSTR(STR("$(NAME).$(FORMAT)")));
	}

	make_var_t dldir = MAKE_END;
	if (file != MAKE_END) {
		dldir = make_add_act(make, make_create_var(make, STR("DLDIR"), MAKE_VAR_INST));
		make_var_add_val(make, dldir, MSTR(STR("$(SLNDIR)dl/$(FILE)/")));
	}

	make_var_t srcdir   = MAKE_END;
	make_var_t biulddir = MAKE_END;
	make_var_t logdir   = MAKE_END;
	if (name != MAKE_END) {
		srcdir = make_add_act(make, make_create_var(make, STR("SRCDIR"), MAKE_VAR_INST));
		make_var_add_val(make, srcdir, MSTR(STR("$(SLNDIR)staging/$(NAME)/")));

		biulddir = make_add_act(make, make_create_var(make, STR("BUILDDIR"), MAKE_VAR_INST));
		make_var_add_val(make, biulddir, MSTR(STR("$(SLNDIR)build/$(ARCH)/$(NAME)/")));

		logdir = make_add_act(make, make_create_var(make, STR("LOGDIR"), MAKE_VAR_INST));
		make_var_add_val(make, logdir, MSTR(STR("$(SLNDIR)logs/$(ARCH)/$(NAME)/")));
	}

	make_add_act(make, make_create_empty(make));

	const make_rule_t all = make_add_act(make, make_create_rule(make, MRULE(MSTR(STR("all"))), 0));
	make_rule_add_depend(make, all, MRULE(MSTR(STR("compile"))));

	const make_rule_t check = make_add_act(make, make_create_rule(make, MRULE(MSTR(STR("check"))), 0));

	if (url != MAKE_END) {
		const make_if_t if_curl = make_rule_add_act(make, check, make_create_if(make, MSTR(str_null()), MSTR(STR("$(shell which curl)"))));
		make_if_add_true_act(make, if_curl, make_create_cmd(make, MCMD(STR("sudo apt install curl -y"))));
	}

	const str_t *require;
	arr_foreach(&gen->requires, require)
	{
		make_if_t if_dpkg =
			make_rule_add_act(make, check, make_create_if(make, MSTR(str_null()), MSTR(strf("$(shell dpkg -l %.*s)", require->len, require->data))));
		make_if_add_true_act(make, if_dpkg, make_create_cmd(make, MCMD(strf("sudo apt install %.*s -y", require->len, require->data))));
	}

	if (dldir != MAKE_END && url != MAKE_END && file != MAKE_END) {
		const make_rule_t rdldir = make_add_act(make, make_create_rule(make, MRULE(MVAR(dldir)), 1));
		make_rule_add_act(make, rdldir, make_create_cmd(make, MCMD(STR("@mkdir -p $(@D)"))));
		make_rule_add_act(make, rdldir, make_create_cmd(make, MCMD(STR("@cd $(@D) && curl -O $(URL)$(FILE)"))));
	}

	if (dldir != MAKE_END && srcdir != MAKE_END) {
		const make_rule_t rsrcdir = make_add_act(make, make_create_rule(make, MRULEACT(MVAR(srcdir), STR("done")), 1));
		make_rule_add_depend(make, rsrcdir, MRULE(MVAR(dldir)));
		make_rule_add_act(make, rsrcdir, make_create_cmd(make, MCMD(STR("@mkdir -p $(SLNDIR)staging"))));
		make_rule_add_act(make, rsrcdir, make_create_cmd(make, MCMD(STR("@tar xf $(DLDIR) -C $(SLNDIR)staging"))));
		make_rule_add_act(make, rsrcdir, make_create_cmd(make, MCMD(STR("@touch $(SRCDIR)done"))));
	}

	if (outdir != MAKE_END && name != MAKE_END && logdir != MAKE_END && biulddir != MAKE_END) {
		const make_rule_t routdir = make_add_act(make, make_create_rule(make, MRULEACT(MSTR(STR("$(OUTDIR)")), STR("$(NAME)")), 1));
		make_rule_add_depend(make, routdir, MRULEACT(MVAR(srcdir), STR("done")));
		make_rule_add_act(make, routdir, make_create_cmd(make, MCMD(STR("@mkdir -p $(LOGDIR) $(BUILDDIR) $(OUTDIR)"))));
		if (gen->config.data) {
			make_rule_add_act(
				make, routdir,
				make_create_cmd(
					make,
					MCMD(strf("@cd $(BUILDDIR) && $(SRCDIR)configure --target=$(ARCH)-elf --prefix=$(OUTDIR) %.*s 2>&1 | tee $(LOGDIR)configure.log",
						  gen->config.len, gen->config.data))));
		}
		if (gen->targets.data) {
			make_rule_add_act(make, routdir,
					  make_create_cmd(make,
							  MCMD(strf("@cd $(BUILDDIR) && make %.*s 2>&1 | tee $(LOGDIR)make.log", gen->targets.len, gen->targets.data))));
		}
		make_rule_add_act(make, routdir, make_create_cmd(make, MCMD(STR("@touch $(OUTDIR)$(NAME)"))));
	}

	if (name != MAKE_END) {
		const make_rule_t compile = make_add_act(make, make_create_rule(make, MRULE(MSTR(STR("compile"))), 0));
		make_rule_add_depend(make, compile, MRULE(MSTR(STR("check"))));
		make_rule_add_depend(make, compile, MRULEACT(MSTR(STR("$(OUTDIR)")), STR("$(NAME)")));
	}

	make_add_act(make, make_create_rule(make, MRULE(MSTR(STR("clean"))), 0));

	return make;
}

make_t *mk_pgen(const mk_pgen_t *gen, make_t *make)
{
	if (gen == NULL || make == NULL) {
		return NULL;
	}

	make_create_var_ext(make, STR("SLNDIR"), MAKE_VAR_INST);

	if (gen->url.data) {
		return mk_pgen_remote(gen, make);
	}

	return mk_pgen_local(gen, make);
}
