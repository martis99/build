#include "gen/mk/pgc_gen_mk.h"

#include "gen/pgc_common.h"

make_t *pgc_gen_mk_local(const pgc_t *pgc, make_t *make)
{
	const make_var_t arch	  = make_create_var_ext(make, STR("ARCH"), MAKE_VAR_INST);
	const make_var_t config	  = make_create_var_ext(make, STR("CONFIG"), MAKE_VAR_INST);
	const make_var_t coverage = make_create_var_ext(make, STR("COVERAGE"), MAKE_VAR_INST);
	const make_var_t show	  = make_create_var_ext(make, STR("SHOW"), MAKE_VAR_INST);

	int is_vars = 0;

	static const char *header_ext[] = {
		[PGC_HEADER_INC] = "inc",
		[PGC_HEADER_H]	 = "h",
		[PGC_HEADER_HPP] = "hpp",
	};

	make_var_t headers = MAKE_END;
	const pgc_str_flags_t *header;
	arr_foreach(&pgc->arr[PGC_ARR_HEADERS], header)
	{
		for (pgc_header_type_t ext = 0; ext < __PGC_HEADER_TYPE_MAX; ext++) {
			if ((header->flags & (1 << ext)) == 0) {
				continue;
			}

			is_vars = 1;

			headers = make_add_act(make, make_create_var(make, STR("HEADERS"), headers == MAKE_END ? MAKE_VAR_INST : MAKE_VAR_APP));
			make_var_add_val(make, headers, MSTR(strf("$(shell find %.*s -name '*.%s')", header->str.len, header->str.data, header_ext[ext])));
		}
	}

	// clang-format off
	static struct {
		str_t name;
		str_t flags;
		const char *ext;
		int cov;
	} src_c[] = {
		[PGC_SRC_NASM] = { STRS("SRC_ASM"), STRS("ASFLAGS"),  "nasm", 0 },
		[PGC_SRC_S]   = { STRS("SRC_S"),   STRS("ASFLAGS"),  "S",    0 },
		[PGC_SRC_C]   = { STRS("SRC_C"),   STRS("CFLAGS"),   "c",    1 },
		[PGC_SRC_CPP] = { STRS("SRC_CPP"), STRS("CXXFLAGS"), "cpp",  1 },
	};
	// clang-format on

	make_var_t src[] = {
		[PGC_SRC_NASM] = MAKE_END,
		[PGC_SRC_S]    = MAKE_END,
		[PGC_SRC_C]    = MAKE_END,
		[PGC_SRC_CPP]  = MAKE_END,
	};

	int is_src = 0;

	for (pgc_src_type_t s = 0; s < __PGC_SRC_TYPE_MAX; s++) {
		const pgc_str_flags_t *data;
		arr_foreach(&pgc->arr[PGC_ARR_SRCS], data)
		{
			if ((data->flags & (1 << s)) == 0) {
				continue;
			}

			is_vars = 1;
			is_src	= 1;

			src[s] = make_add_act(make, make_create_var(make, src_c[s].name, src[s] == MAKE_END ? MAKE_VAR_INST : MAKE_VAR_APP));
			make_var_add_val(make, src[s], MSTR(strf("$(shell find %.*s -name '*.%s')", data->str.len, data->str.data, src_c[s].ext)));
		}
	}

	make_var_t intdir[] = {
		[PGC_INTDIR_OBJECT] = MAKE_END,
		[PGC_INTDIR_STATIC] = MAKE_END,
		[PGC_INTDIR_SHARED] = MAKE_END,
	};

	static struct {
		str_t name;
		str_t defines;
		const char *flags;
	} intdir_c[] = {
		[PGC_INTDIR_OBJECT] = { STRS("INTDIR"), STRS("DEFINES"), " -c" },
		[PGC_INTDIR_STATIC] = { STRS("INTDIR_S"), STRS("DEFINES_S"), " -c" },
		[PGC_INTDIR_SHARED] = { STRS("INTDIR_D"), STRS("DEFINES_D"), " -c -fPIC" },
	};

	for (pgc_intdir_type_t i = 0; i < __PGC_INTDIR_TYPE_MAX; i++) {
		if (pgc->intdir[PGC_INTDIR_STR_INTDIR][i].data == NULL || !is_src) {
			continue;
		}

		is_vars = 1;

		intdir[i] = make_add_act(make, make_create_var(make, intdir_c[i].name, MAKE_VAR_INST));
		make_var_add_val(make, intdir[i], MSTR(str_cpy(pgc->intdir[PGC_INTDIR_STR_INTDIR][i])));
	}

	make_var_t obj[][__PGC_SRC_TYPE_MAX] = {
		[PGC_INTDIR_OBJECT] = {
			[PGC_SRC_NASM] = MAKE_END,
			[PGC_SRC_S]   = MAKE_END,
			[PGC_SRC_C]   = MAKE_END,
			[PGC_SRC_CPP] = MAKE_END,
		},
		[PGC_INTDIR_STATIC] = {
			[PGC_SRC_NASM] = MAKE_END,
			[PGC_SRC_S]   = MAKE_END,
			[PGC_SRC_C]   = MAKE_END,
			[PGC_SRC_CPP] = MAKE_END,
		},
		[PGC_INTDIR_SHARED] = {
			[PGC_SRC_NASM] = MAKE_END,
			[PGC_SRC_S]   = MAKE_END,
			[PGC_SRC_C]   = MAKE_END,
			[PGC_SRC_CPP] = MAKE_END,
		},
	};

	int obj_ext[__PGC_SRC_TYPE_MAX]	      = { 0 };
	int obj_intdir[__PGC_INTDIR_TYPE_MAX] = { 0 };

	static struct {
		str_t name;
	} obj_ext_c[][__PGC_SRC_TYPE_MAX] = {
		[PGC_INTDIR_OBJECT] = {
			[PGC_SRC_NASM] = { STRS("OBJ_ASM") },
			[PGC_SRC_S]   = { STRS("OBJ_S")   },
			[PGC_SRC_C]   = { STRS("OBJ_C")   },
			[PGC_SRC_CPP] = { STRS("OBJ_CPP") },
		},
		[PGC_INTDIR_STATIC] = {
			[PGC_SRC_NASM] = { STRS("OBJ_ASM_S") },
			[PGC_SRC_S]   = { STRS("OBJ_S_S")   },
			[PGC_SRC_C]   = { STRS("OBJ_C_S")   },
			[PGC_SRC_CPP] = { STRS("OBJ_CPP_S") },
		},
		[PGC_INTDIR_SHARED] = {
			[PGC_SRC_NASM] = { STRS("OBJ_ASM_D") },
			[PGC_SRC_S]   = { STRS("OBJ_S_D")   },
			[PGC_SRC_C]   = { STRS("OBJ_C_D")   },
			[PGC_SRC_CPP] = { STRS("OBJ_CPP_D") },
		}, 
	};

	int is_obj = 0;

	for (pgc_intdir_type_t i = 0; i < __PGC_INTDIR_TYPE_MAX; i++) {
		if (intdir[i] == MAKE_END) {
			continue;
		}

		for (pgc_src_type_t s = 0; s < __PGC_SRC_TYPE_MAX; s++) {
			if (!obj_ext_c[i][s].name.data || src[s] == MAKE_END) {
				continue;
			}

			is_vars = 1;
			is_obj	= 1;

			obj_intdir[i] = 1;
			obj_ext[s]    = 1;

			obj[i][s] = make_add_act(make, make_create_var(make, obj_ext_c[i][s].name, MAKE_VAR_INST));
			make_var_add_val(make, obj[i][s],
					 MSTR(strf("$(patsubst %%.%s, $(%s)%%.%s, $(%s))", src_c[s].ext, intdir_c[i].name.data,
						   s == PGC_SRC_NASM && (pgc->builds & (1 << PGC_BUILD_BIN)) ? "bin" : "o", src_c[s].name.data)));
		}
	}

	make_var_t outdir = MAKE_END;
	if (pgc->str[PGC_STR_OUTDIR].data && (is_obj || pgc->arr[PGC_ARR_FILES].cnt > 0)) {
		is_vars = 1;

		outdir = make_add_act(make, make_create_var(make, STR("OUTDIR"), MAKE_VAR_INST));
		make_var_add_val(make, outdir, MSTR(str_cpy(pgc->str[PGC_STR_OUTDIR])));
	}

	make_var_t covdir = MAKE_END;
	if (pgc->str[PGC_STR_COVDIR].data && outdir != MAKE_END) {
		is_vars = 1;

		covdir = make_add_act(make, make_create_var(make, STR("COVDIR"), MAKE_VAR_INST));
		make_var_add_val(make, covdir, MSTR(str_cpy(pgc->str[PGC_STR_COVDIR])));
	}

	make_var_t cov = MAKE_END;
	for (pgc_src_type_t s = 0; s < __PGC_SRC_TYPE_MAX; s++) {
		if (covdir == MAKE_END || src[s] == MAKE_END || src_c[s].cov == 0) {
			continue;
		}

		is_vars = 1;

		cov = make_add_act(make, make_create_var(make, STR("COV"), cov == MAKE_END ? MAKE_VAR_INST : MAKE_VAR_APP));
		make_var_add_val(make, cov, MSTR(strf("$(patsubst %%.%s, $(COVDIR)%%.gcno, $(%s))", src_c[s].ext, src_c[s].name.data)));
		cov = make_add_act(make, make_create_var(make, STR("COV"), MAKE_VAR_APP));
		make_var_add_val(make, cov, MSTR(strf("$(patsubst %%.%s, $(COVDIR)%%.gcda, $(%s))", src_c[s].ext, src_c[s].name.data)));
	}

	// clang-format off
	make_var_t target[] = {
		[PGC_BUILD_EXE]	  = MAKE_END,
		[PGC_BUILD_STATIC] = MAKE_END,
		[PGC_BUILD_SHARED] = MAKE_END,
		[PGC_BUILD_ELF]	  = MAKE_END,
		[PGC_BUILD_BIN]	  = MAKE_END,
		[PGC_BUILD_FAT12]  = MAKE_END,
	};

	static struct {
		str_t name;
		const char *ext;
		str_t act;
		pgc_intdir_type_t intdir;
		str_t run;
		str_t artifact;
		str_t cmdl;
		str_t cmdr;
		int bits;
	} target_c[] = {
		[PGC_BUILD_EXE]	   = { STRS("TARGET"),	     "",     STRS("compile"), PGC_INTDIR_OBJECT, STRS("run"),		 STRS("artifact"),	 STRS("@$(TCC) -m$(BITS)"),			   STRS(" $(LDFLAGS) -o $@"), 1 },
		[PGC_BUILD_STATIC] = { STRS("TARGET_S"),     ".a",   STRS("static"),  PGC_INTDIR_STATIC, STRS("run_s"),		 STRS("artifact_s"),	 STRS("@ar rcs $@"),				   STRS(""),		      0 },
		[PGC_BUILD_SHARED] = { STRS("TARGET_D"),     ".so",  STRS("shared"),  PGC_INTDIR_SHARED, STRS("run_d"),		 STRS("artifact_d"),	 STRS("@$(TCC) -m$(BITS) -shared"),		   STRS(" $(LDFLAGS) -o $@"), 1 },
		[PGC_BUILD_ELF]	   = { STRS("TARGET_ELF"),   ".elf", STRS("elf"),     PGC_INTDIR_OBJECT, STRS("run_elf"),	 STRS("artifact_elf"),	 STRS("@$(TCC) -m$(BITS) -shared -ffreestanding"), STRS(" $(LDFLAGS) -o $@"), 1 }, 
		[PGC_BUILD_BIN]	   = { STRS("TARGET_BIN"),   ".bin", STRS("bin"),     PGC_INTDIR_OBJECT, STRS("run_bin"),	 STRS("artifact_bin"),	 STRS(""),					   STRS(""),		      0 }, 
		[PGC_BUILD_FAT12]  = { STRS("TARGET_FAT12"), ".img", STRS("fat12"),   __PGC_INTDIR_TYPE_MAX,  STRS("run_fat12"), STRS("artifact_fat12"), STRS(""),					   STRS(""),		      0 },
	};
	// clang-format on

	for (pgc_build_type_t b = 0; b < __PGC_BUILD_TYPE_MAX; b++) {
		if (pgc->target[PGC_TARGET_STR_TARGET]->data == NULL) {
			continue;
		}

		is_vars = 1;

		target[b] = make_add_act(make, make_create_var(make, target_c[b].name, MAKE_VAR_INST));
		make_var_add_val(make, target[b], MSTR(str_cpy(pgc->target[PGC_TARGET_STR_TARGET][b])));
	}

	make_var_t args = MAKE_END;
	if (target[PGC_BUILD_EXE] != MAKE_END) {
		args = make_add_act(make, make_create_var(make, STR("ARGS"), MAKE_VAR_INST));
		if (pgc->str[PGC_STR_ARGS].len > 0) {
			make_var_add_val(make, args, MSTR(str_cpy(pgc->str[PGC_STR_ARGS])));
		}
	}

	make_var_t repdir = MAKE_END;
	make_var_t lcov	  = MAKE_END;
	if (covdir != MAKE_END && target[PGC_BUILD_EXE] != MAKE_END) {
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

	int asm_bin = target[PGC_BUILD_BIN] != MAKE_END && obj_ext[PGC_SRC_NASM] && !obj_ext[PGC_SRC_S] && !obj_ext[PGC_SRC_C] && !obj_ext[PGC_SRC_CPP];

	int is_flags = 0;

	make_var_t includes = MAKE_END;
	if (is_obj && pgc->arr[PGC_ARR_INCLUDES].cnt > 0) {
		is_flags = 1;

		includes = make_add_act(make, make_create_var(make, STR("INCLUDES"), MAKE_VAR_INST));

		int first = 1;
		const str_t *include;
		arr_foreach(&pgc->arr[PGC_ARR_INCLUDES], include)
		{
			make_var_add_val(make, includes, MSTR(strf(first ? "-I%.*s" : "-I%.*s", include->len, include->data)));
			first = 0;
		}
	}

	make_var_t flags[] = {
		[PGC_SRC_NASM] = MAKE_END,
		[PGC_SRC_S]    = MAKE_END,
		[PGC_SRC_C]    = MAKE_END,
		[PGC_SRC_CPP]  = MAKE_END,
	};

	for (pgc_src_type_t s = 0; s < __PGC_SRC_TYPE_MAX; s++) {
		if (!obj_ext[s] || pgc->src[PGC_SRC_STR_FLAGS][s].len == 0) {
			continue;
		}

		is_flags = 1;

		flags[s] = make_add_act(make, make_create_var(make, src_c[s].flags, MAKE_VAR_INST));
		make_var_add_val(make, flags[s], MSTR(str_cpy(pgc->src[PGC_SRC_STR_FLAGS][s])));
	}

	make_var_t defines[] = {
		[PGC_INTDIR_OBJECT] = MAKE_END,
		[PGC_INTDIR_STATIC] = MAKE_END,
		[PGC_INTDIR_SHARED] = MAKE_END,
	};

	for (pgc_intdir_type_t i = 0; i < __PGC_INTDIR_TYPE_MAX; i++) {
		if (intdir[i] == MAKE_END || pgc->intdir[PGC_INTDIR_STR_DEFINES][i].len == 0) {
			continue;
		}

		is_flags = 1;

		defines[i] = make_add_act(make, make_create_var(make, intdir_c[i].defines, MAKE_VAR_INST));
		make_var_add_val(make, defines[i], MSTR(str_cpy(pgc->intdir[PGC_INTDIR_STR_DEFINES][i])));
	}

	if (is_obj) {
		is_flags = 1;

		const make_var_t ldflags = make_add_act(make, make_create_var(make, STR("LDFLAGS"), MAKE_VAR_INST));

		int first = 1;

		const pgc_lib_data_t *lib;
		arr_foreach(&pgc->arr[PGC_ARR_LIBS], lib)
		{
			if (lib->dir.data) {
				make_var_add_val(make, ldflags, MSTR(strf("-L%.*s", lib->dir.len, lib->dir.data)));
			}

			if (lib->name.data) {
				if (lib->link_type == PGC_LINK_SHARED) {
					if (first) {
						make_var_add_val(make, ldflags, MSTR(strf("-Wl,-rpath,.")));
						first = 0;
					}
					make_var_add_val(make, ldflags, MSTR(strf("-l:%.*s.so", lib->name.len, lib->name.data)));
				} else {
					make_var_add_val(make, ldflags, MSTR(strf("-l:%.*s.a", lib->name.len, lib->name.data)));
				}
			}
		}

		if (pgc->str[PGC_STR_LDFLAGS].len > 0) {
			make_var_add_val(make, ldflags, MSTR(str_cpy(pgc->str[PGC_STR_LDFLAGS])));
		}
	}

	make_var_t nasm_config_flags = MAKE_END;
	if (obj_ext[PGC_SRC_NASM]) {
		is_flags = 1;

		nasm_config_flags = make_add_act(make, make_create_var(make, STR("NASM_CONFIG_FLAGS"), MAKE_VAR_INST));
	}

	make_var_t gcc_config_flags = MAKE_END;
	if (obj_ext[PGC_SRC_S] || obj_ext[PGC_SRC_C] || obj_ext[PGC_SRC_CPP]) {
		is_flags = 1;

		gcc_config_flags = make_add_act(make, make_create_var(make, STR("GCC_CONFIG_FLAGS"), MAKE_VAR_INST));
	}

	if (is_flags) {
		make_add_act(make, make_create_empty(make));
	}

	const make_var_t rm = make_add_act(make, make_create_var(make, STR("RM"), MAKE_VAR_APP));
	make_var_add_val(make, rm, MSTR(STR("-r")));

	make_add_act(make, make_create_empty(make));

	make_var_t bits = MAKE_END;
	if (is_obj) {
		if (pgc_get_arch(pgc, STR("x86_64")) != PGC_END) {
			const make_if_t if_x86_64   = make_add_act(make, make_create_if(make, MVAR(arch), MSTR(STR("x86_64"))));
			bits			    = make_create_var(make, STR("BITS"), MAKE_VAR_INST);
			const make_var_t bit_x86_64 = make_if_add_true_act(make, if_x86_64, bits);
			make_var_add_val(make, bit_x86_64, MSTR(STR("64")));
		}

		if (pgc_get_arch(pgc, STR("i386")) != PGC_END) {
			const make_if_t if_i386	   = make_add_act(make, make_create_if(make, MVAR(arch), MSTR(STR("i386"))));
			bits			   = make_create_var(make, STR("BITS"), MAKE_VAR_INST);
			const make_var_t bits_i386 = make_if_add_true_act(make, if_i386, bits);
			make_var_add_val(make, bits_i386, MSTR(STR("32")));
		}

		if (bits != MAKE_END) {
			make_add_act(make, make_create_empty(make));
		}
	}

	int is_debug = (nasm_config_flags != MAKE_END || gcc_config_flags != MAKE_END) && pgc_get_config(pgc, STR("Debug")) != PGC_END;

	if (is_debug) {
		const make_if_t if_config = make_add_act(make, make_create_if(make, MVAR(config), MSTR(STR("Debug"))));
		if (nasm_config_flags != MAKE_END) {
			const make_var_t config_flags_true = make_if_add_true_act(make, if_config, make_create_var(make, STR("NASM_CONFIG_FLAGS"), MAKE_VAR_APP));
			if (asm_bin) {
				make_var_add_val(make, config_flags_true, MSTR(STR("-g")));
			} else {
				make_var_add_val(make, config_flags_true, MSTR(STR("-g -F dwarf")));
			}
		}
		if (gcc_config_flags != MAKE_END) {
			const make_var_t config_flags_true = make_if_add_true_act(make, if_config, make_create_var(make, STR("GCC_CONFIG_FLAGS"), MAKE_VAR_APP));
			make_var_add_val(make, config_flags_true, MSTR(STR("-ggdb3 -O0")));
		}
		make_add_act(make, make_create_empty(make));
	}

	if (cov != MAKE_END && gcc_config_flags != MAKE_END) {
		const make_if_t if_cov		= make_add_act(make, make_create_if(make, MVAR(coverage), MSTR(STR("true"))));
		const make_var_t cov_flags_true = make_if_add_true_act(make, if_cov, make_create_var(make, STR("GCC_CONFIG_FLAGS"), MAKE_VAR_APP));
		make_var_add_val(make, cov_flags_true, MSTR(STR("--coverage -fprofile-abs-path")));
		make_add_act(make, make_create_empty(make));
	}

	const make_rule_t all	= make_add_act(make, make_create_rule(make, MRULE(MSTR(STR("all"))), 0));
	const make_rule_t check = make_add_act(make, make_create_rule(make, MRULE(MSTR(STR("check"))), 0));

	if (obj_ext[PGC_SRC_NASM]) {
		const make_if_t if_nasm = make_rule_add_act(make, check, make_create_if(make, MSTR(str_null()), MSTR(STR("$(shell which nasm)"))));
		make_if_add_true_act(make, if_nasm, make_create_cmd(make, MCMD(STR("sudo apt-get install nasm -y"))));
	}

	if (obj_ext[PGC_SRC_S] || obj_ext[PGC_SRC_C]) {
		const make_if_t if_gcc = make_rule_add_act(make, check, make_create_if(make, MSTR(str_null()), MSTR(STR("$(shell which gcc)"))));
		make_if_add_true_act(make, if_gcc, make_create_cmd(make, MCMD(STR("sudo apt-get install gcc -y"))));

		const make_if_t if_arch = make_rule_add_act(make, check, make_create_if(make, MVAR(arch), MSTR(STR("$(shell uname -m)"))));

		make_if_t if_dpkg = make_if_add_false_act(
			make, if_arch, make_create_if(make, MSTR(str_null()), MSTR(STR("$(shell apt list --installed 2>/dev/null | grep gcc-multilib/)"))));
		make_if_add_true_act(make, if_dpkg, make_create_cmd(make, MCMD(STR("sudo apt-get install gcc-multilib -y"))));
	}

	if (obj_ext[PGC_SRC_CPP]) {
		const make_if_t if_gcc = make_rule_add_act(make, check, make_create_if(make, MSTR(str_null()), MSTR(STR("$(shell which g++)"))));
		make_if_add_true_act(make, if_gcc, make_create_cmd(make, MCMD(STR("sudo apt-get install g++ -y"))));

		const make_if_t if_arch = make_rule_add_act(make, check, make_create_if(make, MVAR(arch), MSTR(STR("$(shell uname -m)"))));

		make_if_t if_dpkg = make_if_add_false_act(
			make, if_arch, make_create_if(make, MSTR(str_null()), MSTR(STR("$(shell apt list --installed 2>/dev/null | grep g++-multilib/)"))));
		make_if_add_true_act(make, if_dpkg, make_create_cmd(make, MCMD(STR("sudo apt-get install g++-multilib -y"))));
	}

	if (target[PGC_BUILD_FAT12] != MAKE_END) {
		const make_if_t if_mcopy = make_rule_add_act(make, check, make_create_if(make, MSTR(str_null()), MSTR(STR("$(shell which mcopy)"))));
		make_if_add_true_act(make, if_mcopy, make_create_cmd(make, MCMD(STR("sudo apt-get install mtools -y"))));
	}

	if (lcov != MAKE_END) {
		const make_if_t if_coverage = make_rule_add_act(make, check, make_create_if(make, MVAR(coverage), MSTR(STR("true"))));
		const make_if_t if_lcov	    = make_if_add_true_act(make, if_coverage, make_create_if(make, MSTR(str_null()), MSTR(STR("$(shell which lcov)"))));
		make_if_add_true_act(make, if_lcov, make_create_cmd(make, MCMD(STR("sudo apt-get install lcov -y"))));
	}

	const str_t *require;
	arr_foreach(&pgc->arr[PGC_ARR_REQUIRES], require)
	{
		make_if_t if_dpkg = make_rule_add_act(make, check,
						      make_create_if(make, MSTR(str_null()),
								     MSTR(strf("$(shell apt list --installed 2>/dev/null | grep %.*s/)", require->len, require->data))));
		make_if_add_true_act(make, if_dpkg, make_create_cmd(make, MCMD(strf("sudo apt-get install %.*s -y", require->len, require->data))));
	}

	make_rule_t copyfiles = MAKE_END;
	if (pgc->arr[PGC_ARR_COPYFILES].cnt > 0) {
		copyfiles = make_add_act(make, make_create_rule(make, MRULE(MSTR(STR("copyfiles"))), 0));

		const str_t *copyfile;
		arr_foreach(&pgc->arr[PGC_ARR_COPYFILES], copyfile)
		{
			make_rule_add_act(make, copyfiles, make_create_cmd(make, MCMD(strf("@cp %.*s .", copyfile->len, copyfile->data))));
		}
	}

	for (pgc_build_type_t b = 0; b < __PGC_BUILD_TYPE_MAX; b++) {
		if ((target[b] == MAKE_END && copyfiles == MAKE_END) || ((pgc->builds & (1 << b)) == 0)) {
			continue;
		}

		make_rule_add_depend(make, all, MRULE(MSTR(target_c[b].act)));

		const make_rule_t compile = make_add_act(make, make_create_rule(make, MRULE(MSTR(target_c[b].act)), 0));
		make_rule_add_depend(make, compile, MRULE(MSTR(STR("check"))));
		if (copyfiles != MAKE_END) {
			make_rule_add_depend(make, compile, MRULE(MSTR(STR("copyfiles"))));
		}

		if (target[b] != MAKE_END) {
			make_rule_add_depend(make, compile, MRULE(MVAR(target[b])));
		}
	}

	// clang-format off
	make_rule_t target_rule[] = {
		[PGC_BUILD_EXE]	  = MAKE_END,
		[PGC_BUILD_STATIC] = MAKE_END,
		[PGC_BUILD_SHARED] = MAKE_END,
		[PGC_BUILD_ELF]	  = MAKE_END,
		[PGC_BUILD_BIN]	  = MAKE_END,
		[PGC_BUILD_FAT12]  = MAKE_END,
	};
	// clang-format on

	for (pgc_build_type_t b = PGC_BUILD_EXE; b <= PGC_BUILD_ELF; b++) {
		if (target[b] == MAKE_END || (target_c[b].bits && bits == MAKE_END)) {
			continue;
		}

		if (obj_intdir[target_c[b].intdir]) {
			target_rule[b] = make_add_act(make, make_create_rule(make, MRULE(MVAR(target[b])), 1));
			make_rule_add_act(make, target_rule[b], make_create_cmd(make, MCMD(STR("@mkdir -p $(@D)"))));
		}

		if (obj_intdir[target_c[b].intdir]) {
			str_t cmd = strz(128);
			str_cat(&cmd, target_c[b].cmdl);

			for (pgc_src_type_t s = 0; s < __PGC_SRC_TYPE_MAX; s++) {
				if (obj[target_c[b].intdir][s] == MAKE_END) {
					continue;
				}

				make_rule_add_depend(make, target_rule[b], MRULE(MVAR(obj[target_c[b].intdir][s])));

				str_cat(&cmd, STR(" $("));
				str_cat(&cmd, obj_ext_c[target_c[b].intdir][s].name);
				str_cat(&cmd, STR(")"));
			}

			const pgc_lib_data_t *lib;
			arr_foreach(&pgc->arr[PGC_ARR_LIBS], lib)
			{
				if (lib->name.data == NULL) {
					continue;
				}

				str_t tmp = strz(16);
				if (lib->dir.data) {
					str_cat(&tmp, lib->dir);
				}

				str_cat(&tmp, lib->name);
				str_cat(&tmp, lib->link_type == PGC_LINK_SHARED ? STR(".so") : STR(".a"));

				make_rule_add_depend(make, target_rule[b], MRULE(MSTR(tmp)));
			}

			str_cat(&cmd, target_c[b].cmdr);
			make_rule_add_act(make, target_rule[b], make_create_cmd(make, MCMD(cmd)));
		}
	}

	if (target[PGC_BUILD_BIN] != MAKE_END) {
		target_rule[PGC_BUILD_BIN] = make_add_act(make, make_create_rule(make, MRULE(MVAR(target[PGC_BUILD_BIN])), 1));
		make_rule_add_act(make, target_rule[PGC_BUILD_BIN], make_create_cmd(make, MCMD(STR("@mkdir -p $(@D)"))));

		if (obj_intdir[target_c[PGC_BUILD_BIN].intdir]) {
			str_t cmd = strz(128);
			if (asm_bin) {
				str_cat(&cmd, STR("@cat"));
			} else {
				str_cat(&cmd, STR("@$(TLD) -Tlinker.ld --oformat binary"));
			}

			for (pgc_src_type_t s = 0; s < __PGC_SRC_TYPE_MAX; s++) {
				if (obj[target_c[PGC_BUILD_BIN].intdir][s] == MAKE_END) {
					continue;
				}

				make_rule_add_depend(make, target_rule[PGC_BUILD_BIN], MRULE(MVAR(obj[target_c[PGC_BUILD_BIN].intdir][s])));

				str_cat(&cmd, STR(" $("));
				str_cat(&cmd, obj_ext_c[target_c[PGC_BUILD_BIN].intdir][s].name);
				str_cat(&cmd, STR(")"));
			}

			if (asm_bin) {
				str_cat(&cmd, STR(" > $@"));
			} else {
				str_cat(&cmd, STR(" $(LDFLAGS) -o $@"));
			}

			make_rule_add_act(make, target_rule[PGC_BUILD_BIN], make_create_cmd(make, MCMD(cmd)));
		} else {
			const pgc_str_flags_t *file;
			arr_foreach(&pgc->arr[PGC_ARR_FILES], file)
			{
				make_rule_add_depend(make, target_rule[PGC_BUILD_BIN], MRULE(MSTR(str_cpy(file->str))));
				switch (file->flags) {
				case PGC_FILE_BIN:
					make_rule_add_act(make, target_rule[PGC_BUILD_BIN],
							  make_create_cmd(make, MCMD(strf("@dd if=%.*s status=none >> $@", file->str.len, file->str.data))));
					break;

				case PGC_FILE_ELF:
					make_rule_add_act(make, target_rule[PGC_BUILD_BIN],
							  make_create_cmd(make, MCMD(strf("@objcopy -O binary -j .text %.*s %.*s.bin", file->str.len, file->str.data,
											  file->str.len, file->str.data))));
					make_rule_add_act(make, target_rule[PGC_BUILD_BIN],
							  make_create_cmd(make, MCMD(strf("@dd if=%.*s.bin status=none >> $@", file->str.len, file->str.data))));
					break;
				}
			}

			if (pgc->str[PGC_STR_SIZE].data) {
				make_rule_add_act(make, target_rule[PGC_BUILD_BIN],
						  make_create_cmd(make, MCMD(strf("@dd if=/dev/zero bs=1 count=%.*s status=none >> $@", pgc->str[PGC_STR_SIZE].len,
										  pgc->str[PGC_STR_SIZE].data))));
			}
		}
	}

	if (target[PGC_BUILD_FAT12] != MAKE_END) {
		target_rule[PGC_BUILD_FAT12] = make_add_act(make, make_create_rule(make, MRULE(MVAR(target[PGC_BUILD_FAT12])), 1));

		if (pgc->str[PGC_STR_HEADER].data != NULL) {
			make_rule_add_depend(make, target_rule[PGC_BUILD_FAT12], MRULE(MSTR(str_cpy(pgc->str[PGC_STR_HEADER]))));
		}

		const pgc_str_flags_t *file;
		arr_foreach(&pgc->arr[PGC_ARR_FILES], file)
		{
			make_rule_add_depend(make, target_rule[PGC_BUILD_FAT12], MRULE(MSTR(str_cpy(file->str))));
		}

		make_rule_add_act(make, target_rule[PGC_BUILD_FAT12], make_create_cmd(make, MCMD(STR("@mkdir -p $(@D)"))));

		//create empty 1.44MB image (block size = 512, block count = 2880)
		make_rule_add_act(make, target_rule[PGC_BUILD_FAT12], make_create_cmd(make, MCMD(STR("@dd if=/dev/zero of=$@ bs=512 count=2880 status=none"))));
		if (pgc->str[PGC_STR_HEADER].data == NULL) {
			//create file system
			make_rule_add_act(make, target_rule[PGC_BUILD_FAT12], make_create_cmd(make, MCMD(STR("@mkfs.fat -F12 -n \"NBOS\" $@"))));
		} else {
			//put first binary to the first sector of the disk
			make_rule_add_act(make, target_rule[PGC_BUILD_FAT12],
					  make_create_cmd(make, MCMD(strf("@dd if=%.*s of=$@ conv=notrunc status=none", pgc->str[PGC_STR_HEADER].len,
									  pgc->str[PGC_STR_HEADER].data))));
		}
		//copy files to the image
		make_rule_add_act(make, target_rule[PGC_BUILD_FAT12], make_create_cmd(make, MCMD(STR("@mcopy -i $@ $(word 2,$^) \"::$(shell basename $(word 2,$^))\""))));
	}

	for (pgc_intdir_type_t i = 0; i < __PGC_INTDIR_TYPE_MAX; i++) {
		if (obj[i][PGC_SRC_NASM] == MAKE_END) {
			continue;
		}

		if (asm_bin) {
			const make_rule_t int_o = make_add_act(make, make_create_rule(make, MRULE(MSTR(strf("$(%s)%%.bin", intdir_c[i].name.data))), 1));
			make_rule_add_depend(make, int_o, MRULE(MSTR(STR("%.nasm"))));
			make_rule_add_act(make, int_o, make_create_cmd(make, MCMD(STR("@mkdir -p $(@D)"))));

			make_rule_add_act(
				make, int_o,
				make_create_cmd(make, MCMD(strf("@nasm -fbin $(INCLUDES) $(NASM_CONFIG_FLAGS) $(ASFLAGS) $(%s) $< -o $@", intdir_c[i].defines.data))));
		} else if (bits != MAKE_END) {
			const make_rule_t int_o = make_add_act(make, make_create_rule(make, MRULE(MSTR(strf("$(%s)%%.o", intdir_c[i].name.data))), 1));
			make_rule_add_depend(make, int_o, MRULE(MSTR(STR("%.nasm"))));
			make_rule_add_act(make, int_o, make_create_cmd(make, MCMD(STR("@mkdir -p $(@D)"))));

			make_rule_add_act(make, int_o,
					  make_create_cmd(make, MCMD(strf("@nasm -felf$(BITS) $(INCLUDES) $(NASM_CONFIG_FLAGS) $(ASFLAGS) $(%s) $< -o $@",
									  intdir_c[i].defines.data))));
		}
	}

	for (pgc_intdir_type_t i = 0; i < __PGC_INTDIR_TYPE_MAX; i++) {
		if (obj[i][PGC_SRC_S] == MAKE_END || bits == MAKE_END) {
			continue;
		}

		const make_rule_t int_o = make_add_act(make, make_create_rule(make, MRULE(MSTR(strf("$(%s)%%.o", intdir_c[i].name.data))), 1));
		make_rule_add_depend(make, int_o, MRULE(MSTR(STR("%.S"))));
		make_rule_add_act(make, int_o, make_create_cmd(make, MCMD(STR("@mkdir -p $(@D)"))));
		make_rule_add_act(make, int_o,
				  make_create_cmd(make, MCMD(strf("@$(TCC) -m$(BITS)%s $(INCLUDES) $(GCC_CONFIG_FLAGS) $(ASFLAGS) $(%s) $< -o $@", intdir_c[i].flags,
								  intdir_c[i].defines.data))));
	}

	for (pgc_intdir_type_t i = 0; i < __PGC_INTDIR_TYPE_MAX; i++) {
		if (obj[i][PGC_SRC_C] == MAKE_END || bits == MAKE_END) {
			continue;
		}

		const make_rule_t int_o = make_add_act(make, make_create_rule(make, MRULE(MSTR(strf("$(%s)%%.o", intdir_c[i].name.data))), 1));
		make_rule_add_depend(make, int_o, MRULE(MSTR(STR("%.c"))));
		make_rule_add_act(make, int_o, make_create_cmd(make, MCMD(STR("@mkdir -p $(@D)"))));
		make_rule_add_act(make, int_o,
				  make_create_cmd(make, MCMD(strf("@$(TCC) -m$(BITS)%s $(INCLUDES) $(GCC_CONFIG_FLAGS) $(CFLAGS) $(%s) $< -o $@", intdir_c[i].flags,
								  intdir_c[i].defines.data))));
	}

	for (pgc_intdir_type_t i = 0; i < __PGC_INTDIR_TYPE_MAX; i++) {
		if (obj[i][PGC_SRC_CPP] == MAKE_END || bits == MAKE_END) {
			continue;
		}

		const make_rule_t int_o = make_add_act(make, make_create_rule(make, MRULE(MSTR(strf("$(%s)%%.o", intdir_c[i].name.data))), 1));
		make_rule_add_depend(make, int_o, MRULE(MSTR(STR("%.cpp"))));
		make_rule_add_act(make, int_o, make_create_cmd(make, MCMD(STR("@mkdir -p $(@D)"))));
		make_rule_add_act(make, int_o,
				  make_create_cmd(make, MCMD(strf("@$(TCC) -m$(BITS)%s $(INCLUDES) $(GCC_CONFIG_FLAGS) $(CXXFLAGS) $(%s) $< -o $@", intdir_c[i].flags,
								  intdir_c[i].defines.data))));
	}

	// clang-format off
	make_var_t run[] = {
		[PGC_BUILD_EXE]	  = MAKE_END,
		[PGC_BUILD_STATIC] = MAKE_END,
		[PGC_BUILD_SHARED] = MAKE_END,
		[PGC_BUILD_ELF]	  = MAKE_END,
		[PGC_BUILD_BIN]	  = MAKE_END,
		[PGC_BUILD_FAT12]  = MAKE_END,
	};
	// clang-format on

	for (pgc_build_type_t b = 0; b < __PGC_BUILD_TYPE_MAX; b++) {
		if (target[b] == MAKE_END) {
			continue;
		}

		if ((is_debug && pgc->target[PGC_TARGET_STR_RUN_DBG][b].data) || pgc->target[PGC_TARGET_STR_RUN][b].data || b == PGC_BUILD_EXE) {
			run[b] = make_add_act(make, make_create_rule(make, MRULE(MSTR(target_c[b].run)), 0));
			make_rule_add_depend(make, run[b], MRULE(MSTR(STR("check"))));
			make_rule_add_depend(make, run[b], MRULE(MVAR(target[b])));
		}

		if (is_debug && pgc->target[PGC_TARGET_STR_RUN_DBG][b].data) {
			const make_if_t if_config = make_rule_add_act(make, run[b], make_create_if(make, MVAR(config), MSTR(STR("Debug"))));

			str_t cmd = pgc->str[PGC_STR_CWD].data ? strf("@cd %.*s && ", pgc->str[PGC_STR_CWD].len, pgc->str[PGC_STR_CWD].data) : strz(32);
			str_cat(&cmd, pgc->target[PGC_TARGET_STR_RUN_DBG][b]);
			make_if_add_true_act(make, if_config, make_create_cmd(make, MCMD(cmd)));
			if (pgc->target[PGC_TARGET_STR_RUN][b].data) {
				cmd = pgc->str[PGC_STR_CWD].data ? strf("@cd %.*s && ", pgc->str[PGC_STR_CWD].len, pgc->str[PGC_STR_CWD].data) : strz(32);
				str_cat(&cmd, pgc->target[PGC_TARGET_STR_RUN][b]);
				make_if_add_false_act(make, if_config, make_create_cmd(make, MCMD(cmd)));
			} else if (b == PGC_BUILD_EXE) {
				cmd = pgc->str[PGC_STR_CWD].data ? strf("@cd %.*s && ", pgc->str[PGC_STR_CWD].len, pgc->str[PGC_STR_CWD].data) : strz(32);
				str_cat(&cmd, STR("@$(TARGET) $(ARGS)"));
				make_if_add_false_act(make, if_config, make_create_cmd(make, MCMD(cmd)));
			}
		} else if (pgc->target[PGC_TARGET_STR_RUN][b].data) {
			str_t cmd = pgc->str[PGC_STR_CWD].data ? strf("@cd %.*s && ", pgc->str[PGC_STR_CWD].len, pgc->str[PGC_STR_CWD].data) : strz(32);
			str_cat(&cmd, pgc->target[PGC_TARGET_STR_RUN][b]);
			make_rule_add_act(make, run[b], make_create_cmd(make, MCMD(cmd)));
		} else if (b == PGC_BUILD_EXE) {
			str_t cmd = pgc->str[PGC_STR_CWD].data ? strf("@cd %.*s && ", pgc->str[PGC_STR_CWD].len, pgc->str[PGC_STR_CWD].data) : strz(32);
			str_cat(&cmd, STR("@$(TARGET) $(ARGS)"));
			make_rule_add_act(make, run[b], make_create_cmd(make, MCMD(cmd)));
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

	if (target[PGC_BUILD_EXE] != MAKE_END && is_obj) {
		const make_rule_t rdebug = make_add_act(make, make_create_rule(make, MRULE(MSTR(STR("debug"))), 0));
		make_rule_add_depend(make, rdebug, MRULE(MSTR(STR("check"))));
		make_rule_add_depend(make, rdebug, MRULE(MVAR(target[PGC_BUILD_EXE])));

		if (pgc->target[PGC_TARGET_STR_RUN_DBG][PGC_BUILD_EXE].data) {
			str_t cmd = pgc->str[PGC_STR_CWD].data ? strf("@cd %.*s && ", pgc->str[PGC_STR_CWD].len, pgc->str[PGC_STR_CWD].data) : strz(32);
			str_cat(&cmd, STR("@gdb --args "));
			str_cat(&cmd, pgc->target[PGC_TARGET_STR_RUN_DBG][PGC_BUILD_EXE]);
			make_rule_add_act(make, rdebug, make_create_cmd(make, MCMD(cmd)));
		} else if (pgc->target[PGC_TARGET_STR_RUN][PGC_BUILD_EXE].data) {
			str_t cmd = pgc->str[PGC_STR_CWD].data ? strf("@cd %.*s && ", pgc->str[PGC_STR_CWD].len, pgc->str[PGC_STR_CWD].data) : strz(32);
			str_cat(&cmd, STR("@gdb --args "));
			str_cat(&cmd, pgc->target[PGC_TARGET_STR_RUN][PGC_BUILD_EXE]);
			make_rule_add_act(make, rdebug, make_create_cmd(make, MCMD(cmd)));
		} else {
			str_t cmd = pgc->str[PGC_STR_CWD].data ? strf("@cd %.*s && ", pgc->str[PGC_STR_CWD].len, pgc->str[PGC_STR_CWD].data) : strz(32);
			str_cat(&cmd, STR("@gdb --args $(TARGET) $(ARGS)"));
			make_rule_add_act(make, rdebug, make_create_cmd(make, MCMD(cmd)));
		}
	}

	// clang-format off
	make_act_t artifact[] = {
		[PGC_BUILD_EXE]	  = MAKE_END,
		[PGC_BUILD_STATIC] = MAKE_END,
		[PGC_BUILD_SHARED] = MAKE_END,
		[PGC_BUILD_ELF]	  = MAKE_END,
		[PGC_BUILD_BIN]	  = MAKE_END,
		[PGC_BUILD_FAT12]  = MAKE_END,
	};

	int artifacts = 0;
	// clang-format on
	for (pgc_build_type_t b = 0; b < __PGC_BUILD_TYPE_MAX; b++) {
		if (target[b] == MAKE_END || pgc->target[PGC_TARGET_STR_ARTIFACT][b].data == NULL) {
			continue;
		}

		artifact[b] = make_add_act(make, make_create_rule(make, MRULE(MSTR(target_c[b].artifact)), 0));
		make_rule_add_depend(make, artifact[b], MRULE(MSTR(STR("check"))));
		make_rule_add_depend(make, artifact[b], MRULE(MVAR(target[b])));
		make_rule_add_act(make, artifact[b], make_create_cmd(make, MCMD(STR("@mkdir -p $(SLNDIR)tmp/artifact/"))));
		make_rule_add_act(make, artifact[b],
				  make_create_cmd(make, MCMD(strf("@cp $(%.*s) $(SLNDIR)tmp/artifact/%.*s", target_c[b].name.len, target_c[b].name.data,
								  pgc->target[PGC_TARGET_STR_ARTIFACT][b].len, pgc->target[PGC_TARGET_STR_ARTIFACT][b].data))));

		if (b != PGC_BUILD_EXE) {
			artifacts = 1;
		}
	}

	if (artifacts) {
		const make_rule_t art = make_add_act(make, make_create_rule(make, MRULE(MSTR(STR("artifact"))), 0));

		for (pgc_build_type_t b = 0; b < __PGC_BUILD_TYPE_MAX; b++) {
			if (artifact[b] == MAKE_END) {
				continue;
			}
			make_rule_add_depend(make, art, MRULE(MSTR(str_cpy(target_c[b].artifact))));
		}
	}

	const make_rule_t clean = make_add_act(make, make_create_rule(make, MRULE(MSTR(STR("clean"))), 0));

	str_t cleans = strz(128);
	str_cat(&cleans, STR("@$(RM)"));

	for (pgc_build_type_t b = 0; b < __PGC_BUILD_TYPE_MAX; b++) {
		if (target[b] == MAKE_END) {
			continue;
		}
		str_cat(&cleans, STR(" $("));
		str_cat(&cleans, target_c[b].name);
		str_cat(&cleans, STR(")"));
	}

	for (pgc_build_type_t b = 0; b < __PGC_BUILD_TYPE_MAX; b++) {
		if (artifact[b] == MAKE_END) {
			continue;
		}
		str_cat(&cleans, STR(" $(SLNDIR)tmp/artifact/"));
		str_cat(&cleans, pgc->target[PGC_TARGET_STR_ARTIFACT][b]);
	}

	for (pgc_intdir_type_t i = 0; i < __PGC_INTDIR_TYPE_MAX; i++) {
		for (pgc_src_type_t s = 0; s < __PGC_SRC_TYPE_MAX; s++) {
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

static make_t *pgc_gen_mk_remote(const pgc_t *pgc, make_t *make)
{
	make_var_t outdir = MAKE_END;
	if (pgc->str[PGC_STR_OUTDIR].data) {
		outdir = make_add_act(make, make_create_var(make, STR("OUTDIR"), MAKE_VAR_INST));
		make_var_add_val(make, outdir, MSTR(str_cpy(pgc->str[PGC_STR_OUTDIR])));
	}

	make_var_t url = MAKE_END;
	if (pgc->str[PGC_STR_URL].data) {
		url = make_add_act(make, make_create_var(make, STR("URL"), MAKE_VAR_INST));
		make_var_add_val(make, url, MSTR(str_cpy(pgc->str[PGC_STR_URL])));
	}

	make_var_t name = MAKE_END;
	if (pgc->str[PGC_STR_NAME].data) {
		name = make_add_act(make, make_create_var(make, STR("NAME"), MAKE_VAR_INST));
		make_var_add_val(make, name, MSTR(str_cpy(pgc->str[PGC_STR_NAME])));
	}

	make_var_t format = MAKE_END;
	if (pgc->str[PGC_STR_FORMAT].data) {
		format = make_add_act(make, make_create_var(make, STR("FORMAT"), MAKE_VAR_INST));
		make_var_add_val(make, format, MSTR(str_cpy(pgc->str[PGC_STR_FORMAT])));
	}

	make_var_t file = MAKE_END;
	if (name != MAKE_END && format != MAKE_END) {
		file = make_add_act(make, make_create_var(make, STR("FILE"), MAKE_VAR_INST));
		make_var_add_val(make, file, MSTR(STR("$(NAME).$(FORMAT)")));
	}

	make_var_t dlpath = MAKE_END;
	if (file != MAKE_END) {
		dlpath = make_add_act(make, make_create_var(make, STR("DLPATH"), MAKE_VAR_INST));
		make_var_add_val(make, dlpath, MSTR(STR("$(SLNDIR)tmp/dl/$(FILE)")));
	}

	make_var_t srcdir   = MAKE_END;
	make_var_t biulddir = MAKE_END;
	make_var_t logdir   = MAKE_END;
	if (name != MAKE_END) {
		srcdir = make_add_act(make, make_create_var(make, STR("SRCDIR"), MAKE_VAR_INST));
		make_var_add_val(make, srcdir, MSTR(STR("$(SLNDIR)tmp/src/$(NAME)/")));

		biulddir = make_add_act(make, make_create_var(make, STR("BUILDDIR"), MAKE_VAR_INST));
		make_var_add_val(make, biulddir, MSTR(STR("$(SLNDIR)tmp/bin/$(ARCH)/$(NAME)/")));

		logdir = make_add_act(make, make_create_var(make, STR("LOGDIR"), MAKE_VAR_INST));
		make_var_add_val(make, logdir, MSTR(STR("$(SLNDIR)tmp/logs/$(ARCH)/$(NAME)/")));
	}

	make_add_act(make, make_create_empty(make));

	const make_rule_t all = make_add_act(make, make_create_rule(make, MRULE(MSTR(STR("all"))), 0));
	make_rule_add_depend(make, all, MRULE(MSTR(STR("compile"))));

	const make_rule_t check = make_add_act(make, make_create_rule(make, MRULE(MSTR(STR("check"))), 0));

	if (url != MAKE_END) {
		const make_if_t if_curl = make_rule_add_act(make, check, make_create_if(make, MSTR(str_null()), MSTR(STR("$(shell which curl)"))));
		make_if_add_true_act(make, if_curl, make_create_cmd(make, MCMD(STR("sudo apt-get install curl -y"))));
	}

	const str_t *require;
	arr_foreach(&pgc->arr[PGC_ARR_REQUIRES], require)
	{
		make_if_t if_dpkg = make_rule_add_act(make, check,
						      make_create_if(make, MSTR(str_null()),
								     MSTR(strf("$(shell apt list --installed 2>/dev/null | grep %.*s/)", require->len, require->data))));
		make_if_add_true_act(make, if_dpkg, make_create_cmd(make, MCMD(strf("sudo apt-get install %.*s -y", require->len, require->data))));
	}

	if (dlpath != MAKE_END && url != MAKE_END && file != MAKE_END) {
		const make_rule_t rdlpath = make_add_act(make, make_create_rule(make, MRULE(MVAR(dlpath)), 1));
		make_rule_add_act(make, rdlpath, make_create_cmd(make, MCMD(STR("@mkdir -p $(@D)"))));
		make_rule_add_act(make, rdlpath, make_create_cmd(make, MCMD(STR("@cd $(@D) && curl -s -O $(URL)$(FILE)"))));
	}

	if (dlpath != MAKE_END && srcdir != MAKE_END) {
		const make_rule_t rsrcdir = make_add_act(make, make_create_rule(make, MRULEACT(MVAR(srcdir), STR("done")), 1));
		make_rule_add_depend(make, rsrcdir, MRULE(MVAR(dlpath)));
		make_rule_add_act(make, rsrcdir, make_create_cmd(make, MCMD(STR("@mkdir -p $(SLNDIR)tmp/src"))));
		make_rule_add_act(make, rsrcdir, make_create_cmd(make, MCMD(STR("@tar xf $(DLPATH) -C $(SLNDIR)tmp/src"))));
		make_rule_add_act(make, rsrcdir, make_create_cmd(make, MCMD(STR("@touch $(SRCDIR)done"))));
	}

	if (outdir != MAKE_END && name != MAKE_END && logdir != MAKE_END && biulddir != MAKE_END) {
		const make_rule_t routdir = make_add_act(make, make_create_rule(make, MRULEACT(MSTR(STR("$(OUTDIR)")), STR("$(NAME)")), 1));
		make_rule_add_depend(make, routdir, MRULEACT(MVAR(srcdir), STR("done")));
		make_rule_add_act(make, routdir, make_create_cmd(make, MCMD(STR("@mkdir -p $(LOGDIR) $(BUILDDIR) $(OUTDIR)"))));
		if (pgc->str[PGC_STR_CONFIG].data) {
			make_rule_add_act(
				make, routdir,
				make_create_cmd(
					make,
					MCMD(strf("@cd $(BUILDDIR) && $(SRCDIR)configure --target=$(ARCH)-elf --prefix=$(OUTDIR) %.*s > $(LOGDIR)configure.log 2>&1",
						  pgc->str[PGC_STR_CONFIG].len, pgc->str[PGC_STR_CONFIG].data))));
		}
		if (pgc->str[PGC_STR_TARGETS].data) {
			make_rule_add_act(make, routdir,
					  make_create_cmd(make, MCMD(strf("@cd $(BUILDDIR) && make %.*s > $(LOGDIR)make.log 2>&1", pgc->str[PGC_STR_TARGETS].len,
									  pgc->str[PGC_STR_TARGETS].data))));
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

make_t *pgc_gen_mk(const pgc_t *pgc, make_t *make)
{
	if (pgc == NULL || make == NULL) {
		return NULL;
	}

	make_create_var_ext(make, STR("SLNDIR"), MAKE_VAR_INST);

	if (pgc->str[PGC_STR_URL].data) {
		return pgc_gen_mk_remote(pgc, make);
	}

	return pgc_gen_mk_local(pgc, make);
}
