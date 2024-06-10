#include "gen/mk/mk_gen.h"

#include "make.h"

typedef struct mk_gen_header_data_s {
	str_t dir;
	mk_gen_header_ext_t exts;
} mk_gen_header_data_t;

typedef struct mk_gen_src_data_s {
	str_t dir;
	mk_gen_src_ext_t exts;
} mk_gen_src_data_t;

mk_gen_t *mk_gen_init(mk_gen_t *gen)
{
	if (gen == NULL) {
		return NULL;
	}

	if (arr_init(&gen->headers, 1, sizeof(mk_gen_header_data_t)) == NULL) {
		return NULL;
	}

	if (arr_init(&gen->srcs, 1, sizeof(mk_gen_src_data_t)) == NULL) {
		return NULL;
	}

	gen->includes = strz(64);

	for (mk_gen_src_ext_t s = 0; s < __MK_SRC_MAX; s++) {
		gen->flags[s] = strz(16);
	}

	for (mk_gen_intdir_type_t i = 0; i < __MK_INTDIR_MAX; i++) {
		gen->defines[i] = strz(16);
	}

	gen->ldflags = strz(16);

	return gen;
}

void mk_gen_free(mk_gen_t *gen)
{
	if (gen == NULL) {
		return;
	}

	str_free(&gen->outdir);

	for (mk_gen_intdir_type_t i = 0; i < __MK_INTDIR_MAX; i++) {
		str_free(&gen->intdir[i]);
	}

	str_free(&gen->covdir);

	mk_gen_header_data_t *header;
	arr_foreach(&gen->headers, header)
	{
		str_free(&header->dir);
	}
	arr_free(&gen->headers);

	mk_gen_src_data_t *src;
	arr_foreach(&gen->srcs, src)
	{
		str_free(&src->dir);
	}
	arr_free(&gen->srcs);

	for (mk_gen_target_type_t t = 0; t < __MK_TARGET_MAX; t++) {
		str_free(&gen->target[t]);
	}

	str_free(&gen->args);
	str_free(&gen->includes);

	for (mk_gen_src_ext_t s = 0; s < __MK_SRC_MAX; s++) {
		str_free(&gen->flags[s]);
	}

	for (mk_gen_intdir_type_t i = 0; i < __MK_INTDIR_MAX; i++) {
		str_free(&gen->defines[i]);
	}

	str_free(&gen->ldflags);

	for (mk_gen_target_type_t t = 0; t < __MK_TARGET_MAX; t++) {
		str_free(&gen->run[t]);
	}

	for (mk_gen_target_type_t t = 0; t < __MK_TARGET_MAX; t++) {
		str_free(&gen->run_debug[t]);
	}

	for (mk_gen_target_type_t t = 0; t < __MK_TARGET_MAX; t++) {
		str_free(&gen->artifact[t]);
	}
}

uint mk_gen_add_header(mk_gen_t *gen, str_t dir, mk_gen_header_ext_t exts)
{
	if (gen == NULL) {
		return MK_HEADER_END;
	}

	uint id = arr_add(&gen->headers);

	mk_gen_header_data_t *header = arr_get(&gen->headers, id);
	if (header == NULL) {
		return MK_HEADER_END;
	}

	*header = (mk_gen_header_data_t){
		.dir  = dir,
		.exts = exts,
	};

	return id;
}

uint mk_gen_add_src(mk_gen_t *gen, str_t dir, mk_gen_src_ext_t exts)
{
	if (gen == NULL) {
		return MK_SRC_END;
	}

	uint id = arr_add(&gen->srcs);

	mk_gen_src_data_t *src = arr_get(&gen->srcs, id);
	if (src == NULL) {
		return MK_SRC_END;
	}

	*src = (mk_gen_src_data_t){
		.dir  = dir,
		.exts = exts,
	};

	return id;
}

void mk_gen_add_include(mk_gen_t *gen, str_t dir)
{
	if (gen == NULL) {
		return;
	}

	str_cat(&gen->includes, gen->includes.len > 0 ? STR(" -I") : STR("-I"));
	str_cat(&gen->includes, dir);
}

void mk_gen_add_flag(mk_gen_t *gen, str_t flag, mk_gen_src_ext_t exts)
{
	if (gen == NULL) {
		return;
	}

	for (mk_gen_src_ext_t s = 0; s < __MK_SRC_MAX; s++) {
		if ((exts & (1 << s)) == 0) {
			continue;
		}
		if (gen->flags[s].len > 0) {
			str_cat(&gen->flags[s], STR(" "));
		}
		str_cat(&gen->flags[s], flag);
	}
}

