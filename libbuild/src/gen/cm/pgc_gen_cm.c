#include "gen/cm/pgc_gen_cm.h"

#include "gen/pgc_types.h"

cmake_t *pgc_gen_cm_local(const pgc_t *gen, cmake_t *cmake)
{
	static const char *header_ext[] = {
		[PGC_HEADER_INC] = "inc",
		[PGC_HEADER_H]	 = "h",
		[PGC_HEADER_HPP] = "hpp",
	};

	// clang-format off
	static struct {
		const char *lang;
		const char *ext;
	} src_c[] = {
		[PGC_SRC_NASM] = { "ASM_NASM", "nasm" },
		[PGC_SRC_S]    = { "ASM",      "S" },
		[PGC_SRC_C]    = { "C",        "c" },
		[PGC_SRC_CPP]  = { "CXX",      "cpp" },
	};

	static struct {
		str_t postfix;
		pgc_intdir_type_t intdir;
	} target_c[] = {
		[PGC_BUILD_EXE]	   = { STRS(""),   PGC_INTDIR_OBJECT },
		[PGC_BUILD_STATIC] = { STRS("_s"), PGC_INTDIR_STATIC },
		[PGC_BUILD_SHARED] = { STRS("_d"), PGC_INTDIR_SHARED },
		[PGC_BUILD_ELF]	   = { STRS(""),   PGC_INTDIR_OBJECT },
		[PGC_BUILD_BIN]	   = { STRS(""),   PGC_INTDIR_OBJECT },
		[PGC_BUILD_FAT12]  = { STRS(""),   __PGC_INTDIR_TYPE_MAX  },
	};
	// clang-format on

	char buf_d[P_MAX_PATH] = { 0 };

	str_t buf = strb(buf_d, sizeof(buf_d), 0);

	int src[__PGC_SRC_TYPE_MAX] = { 0 };
	// clang-format off
	uint target[] = { 
		[PGC_BUILD_EXE]	   = CMAKE_END,
		[PGC_BUILD_STATIC] = CMAKE_END,
		[PGC_BUILD_SHARED] = CMAKE_END,
		[PGC_BUILD_ELF]	   = CMAKE_END,
		[PGC_BUILD_BIN]	   = CMAKE_END,
		[PGC_BUILD_FAT12]  = CMAKE_END,
	};
	// clang-format on

	int is_src = 0;

	uint sources = CMAKE_END;

	if (gen->str[PGC_STR_NAME].data && (gen->arr[PGC_ARR_HEADERS].cnt > 0 || gen->arr[PGC_ARR_SRCS].cnt > 0)) {
		sources = cmake_file(cmake, CMAKE_FILE_GLOB_RECURSE, strf("%.*s_SOURCE", gen->str[PGC_STR_NAME].len, gen->str[PGC_STR_NAME].data));

		const pgc_str_flags_t *header;
		arr_foreach(&gen->arr[PGC_ARR_HEADERS], header)
		{
			for (pgc_header_type_t ext = 0; ext < __PGC_HEADER_TYPE_MAX; ext++) {
				if ((header->flags & (1 << ext)) == 0) {
					continue;
				}

				cmake_cmd_add_str(cmake, sources, strf("%.*s*.%s", header->str.len, header->str.data, header_ext[ext]));
			}
		}

		const pgc_str_flags_t *data;
		arr_foreach(&gen->arr[PGC_ARR_SRCS], data)
		{
			for (pgc_src_type_t s = 0; s < __PGC_SRC_TYPE_MAX; s++) {
				if ((data->flags & (1 << s)) == 0) {
					continue;
				}

				is_src = 1;
				src[s] = 1;
				cmake_cmd_add_str(cmake, sources, strf("%.*s*.%s", data->str.len, data->str.data, src_c[s].ext));
			}
		}
	}

	for (pgc_build_type_t b = 0; b < __PGC_BUILD_TYPE_MAX; b++) {
		if (gen->str[PGC_STR_NAME].data == NULL || (gen->builds & (1 << b)) == 0) {
			continue;
		}

		switch (b) {
		case PGC_BUILD_EXE:
			if (is_src) {
				target[b] = cmake_add_exe(cmake,
							  strf("%.*s%.*s", gen->str[PGC_STR_NAME].len, gen->str[PGC_STR_NAME].data, target_c[b].postfix.len,
							       target_c[b].postfix.data),
							  sources);
			}
			break;
		case PGC_BUILD_STATIC:
			if (is_src) {
				target[b] = cmake_add_lib(cmake,
							  strf("%.*s%.*s", gen->str[PGC_STR_NAME].len, gen->str[PGC_STR_NAME].data, target_c[b].postfix.len,
							       target_c[b].postfix.data),
							  CMAKE_LIB_STATIC, sources);
			} else {
				target[b] = cmake_add_custom_target(cmake, strf("%.*s%.*s", gen->str[PGC_STR_NAME].len, gen->str[PGC_STR_NAME].data,
										target_c[b].postfix.len, target_c[b].postfix.data));
			}
			break;
		case PGC_BUILD_SHARED:
			if (is_src) {
				target[b] = cmake_add_lib(cmake,
							  strf("%.*s%.*s", gen->str[PGC_STR_NAME].len, gen->str[PGC_STR_NAME].data, target_c[b].postfix.len,
							       target_c[b].postfix.data),
							  CMAKE_LIB_SHARED, sources);
			} else {
				target[b] = cmake_add_custom_target(cmake, strf("%.*s%.*s", gen->str[PGC_STR_NAME].len, gen->str[PGC_STR_NAME].data,
										target_c[b].postfix.len, target_c[b].postfix.data));
			}
			break;
		case PGC_BUILD_BIN:
			if (is_src) {
				target[b] = cmake_add_exe(cmake,
							  strf("%.*s%.*s", gen->str[PGC_STR_NAME].len, gen->str[PGC_STR_NAME].data, target_c[b].postfix.len,
							       target_c[b].postfix.data),
							  sources);
			} else {
				target[b] = cmake_add_custom_target(cmake, strf("%.*s%.*s", gen->str[PGC_STR_NAME].len, gen->str[PGC_STR_NAME].data,
										target_c[b].postfix.len, target_c[b].postfix.data));
			}
			break;
		case PGC_BUILD_ELF:
			if (is_src) {
				target[b] = cmake_add_lib(cmake,
							  strf("%.*s%.*s", gen->str[PGC_STR_NAME].len, gen->str[PGC_STR_NAME].data, target_c[b].postfix.len,
							       target_c[b].postfix.data),
							  CMAKE_LIB_STATIC, sources);
			}
			break;
		default:
			target[b] = cmake_add_custom_target(cmake, strf("%.*s%.*s", gen->str[PGC_STR_NAME].len, gen->str[PGC_STR_NAME].data, target_c[b].postfix.len,
									target_c[b].postfix.data));
			break;
		}

		if (target[b] != CMAKE_END && gen->arr[PGC_ARR_COPYFILES].cnt > 0) {
			const str_t *copyfile;
			arr_foreach(&gen->arr[PGC_ARR_COPYFILES], copyfile)
			{
				cmake_add_custom_cmd_target(cmake, target[b], CMAKE_CMD_TARGET_PRE_BUILD,
							    strf("${CMAKE_COMMAND} -E copy_if_different %.*s ${CMAKE_CURRENT_SOURCE_DIR}", copyfile->len,
								 copyfile->data));
			}
		}

		if (target[b] != CMAKE_END && (gen->arr[PGC_ARR_DEPENDS].cnt > 0)) {
			uint depends = cmake_add_depends(cmake, target[b]);

			const str_t *depend;
			arr_foreach(&gen->arr[PGC_ARR_DEPENDS], depend)
			{
				cmake_cmd_add_str(cmake, depends, str_cpy(*depend));
			}
		}

		if (target[b] != CMAKE_END && is_src && gen->intdir[PGC_INTDIR_STR_DEFINES][target_c[b].intdir].len > 0) {
			uint defines = cmake_target_cmd(cmake, CMAKE_CMD_TARGET_DEFINES, target[b], CMAKE_SCOPE_PRIVATE);
			cmake_cmd_add_str(cmake, defines, str_cpy(gen->intdir[PGC_INTDIR_STR_DEFINES][target_c[b].intdir]));
		}

		if (target[b] != CMAKE_END && is_src) {
			int is_flags = 0;
			for (pgc_src_type_t s = 0; s < __PGC_SRC_TYPE_MAX; s++) {
				if (gen->src[PGC_SRC_STR_FLAGS][s].data && gen->src[PGC_SRC_STR_FLAGS][s].len > 0) {
					is_flags = 1;
				}
			}

			if (is_flags) {
				uint opts = cmake_target_cmd(cmake, CMAKE_CMD_TARGET_COMPILE_OPTS, target[b], CMAKE_SCOPE_PRIVATE);

				for (pgc_src_type_t s = 0; s < __PGC_SRC_TYPE_MAX; s++) {
					if (gen->src[PGC_SRC_STR_FLAGS][s].data == NULL || gen->src[PGC_SRC_STR_FLAGS][s].len <= 0) {
						continue;
					}

					cmake_cmd_add_str(cmake, opts,
							  strf("$<$<COMPILE_LANGUAGE:%s>:%.*s>", src_c[s].lang, gen->src[PGC_SRC_STR_FLAGS][s].len,
							       gen->src[PGC_SRC_STR_FLAGS][s].data));
				}
			}
		}

		if (target[b] != CMAKE_END && is_src && gen->arr[PGC_ARR_INCLUDES].cnt > 0) {
			uint inc_dirs = cmake_target_cmd(cmake, CMAKE_CMD_TARGET_INCLUDE_DIRS, target[b], CMAKE_SCOPE_PRIVATE);

			const str_t *include;
			arr_foreach(&gen->arr[PGC_ARR_INCLUDES], include)
			{
				cmake_cmd_add_str(cmake, inc_dirs, str_cpy(*include));
			}
		}

		if (target[b] != CMAKE_END && is_src && gen->arr[PGC_ARR_LIBS].cnt > 0) {
			uint link_dirs = cmake_target_cmd(cmake, CMAKE_CMD_TARGET_LINK_DIRS, target[b], CMAKE_SCOPE_PRIVATE);

			const pgc_lib_data_t *lib;
			arr_foreach(&gen->arr[PGC_ARR_LIBS], lib)
			{
				cmake_cmd_add_str(cmake, link_dirs, str_cpy(lib->dir));
			}
		}

		if (target[b] != CMAKE_END && is_src && gen->arr[PGC_ARR_LIBS].cnt > 0) {
			uint link_libs = cmake_target_cmd(cmake, CMAKE_CMD_TARGET_LINK_LIBS, target[b], CMAKE_SCOPE_PRIVATE);

			const pgc_lib_data_t *lib;
			arr_foreach(&gen->arr[PGC_ARR_LIBS], lib)
			{
				if (lib->name.data == NULL) {
					continue;
				}

				if (lib->link_type == PGC_LINK_SHARED) {
					cmake_cmd_add_str(cmake, link_libs, strf("-l:%.*s.so", lib->name.len, lib->name.data));
				} else if (lib->lib_type == PGC_LIB_INT) {
					cmake_cmd_add_str(cmake, link_libs, strf("%.*s_s", lib->name.len, lib->name.data));
				} else {
					cmake_cmd_add_str(cmake, link_libs, strf("-l:%.*s.a", lib->name.len, lib->name.data));
				}
			}
		}

		if (target[b] != CMAKE_END && is_src && gen->str[PGC_STR_LDFLAGS].len > 0) {
			uint link_opts = cmake_target_cmd(cmake, CMAKE_CMD_TARGET_LINK_OPTS, target[b], CMAKE_SCOPE_PRIVATE);
			cmake_cmd_add_str(cmake, link_opts, str_cpy(gen->str[PGC_STR_LDFLAGS]));
		}

		if (target[b] != CMAKE_END && gen->str[PGC_STR_OUTDIR].data) {
			uint props = cmake_set_target_props(cmake, target[b]);

			str_cpyd(gen->str[PGC_STR_OUTDIR], &buf);
			str_replace(&buf, STR("${CMAKE_BUILD_TYPE}"), STR("Debug"));
			cmake_add_target_prop(cmake, props, CMAKE_TARGET_PROP_ARC_OUT_DIR, str_cpy(buf));
			cmake_add_target_prop(cmake, props, CMAKE_TARGET_PROP_LIB_OUT_DIR, str_cpy(buf));
			cmake_add_target_prop(cmake, props, CMAKE_TARGET_PROP_RUN_OUT_DIR, str_cpy(buf));
			cmake_add_target_prop(cmake, props, CMAKE_TARGET_PROP_ARC_OUT_DIR_DBG, str_cpy(buf));
			cmake_add_target_prop(cmake, props, CMAKE_TARGET_PROP_LIB_OUT_DIR_DBG, str_cpy(buf));
			cmake_add_target_prop(cmake, props, CMAKE_TARGET_PROP_RUN_OUT_DIR_DBG, str_cpy(buf));

			str_cpyd(gen->str[PGC_STR_OUTDIR], &buf);
			str_replace(&buf, STR("${CMAKE_BUILD_TYPE}"), STR("Release"));
			cmake_add_target_prop(cmake, props, CMAKE_TARGET_PROP_ARC_OUT_DIR_RLS, str_cpy(buf));
			cmake_add_target_prop(cmake, props, CMAKE_TARGET_PROP_LIB_OUT_DIR_RLS, str_cpy(buf));
			cmake_add_target_prop(cmake, props, CMAKE_TARGET_PROP_RUN_OUT_DIR_RLS, str_cpy(buf));

			cmake_add_target_prop(cmake, props, CMAKE_TARGET_PROP_BUILD_RPATH, STR("\".\""));
			cmake_add_target_prop(cmake, props, CMAKE_TARGET_PROP_OUTPUT_NAME, str_cpy(gen->str[PGC_STR_NAME]));
			cmake_add_target_prop(cmake, props, CMAKE_TARGET_PROP_PREFIX, STR("\"\""));
		}
	}

	return cmake;
}

cmake_t *pgc_gen_cm_remote(const pgc_t *gen, cmake_t *cmake)
{
	cmake_add_custom_target(cmake, strf("%.*s", gen->str[PGC_STR_NAME].len, gen->str[PGC_STR_NAME].data));

	return cmake;
}

cmake_t *pgc_gen_cm(const pgc_t *gen, cmake_t *cmake)
{
	if (gen == NULL || cmake == NULL) {
		return NULL;
	}

	if (gen->str[PGC_STR_URL].data) {
		return pgc_gen_cm_remote(gen, cmake);
	}

	return pgc_gen_cm_local(gen, cmake);
}