void mk_gen_add_define(mk_gen_t *gen, str_t define, mk_gen_intdir_type_t intdirs)
{
	if (gen == NULL) {
		return;
	}

	for (mk_gen_intdir_type_t i = 0; i < __MK_INTDIR_MAX; i++) {
		if ((intdirs & (1 << i)) == 0) {
			continue;
		}

		str_cat(&gen->defines[i], gen->defines[i].len > 0 ? STR(" -D") : STR("-D"));
		str_cat(&gen->defines[i], define);
	}
}

void mk_gen_add_ldflag(mk_gen_t *gen, str_t ldflag)
{
	if (gen == NULL) {
		return;
	}

	if (gen->ldflags.len > 0) {
		str_cat(&gen->ldflags, STR(" "));
	}
	str_cat(&gen->ldflags, ldflag);
}

void mk_gen_set_run(mk_gen_t *gen, str_t run, mk_gen_target_type_t targets)
{
	if (gen == NULL) {
		return;
	}

	for (mk_gen_target_type_t t = 0; t < __MK_TARGET_MAX; t++) {
		if ((targets & (1 << t)) == 0) {
			continue;
		}

		gen->run[t] = run;
	}
}

void mk_gen_set_run_debug(mk_gen_t *gen, str_t run, mk_gen_target_type_t targets)
{
	if (gen == NULL) {
		return;
	}

	for (mk_gen_target_type_t t = 0; t < __MK_TARGET_MAX; t++) {
		if ((targets & (1 << t)) == 0) {
			continue;
		}

		gen->run_debug[t] = run;
	}
}

int mk_gen_print(const mk_gen_t *gen, print_dst_t dst)
{
	if (gen == NULL) {
		return 0;
	}

	make_t make = { 0 };
	make_init(&make, 8, 8, 8);

	make_create_var_ext(&make, STR("SLNDIR"), MAKE_VAR_INST);
	const make_var_t platform = make_create_var_ext(&make, STR("PLATFORM"), MAKE_VAR_INST);
	const make_var_t config	  = make_create_var_ext(&make, STR("CONFIG"), MAKE_VAR_INST);
	const make_var_t coverage = make_create_var_ext(&make, STR("COVERAGE"), MAKE_VAR_INST);
	const make_var_t show	  = make_create_var_ext(&make, STR("SHOW"), MAKE_VAR_INST);

	int is_vars = 0;

	make_var_t outdir = MAKE_END;
	if (gen->outdir.data) {
		is_vars = 1;

		outdir = make_add_act(&make, make_create_var(&make, STR("OUTDIR"), MAKE_VAR_INST));
		make_var_add_val(&make, outdir, MSTR(strs(gen->outdir)));
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

	for (mk_gen_intdir_type_t i = 0; i < __MK_INTDIR_MAX; i++) {
		if (gen->intdir[i].data == NULL) {
			continue;
		}

		is_vars = 1;

		intdir[i] = make_add_act(&make, make_create_var(&make, intdir_c[i].name, MAKE_VAR_INST));
		make_var_add_val(&make, intdir[i], MSTR(strs(gen->intdir[i])));
	}

	make_var_t covdir = MAKE_END;
	if (gen->covdir.data) {
		is_vars = 1;

		covdir = make_add_act(&make, make_create_var(&make, STR("COVDIR"), MAKE_VAR_INST));
		make_var_add_val(&make, covdir, MSTR(strs(gen->covdir)));
	}

	static const char *header_ext[] = {
		[MK_EXT_INC] = "inc",
		[MK_EXT_H]   = "h",
		[MK_EXT_HPP] = "hpp",
	};

	make_var_t headers = MAKE_END;
	mk_gen_header_data_t *header;
	arr_foreach(&gen->headers, header)
	{
		for (mk_gen_header_ext_t ext = 0; ext < __MK_HEADER_MAX; ext++) {
			if ((header->exts & (1 << ext)) == 0) {
				continue;
			}

			is_vars = 1;

			headers = make_add_act(&make, make_create_var(&make, STR("HEADERS"), headers == MAKE_END ? MAKE_VAR_INST : MAKE_VAR_APP));
			make_var_add_val(&make, headers, MSTR(strf("$(shell find %.*s -name '*.%s')", header->dir.len, header->dir.data, header_ext[ext])));
		}
	}

	// clang-format off
	make_var_t target[] = {
		[MK_TARGET_EXE]	   = MAKE_END,
		[MK_TARGET_STATIC] = MAKE_END,
		[MK_TARGET_SHARED] = MAKE_END,
		[MK_TARGET_BIN]	   = MAKE_END,
		[MK_TARGET_ELF]	   = MAKE_END,
		[MK_TARGET_FAT12]  = MAKE_END,
	};

	static struct {
		str_t name;
		const char *ext;
		str_t act;
		mk_gen_intdir_type_t intdir;
		str_t run;
		str_t artifact;
	} target_c[] = {
		[MK_TARGET_EXE]	   = { STRS("TARGET"),	     "",     STRS("compile"), MK_INTDIR_OBJECT, STRS("run"),	   STRS("artifact")	  },
		[MK_TARGET_STATIC] = { STRS("TARGET_S"),     ".a",   STRS("static"),  MK_INTDIR_STATIC, STRS("run_s"),	   STRS("artifact_s")	  },
		[MK_TARGET_SHARED] = { STRS("TARGET_D"),     ".so",  STRS("shared"),  MK_INTDIR_SHARED, STRS("run_d"),	   STRS("artifact_d")	  },
		[MK_TARGET_BIN]	   = { STRS("TARGET_BIN"),   ".bin", STRS("bin"),     __MK_INTDIR_MAX,  STRS("run_bin"),   STRS("artifact_bin")	  }, 
		[MK_TARGET_ELF]	   = { STRS("TARGET_ELF"),   ".elf", STRS("elf"),     MK_INTDIR_OBJECT, STRS("run_elf"),   STRS("artifact_elf")	  }, 
		[MK_TARGET_FAT12]  = { STRS("TARGET_FAT12"), ".img", STRS("fat12"),   __MK_INTDIR_MAX,  STRS("run_fat12"), STRS("artifact_fat12") },
	};
	// clang-format on

	for (mk_gen_target_type_t t = 0; t < __MK_TARGET_MAX; t++) {
		if (outdir == MAKE_END || gen->target[t].data == NULL) {
			continue;
		}

		is_vars = 1;

		target[t] = make_add_act(&make, make_create_var(&make, target_c[t].name, MAKE_VAR_INST));
		make_var_add_val(&make, target[t], MSTR(strf("$(OUTDIR)%.*s%s", gen->target[t].len, gen->target[t].data, target_c[t].ext)));
	}

	make_var_t args = MAKE_END;
	if (target[MK_TARGET_EXE] != MAKE_END) {
		args = make_add_act(&make, make_create_var(&make, STR("ARGS"), MAKE_VAR_INST));
		if (gen->args.len > 0) {
			make_var_add_val(&make, args, MSTR(strs(gen->args)));
		}
	}

	// clang-format off
	static struct {
		str_t name;
		str_t flags;
		const char *ext;
		int cov;
	} src_c[] = {
		[MK_EXT_ASM] = { STRS("SRC_ASM"), STRS("ASFLAGS"),  "asm", 0 },
		[MK_EXT_C]   = { STRS("SRC_C"),	  STRS("CFLAGS"),   "c",   1 },
		[MK_EXT_CPP] = { STRS("SRC_CPP"), STRS("CXXFLAGS"), "cpp", 1 },
	};
	// clang-format on

	make_var_t src[] = {
		[MK_EXT_ASM] = MAKE_END,
		[MK_EXT_C]   = MAKE_END,
		[MK_EXT_CPP] = MAKE_END,
	};

	for (mk_gen_src_ext_t s = 0; s < __MK_SRC_MAX; s++) {
		mk_gen_src_data_t *data;
		arr_foreach(&gen->srcs, data)
		{
			if ((data->exts & (1 << s)) == 0) {
				continue;
			}

			is_vars = 1;

			src[s] = make_add_act(&make, make_create_var(&make, src_c[s].name, src[s] == MAKE_END ? MAKE_VAR_INST : MAKE_VAR_APP));
			make_var_add_val(&make, src[s], MSTR(strf("$(shell find %.*s -name '*.%s')", data->dir.len, data->dir.data, src_c[s].ext)));
		}
	}

	make_var_t obj[][__MK_SRC_MAX] = {
		[MK_INTDIR_OBJECT] = {
			[MK_EXT_ASM] = MAKE_END,
			[MK_EXT_C]   = MAKE_END,
			[MK_EXT_CPP] = MAKE_END,
		},
		[MK_INTDIR_STATIC] = {
			[MK_EXT_ASM] = MAKE_END,
			[MK_EXT_C]   = MAKE_END,
			[MK_EXT_CPP] = MAKE_END,
		},
		[MK_INTDIR_SHARED] = {
			[MK_EXT_ASM] = MAKE_END,
			[MK_EXT_C]   = MAKE_END,
			[MK_EXT_CPP] = MAKE_END,
		},
	};

	make_var_t obj_ext[] = {
		[MK_EXT_ASM] = MAKE_END,
		[MK_EXT_C]   = MAKE_END,
		[MK_EXT_CPP] = MAKE_END,
	};

	static struct {
		str_t name;
	} obj_ext_c[][__MK_SRC_MAX] = {
		[MK_INTDIR_OBJECT] = {
			[MK_EXT_ASM] = {STRS("OBJ_ASM") },
			[MK_EXT_C]   = {STRS("OBJ_C")  },
			[MK_EXT_CPP] = {STRS("OBJ_CPP") },
		},
		[MK_INTDIR_STATIC] = {
			[MK_EXT_ASM] = {STRS("OBJ_ASM_S") },
			[MK_EXT_C]   = {STRS("OBJ_C_S")  },
			[MK_EXT_CPP] = {STRS("OBJ_CPP_S") },
		},
		[MK_INTDIR_SHARED] = {
			[MK_EXT_ASM] = {STRS("OBJ_ASM_D") },
			[MK_EXT_C]   = {STRS("OBJ_C_D")  },
			[MK_EXT_CPP] = {STRS("OBJ_CPP_D") },
		}, 
	};

	int is_obj = 0;

	for (mk_gen_intdir_type_t i = 0; i < __MK_INTDIR_MAX; i++) {
		if (intdir[i] == MAKE_END) {
			continue;
		}

		for (mk_gen_src_ext_t s = 0; s < __MK_SRC_MAX; s++) {
			if (!obj_ext_c[i][s].name.data || src[s] == MAKE_END) {
				continue;
			}

			is_vars = 1;
			is_obj	= 1;

			obj_ext[s] = obj[i][s] = make_add_act(&make, make_create_var(&make, obj_ext_c[i][s].name, MAKE_VAR_INST));
			make_var_add_val(&make, obj[i][s], MSTR(strf("$(patsubst %%.%s, $(%s)%%.o, $(%s))", src_c[s].ext, intdir_c[i].name.data, src_c[s].name.data)));
		}
	}

	make_var_t cov = MAKE_END;
	for (mk_gen_src_ext_t s = 0; s < __MK_SRC_MAX; s++) {
		if (covdir == MAKE_END || src[s] == MAKE_END || src_c[s].cov == 0) {
			continue;
		}

		is_vars = 1;

		cov = make_add_act(&make, make_create_var(&make, STR("COV"), cov == MAKE_END ? MAKE_VAR_INST : MAKE_VAR_APP));
		make_var_add_val(&make, cov, MSTR(strf("$(patsubst %%.%s, $(COVDIR)%%.gcno, $(%s))", src_c[s].ext, src_c[s].name.data)));
		cov = make_add_act(&make, make_create_var(&make, STR("COV"), MAKE_VAR_APP));
		make_var_add_val(&make, cov, MSTR(strf("$(patsubst %%.%s, $(COVDIR)%%.gcda, $(%s))", src_c[s].ext, src_c[s].name.data)));
	}

	make_var_t repdir = MAKE_END;
	make_var_t lcov	  = MAKE_END;
	if (target[MK_TARGET_EXE] != MAKE_END && covdir != MAKE_END) {
		is_vars = 1;

		repdir = make_add_act(&make, make_create_var(&make, STR("REPDIR"), MAKE_VAR_INST));
		make_var_add_val(&make, repdir, MSTR(STR("$(COVDIR)coverage-report/")));
		lcov = make_add_act(&make, make_create_var(&make, STR("LCOV"), MAKE_VAR_INST));
		make_var_add_val(&make, lcov, MSTR(STR("$(COVDIR)lcov.info")));

		cov = make_add_act(&make, make_create_var(&make, STR("COV"), cov == MAKE_END ? MAKE_VAR_INST : MAKE_VAR_APP));
		make_var_add_val(&make, cov, MVAR(lcov));
		make_var_add_val(&make, cov, MVAR(repdir));
	}

	if (is_vars) {
		make_add_act(&make, make_create_empty(&make));
	}

	int is_flags = 0;

	make_var_t includes = MAKE_END;
	if (is_obj && gen->includes.len > 0) {
		is_flags = 1;

		includes = make_add_act(&make, make_create_var(&make, STR("INCLUDES"), MAKE_VAR_INST));
		make_var_add_val(&make, includes, MSTR(strs(gen->includes)));
	}

	make_var_t flags[] = {
		[MK_EXT_ASM] = MAKE_END,
		[MK_EXT_C]   = MAKE_END,
		[MK_EXT_CPP] = MAKE_END,
	};

	for (mk_gen_src_ext_t s = 0; s < __MK_SRC_MAX; s++) {
		if (obj_ext[s] == MAKE_END || gen->flags[s].len == 0) {
			continue;
		}

		is_flags = 1;

		flags[s] = make_add_act(&make, make_create_var(&make, src_c[s].flags, MAKE_VAR_INST));
		make_var_add_val(&make, flags[s], MSTR(strs(gen->flags[s])));
	}

	make_var_t defines[] = {
		[MK_INTDIR_OBJECT] = MAKE_END,
		[MK_INTDIR_STATIC] = MAKE_END,
		[MK_INTDIR_SHARED] = MAKE_END,
	};

	for (mk_gen_intdir_type_t i = 0; i < __MK_INTDIR_MAX; i++) {
		if (target[i] == MAKE_END || gen->defines[i].len == 0) {
			continue;
		}

		is_flags = 1;

		defines[i] = make_add_act(&make, make_create_var(&make, intdir_c[i].defines, MAKE_VAR_INST));
		make_var_add_val(&make, defines[i], MSTR(strs(gen->defines[i])));
	}

	make_var_t ldflags = MAKE_END;
	if (is_obj) {
		is_flags = 1;

		ldflags = make_add_act(&make, make_create_var(&make, STR("LDFLAGS"), MAKE_VAR_INST));
		if (gen->ldflags.len > 0) {
			make_var_add_val(&make, ldflags, MSTR(strs(gen->ldflags)));
		}
	}

	make_var_t config_flags = MAKE_END;
	if (is_obj) {
		is_flags = 1;

		config_flags = make_add_act(&make, make_create_var(&make, STR("CONFIG_FLAGS"), MAKE_VAR_INST));
	}

	if (is_flags) {
		make_add_act(&make, make_create_empty(&make));
	}

	const make_var_t rm = make_add_act(&make, make_create_var(&make, STR("RM"), MAKE_VAR_APP));
	make_var_add_val(&make, rm, MSTR(STR("-r")));

	make_add_act(&make, make_create_empty(&make));

	if (config_flags != MAKE_END) {
		const make_if_t if_config	   = make_add_act(&make, make_create_if(&make, MVAR(config), MSTR(STR("Debug"))));
		const make_var_t config_flags_true = make_if_add_true_act(&make, if_config, make_create_var(&make, STR("CONFIG_FLAGS"), MAKE_VAR_APP));
		make_var_add_val(&make, config_flags_true, MSTR(STR("-ggdb3 -O0")));
		make_add_act(&make, make_create_empty(&make));

		if (cov != MAKE_END) {
			const make_if_t if_cov		= make_add_act(&make, make_create_if(&make, MVAR(coverage), MSTR(STR("true"))));
			const make_var_t cov_flags_true = make_if_add_true_act(&make, if_cov, make_create_var(&make, STR("CONFIG_FLAGS"), MAKE_VAR_APP));
			make_var_add_val(&make, cov_flags_true, MSTR(STR("--coverage -fprofile-abs-path")));
			make_add_act(&make, make_create_empty(&make));
		}
	}

	const make_rule_t all = make_add_act(&make, make_create_rule(&make, MRULE(MSTR(STR("all"))), 0));
	for (mk_gen_target_type_t t = 0; t < __MK_TARGET_MAX; t++) {
		if (target[t] == MAKE_END) {
			continue;
		}

		make_rule_add_depend(&make, all, MRULE(MSTR(target_c[t].act)));
	}

	const make_rule_t check = make_add_act(&make, make_create_rule(&make, MRULE(MSTR(STR("check"))), 0));

	if (obj_ext[MK_EXT_ASM] != MAKE_END) {
		const make_if_t if_nasm = make_rule_add_act(&make, check, make_create_if(&make, MSTR(str_null()), MSTR(STR("$(shell which nasm)"))));
		make_if_add_true_act(&make, if_nasm, make_create_cmd(&make, MCMD(STR("sudo apt install nasm -y"))));
	}

	if (obj_ext[MK_EXT_C] != MAKE_END || obj_ext[MK_EXT_CPP] != MAKE_END) {
		const make_if_t if_gcc = make_rule_add_act(&make, check, make_create_if(&make, MSTR(str_null()), MSTR(STR("$(shell which gcc)"))));
		make_if_add_true_act(&make, if_gcc, make_create_cmd(&make, MCMD(STR("sudo apt install gcc -y"))));
	}

	if (target[MK_TARGET_FAT12] != MAKE_END) {
		const make_if_t if_mcopy = make_rule_add_act(&make, check, make_create_if(&make, MSTR(str_null()), MSTR(STR("$(shell which mcopy)"))));
		make_if_add_true_act(&make, if_mcopy, make_create_cmd(&make, MCMD(STR("sudo apt install mtools -y"))));
	}

	if (lcov != MAKE_END) {
		const make_if_t if_coverage = make_rule_add_act(&make, check, make_create_if(&make, MVAR(coverage), MSTR(STR("true"))));
		const make_if_t if_lcov	    = make_if_add_true_act(&make, if_coverage, make_create_if(&make, MSTR(str_null()), MSTR(STR("$(shell which lcov)"))));
		make_if_add_true_act(&make, if_lcov, make_create_cmd(&make, MCMD(STR("sudo apt install lcov -y"))));
	}

	for (mk_gen_target_type_t t = 0; t < __MK_TARGET_MAX; t++) {
		if (target[t] != MAKE_END) {
			const make_rule_t compile = make_add_act(&make, make_create_rule(&make, MRULE(MSTR(target_c[t].act)), 0));
			make_rule_add_depend(&make, compile, MRULE(MSTR(STR("check"))));
			make_rule_add_depend(&make, compile, MRULE(MVAR(target[t])));
		}
	}

	if (target[MK_TARGET_EXE] != MAKE_END && ldflags != MAKE_END) {
		make_var_t rtarget = make_add_act(&make, make_create_rule(&make, MRULE(MVAR(target[MK_TARGET_EXE])), 1));
		for (mk_gen_src_ext_t s = 0; s < __MK_SRC_MAX; s++) {
			if (obj[target_c[MK_TARGET_EXE].intdir][s] == MAKE_END) {
				continue;
			}

			make_rule_add_depend(&make, rtarget, MRULE(MVAR(obj[target_c[MK_TARGET_EXE].intdir][s])));
		}

		make_rule_add_act(&make, rtarget, make_create_cmd(&make, MCMD(STR("@mkdir -p $(@D)"))));
		make_rule_add_act(&make, rtarget, make_create_cmd(&make, MCMD(STR("@$(TCC) -o $@ $^ $(LDFLAGS)"))));
	}

	if (target[MK_TARGET_STATIC] != MAKE_END) {
		make_var_t rtarget = make_add_act(&make, make_create_rule(&make, MRULE(MVAR(target[MK_TARGET_STATIC])), 1));
		for (mk_gen_src_ext_t s = 0; s < __MK_SRC_MAX; s++) {
			if (obj[target_c[MK_TARGET_STATIC].intdir][s] == MAKE_END) {
				continue;
			}

			make_rule_add_depend(&make, rtarget, MRULE(MVAR(obj[target_c[MK_TARGET_STATIC].intdir][s])));
		}

		make_rule_add_act(&make, rtarget, make_create_cmd(&make, MCMD(STR("@mkdir -p $(@D)"))));
		make_rule_add_act(&make, rtarget, make_create_cmd(&make, MCMD(STR("@ar rcs $@ $^"))));
	}

	if (target[MK_TARGET_SHARED] != MAKE_END && ldflags != MAKE_END) {
		make_var_t rtarget = make_add_act(&make, make_create_rule(&make, MRULE(MVAR(target[MK_TARGET_SHARED])), 1));
		for (mk_gen_src_ext_t s = 0; s < __MK_SRC_MAX; s++) {
			if (obj[target_c[MK_TARGET_SHARED].intdir][s] == MAKE_END) {
				continue;
			}

			make_rule_add_depend(&make, rtarget, MRULE(MVAR(obj[target_c[MK_TARGET_SHARED].intdir][s])));
		}

		make_rule_add_act(&make, rtarget, make_create_cmd(&make, MCMD(STR("@mkdir -p $(@D)"))));
		make_rule_add_act(&make, rtarget, make_create_cmd(&make, MCMD(STR("@$(TCC) -shared -o $@ $^ $(LDFLAGS)"))));
	}

	if (target[MK_TARGET_ELF] != MAKE_END && ldflags != MAKE_END) {
		make_var_t rtarget = make_add_act(&make, make_create_rule(&make, MRULE(MVAR(target[MK_TARGET_ELF])), 1));
		for (mk_gen_src_ext_t s = 0; s < __MK_SRC_MAX; s++) {
			if (obj[target_c[MK_TARGET_ELF].intdir][s] == MAKE_END) {
				continue;
			}

			make_rule_add_depend(&make, rtarget, MRULE(MVAR(obj[target_c[MK_TARGET_ELF].intdir][s])));
		}

		make_rule_add_act(&make, rtarget, make_create_cmd(&make, MCMD(STR("@mkdir -p $(@D)"))));
		make_rule_add_act(&make, rtarget, make_create_cmd(&make, MCMD(STR("@$(TLD) -Tlinker.ld -o $@ $^ $(LDFLAGS)"))));
	}

	for (mk_gen_intdir_type_t i = 0; i < __MK_INTDIR_MAX; i++) {
		if (obj[i][MK_EXT_ASM] != MAKE_END) {
			const make_rule_t int_o = make_add_act(&make, make_create_rule(&make, MRULE(MSTR(strf("$(%s)%%.o", intdir_c[i].name.data))), 1));
			make_rule_add_depend(&make, int_o, MRULE(MSTR(STR("%.asm"))));
			make_rule_add_act(&make, int_o, make_create_cmd(&make, MCMD(STR("@mkdir -p $(@D)"))));
			make_rule_add_act(
				&make, int_o,
				make_create_cmd(&make, MCMD(strf("@nasm $(CONFIG_FLAGS) $(ASFLAGS) $(%s)%s -c -o $@ $<", intdir_c[i].defines.data, intdir_c[i].flags))));
		}
	}

	for (mk_gen_intdir_type_t i = 0; i < __MK_INTDIR_MAX; i++) {
		if (obj[i][MK_EXT_C] != MAKE_END) {
			const make_rule_t int_o = make_add_act(&make, make_create_rule(&make, MRULE(MSTR(strf("$(%s)%%.o", intdir_c[i].name.data))), 1));
			make_rule_add_depend(&make, int_o, MRULE(MSTR(STR("%.c"))));
			make_rule_add_act(&make, int_o, make_create_cmd(&make, MCMD(STR("@mkdir -p $(@D)"))));
			make_rule_add_act(
				&make, int_o,
				make_create_cmd(&make, MCMD(strf("@$(TCC) $(CONFIG_FLAGS) $(CFLAGS) $(%s)%s -c -o $@ $<", intdir_c[i].defines.data, intdir_c[i].flags))));
		}
	}

	for (mk_gen_intdir_type_t i = 0; i < __MK_INTDIR_MAX; i++) {
		if (obj[i][MK_EXT_CPP] != MAKE_END) {
			const make_rule_t int_o = make_add_act(&make, make_create_rule(&make, MRULE(MSTR(strf("$(%s)%%.o", intdir_c[i].name.data))), 1));
			make_rule_add_depend(&make, int_o, MRULE(MSTR(STR("%.cpp"))));
			make_rule_add_act(&make, int_o, make_create_cmd(&make, MCMD(STR("@mkdir -p $(@D)"))));
			make_rule_add_act(&make, int_o,
					  make_create_cmd(&make, MCMD(strf("@$(TCC) $(CONFIG_FLAGS) $(CXXFLAGS) $(%s)%s -c -o $@ $<", intdir_c[i].defines.data,
									   intdir_c[i].flags))));
		}
	}

	// clang-format off
	make_var_t run[] = {
		[MK_TARGET_EXE]	   = MAKE_END,
		[MK_TARGET_STATIC] = MAKE_END,
		[MK_TARGET_SHARED] = MAKE_END,
		[MK_TARGET_BIN]	   = MAKE_END,
		[MK_TARGET_ELF]	   = MAKE_END,
		[MK_TARGET_FAT12]  = MAKE_END,
	};
	// clang-format on

	for (mk_gen_target_type_t t = 0; t < __MK_TARGET_MAX; t++) {
		if (target[t] == MAKE_END) {
			continue;
		}

		if (gen->run_debug[t].data || gen->run[t].data || t == MK_TARGET_EXE) {
			run[t] = make_add_act(&make, make_create_rule(&make, MRULE(MSTR(STR("run"))), 0));
			make_rule_add_depend(&make, run[t], MRULE(MSTR(STR("check"))));
			make_rule_add_depend(&make, run[t], MRULE(MVAR(target[t])));
		}

		if (gen->run_debug[t].data) {
			const make_if_t if_config = make_rule_add_act(&make, run[t], make_create_if(&make, MVAR(config), MSTR(STR("Debug"))));
			make_if_add_true_act(&make, if_config, make_create_cmd(&make, MCMD(strs(gen->run_debug[t]))));
			if (gen->run[t].data) {
				make_if_add_false_act(&make, if_config, make_create_cmd(&make, MCMD(strs(gen->run[t]))));
			} else if (t == MK_TARGET_EXE) {
				make_if_add_false_act(&make, if_config, make_create_cmd(&make, MCMD(STR("@$(TARGET) $(ARGS)"))));
			}
		} else if (gen->run[t].data) {
			make_rule_add_act(&make, run[t], make_create_cmd(&make, MCMD(strs(gen->run[t]))));
		} else if (t == MK_TARGET_EXE) {
			make_rule_add_act(&make, run[t], make_create_cmd(&make, MCMD(STR("@$(TARGET) $(ARGS)"))));
		}

		if (run[t] == MAKE_END || repdir == MAKE_END || lcov == MAKE_END) {
			continue;
		}

		const make_if_t if_coverage = make_rule_add_act(&make, run[t], make_create_if(&make, MVAR(coverage), MSTR(STR("true"))));
		make_if_add_true_act(&make, if_coverage, make_create_cmd(&make, MCMD(STR("@lcov -q -c -d $(SLNDIR) -o $(LCOV)"))));
		const make_if_t if_show = make_if_add_true_act(&make, if_coverage, make_create_if(&make, MVAR(show), MSTR(STR("true"))));
		make_if_add_true_act(&make, if_show, make_create_cmd(&make, MCMD(STR("@genhtml -q $(LCOV) -o $(REPDIR)"))));
		make_if_add_true_act(&make, if_show, make_create_cmd(&make, MCMD(STR("@open $(REPDIR)index.html"))));
	}

	// clang-format off
	make_var_t artifact[] = {
		[MK_TARGET_EXE]	   = MAKE_END,
		[MK_TARGET_STATIC] = MAKE_END,
		[MK_TARGET_SHARED] = MAKE_END,
		[MK_TARGET_BIN]	   = MAKE_END,
		[MK_TARGET_ELF]	   = MAKE_END,
		[MK_TARGET_FAT12]  = MAKE_END,
	};

	// clang-format on
	for (mk_gen_target_type_t t = 0; t < __MK_TARGET_MAX; t++) {
		if (target[t] == MAKE_END || gen->artifact[t].data == NULL) {
			continue;
		}

		artifact[t] = make_add_act(&make, make_create_rule(&make, MRULE(MSTR(target_c[t].artifact)), 0));
		make_rule_add_depend(&make, artifact[t], MRULE(MSTR(STR("check"))));
		make_rule_add_depend(&make, artifact[t], MRULE(MVAR(target[t])));
		make_rule_add_act(&make, artifact[t], make_create_cmd(&make, MCMD(STR("@mkdir -p $(SLNDIR)tmp/artifact/"))));
		make_rule_add_act(&make, artifact[t],
				  make_create_cmd(&make, MCMD(strf("@cp $(TARGET) $(SLNDIR)tmp/artifact/%.*s", gen->artifact[t].len, gen->artifact[t].data))));
	}

	const make_rule_t clean = make_add_act(&make, make_create_rule(&make, MRULE(MSTR(STR("clean"))), 0));

	str_t cleans = strz(128);
	str_cat(&cleans, STR("@$(RM)"));

	for (mk_gen_target_type_t t = 0; t < __MK_TARGET_MAX; t++) {
		if (target[t] == MAKE_END) {
			continue;
		}
		str_cat(&cleans, STR(" $("));
		str_cat(&cleans, target_c[t].name);
		str_cat(&cleans, STR(")"));
	}

	for (mk_gen_target_type_t t = 0; t < __MK_TARGET_MAX; t++) {
		if (artifact[t] == MAKE_END) {
			continue;
		}
		str_cat(&cleans, STR(" $(SLNDIR)tmp/artifact/"));
		str_cat(&cleans, gen->artifact[t]);
	}

	for (mk_gen_intdir_type_t i = 0; i < __MK_INTDIR_MAX; i++) {
		for (mk_gen_src_ext_t s = 0; s < __MK_SRC_MAX; s++) {
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

	make_rule_add_act(&make, clean, make_create_cmd(&make, MCMD(cleans)));

	int off = make_print(&make, dst);

	make_free(&make);

	return off;
}
