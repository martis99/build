#include "gen/cm/pgc_gen_cm.h"

#include "gen/pgc_common.h"

cmake_t *pgc_gen_cm_local(const pgc_t *pgc, cmake_t *cmake)
{
	static const char *header_ext[] = {
		[PGC_HEADER_INC] = "inc",
		[PGC_HEADER_H]	 = "h",
		[PGC_HEADER_HPP] = "hpp",
	};

	// clang-format off
	static const struct {
		const char *lang;
		const char *ext;
	} src_c[] = {
		[PGC_SRC_NASM] = { "ASM_NASM", "nasm" },
		[PGC_SRC_S]    = { "ASM",      "S" },
		[PGC_SRC_C]    = { "C",        "c" },
		[PGC_SRC_CPP]  = { "CXX",      "cpp" },
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

	if (pgc->intdir[PGC_INTDIR_STR_NAME][PGC_INTDIR_OBJECT].data && (pgc->arr[PGC_ARR_HEADERS].cnt > 0 || pgc->arr[PGC_ARR_SRCS].cnt > 0)) {
		str_t name = strz(pgc->intdir[PGC_INTDIR_STR_NAME][PGC_INTDIR_OBJECT].len + sizeof("_SOURCE"));
		str_cat(&name, pgc->intdir[PGC_INTDIR_STR_NAME][PGC_INTDIR_OBJECT]);
		str_cat(&name, STR("_SOURCE"));

		sources = cmake_file(cmake, CMAKE_FILE_GLOB_RECURSE, name);

		const pgc_str_flags_t *header;
		arr_foreach(&pgc->arr[PGC_ARR_HEADERS], header)
		{
			if (header->str.data == NULL) {
				continue;
			}

			for (pgc_header_type_t ext = 0; ext < __PGC_HEADER_TYPE_MAX; ext++) {
				if ((header->flags & (1 << ext)) == 0) {
					continue;
				}

				cmake_cmd_add_str(cmake, sources, strf("%.*s*.%s", header->str.len, header->str.data, header_ext[ext]));
			}
		}

		const pgc_str_flags_t *data;
		arr_foreach(&pgc->arr[PGC_ARR_SRCS], data)
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
		str_t name = pgc->intdir[PGC_INTDIR_STR_NAME][s_build_c[b].intdir];

		if (name.data == NULL || (pgc->builds & (1 << b)) == 0) {
			continue;
		}

		if (is_src) {
			switch (b) {
			case PGC_BUILD_EXE:
			case PGC_BUILD_BIN:
			case PGC_BUILD_ELF:
				target[b] = cmake_add_exe(cmake, str_cpy(name), sources);
				break;
			case PGC_BUILD_STATIC:
				target[b] = cmake_add_lib(cmake, str_cpy(name), CMAKE_LIB_STATIC, sources);
				break;
			case PGC_BUILD_SHARED:
				target[b] = cmake_add_lib(cmake, str_cpy(name), CMAKE_LIB_SHARED, sources);
				break;
			default:
				target[b] = cmake_add_custom_target(cmake, str_cpy(name));
				break;
			}
		} else {
			target[b] = cmake_add_custom_target(cmake, str_cpy(name));
		}

		if (target[b] != CMAKE_END && pgc->arr[PGC_ARR_COPYFILES].cnt > 0) {
			const pgc_str_flags_t *copyfile;
			arr_foreach(&pgc->arr[PGC_ARR_COPYFILES], copyfile)
			{
				if (!(copyfile->flags & (1 << s_build_c[b].intdir)) || copyfile->str.data == NULL) {
					continue;
				}

				cmake_add_custom_cmd_target(cmake, target[b], CMAKE_CMD_TARGET_PRE_BUILD,
							    strf("${CMAKE_COMMAND} -E copy_if_different %.*s ${CMAKE_CURRENT_SOURCE_DIR}", copyfile->str.len,
								 copyfile->str.data));
			}
		}

		if (target[b] != CMAKE_END && (pgc->arr[PGC_ARR_DEPENDS].cnt > 0)) {
			uint depends = cmake_add_depends(cmake, target[b]);

			const pgc_depend_data_t *depend;
			arr_foreach(&pgc->arr[PGC_ARR_DEPENDS], depend)
			{
				if (depend->name.data == NULL) {
					continue;
				}

				cmake_cmd_add_str(cmake, depends, str_cpy(depend->name));
			}
		}

		if (target[b] != CMAKE_END && is_src) {
			uint defines = CMAKE_END;

			const pgc_str_flags_t *define;
			arr_foreach(&pgc->arr[PGC_ARR_DEFINES], define)
			{
				if (!(define->flags & (1 << s_build_c[b].intdir)) || define->str.data == NULL) {
					continue;
				}

				if (defines == CMAKE_END) {
					defines = cmake_target_cmd(cmake, CMAKE_CMD_TARGET_DEFINES, target[b], CMAKE_SCOPE_PRIVATE);
				}

				cmake_cmd_add_str(cmake, defines, strf("-D%.*s", define->str.len, define->str.data));
			}
		}

		if (target[b] != CMAKE_END && is_src) {
			uint opts = CMAKE_END;

			for (pgc_src_type_t s = 0; s < __PGC_SRC_TYPE_MAX; s++) {
				const pgc_str_flags_t *flag;
				str_t tmp = { 0 };
				arr_foreach(&pgc->arr[PGC_ARR_FLAGS], flag)
				{
					if (!(flag->flags & (1 << s)) || flag->str.data == NULL) {
						continue;
					}

					if (opts == CMAKE_END) {
						opts = cmake_target_cmd(cmake, CMAKE_CMD_TARGET_COMPILE_OPTS, target[b], CMAKE_SCOPE_PRIVATE);
					}

					if (tmp.data == NULL) {
						tmp = strf("$<$<COMPILE_LANGUAGE:%s>:", src_c[s].lang);
					} else {
						str_cat(&tmp, STR(" "));
					}

					str_cat(&tmp, flag->str);
				}

				if (tmp.data) {
					str_cat(&tmp, STR(">"));
					cmake_cmd_add_str(cmake, opts, tmp);
				}
			}
		}

		if (target[b] != CMAKE_END && is_src && pgc->arr[PGC_ARR_INCLUDES].cnt > 0) {
			uint inc_dirs = cmake_target_cmd(cmake, CMAKE_CMD_TARGET_INCLUDE_DIRS, target[b], CMAKE_SCOPE_PRIVATE);

			const pgc_str_flags_t *include;
			arr_foreach(&pgc->arr[PGC_ARR_INCLUDES], include)
			{
				if (include->str.data == NULL) {
					continue;
				}

				switch (include->flags) {
				case PGC_SCOPE_PRIVATE:
				case PGC_SCOPE_PUBLIC:
					cmake_cmd_add_str(cmake, inc_dirs, str_cpy(include->str));
					break;
				default:
					break;
				}
			}
		}

		if (target[b] != CMAKE_END && is_src && pgc->arr[PGC_ARR_LIBS].cnt > 0) {
			uint link_dirs = cmake_target_cmd(cmake, CMAKE_CMD_TARGET_LINK_DIRS, target[b], CMAKE_SCOPE_PRIVATE);

			const pgc_lib_data_t *lib;
			arr_foreach(&pgc->arr[PGC_ARR_LIBS], lib)
			{
				if (!(lib->intdirs & (1 << s_build_c[b].intdir)) || lib->dir.data == NULL) {
					continue;
				}

				switch (lib->link_type) {
				case PGC_LINK_STATIC:
				case PGC_LINK_SHARED:
					cmake_cmd_add_str(cmake, link_dirs, str_cpy(lib->dir));
					break;
				default:
					break;
				}
			}
		}

		if (target[b] != CMAKE_END && is_src && pgc->arr[PGC_ARR_LIBS].cnt > 0) {
			uint link_libs = cmake_target_cmd(cmake, CMAKE_CMD_TARGET_LINK_LIBS, target[b], CMAKE_SCOPE_PRIVATE);

			const pgc_lib_data_t *lib;
			arr_foreach(&pgc->arr[PGC_ARR_LIBS], lib)
			{
				if (!(lib->intdirs & (1 << s_build_c[b].intdir)) || lib->name.data == NULL) {
					continue;
				}

				switch (lib->link_type) {
				case PGC_LINK_STATIC:
					if (lib->lib_type == PGC_LIB_INT) {
						cmake_cmd_add_str(cmake, link_libs, strf("%.*s_s", lib->name.len, lib->name.data));
					} else {
						cmake_cmd_add_str(cmake, link_libs, strf("-l:%.*s.a", lib->name.len, lib->name.data));
					}
					break;
				case PGC_LINK_SHARED:
					cmake_cmd_add_str(cmake, link_libs, strf("-l:%.*s.so", lib->name.len, lib->name.data));
					break;
				default:
					break;
				}
			}
		}

		if (target[b] != CMAKE_END && is_src && pgc->arr[PGC_ARR_LDFLAGS].cnt > 0) {
			uint link_opts = cmake_target_cmd(cmake, CMAKE_CMD_TARGET_LINK_OPTS, target[b], CMAKE_SCOPE_PRIVATE);

			const str_t *ldflag;
			arr_foreach(&pgc->arr[PGC_ARR_LDFLAGS], ldflag)
			{
				if (ldflag->data == NULL) {
					continue;
				}

				cmake_cmd_add_str(cmake, link_opts, str_cpy(*ldflag));
			}
		}

		if (target[b] != CMAKE_END && pgc->str[PGC_STR_OUTDIR].data) {
			uint props = cmake_set_target_props(cmake, target[b]);

			str_cpyd(pgc->str[PGC_STR_OUTDIR], &buf);
			str_replace(&buf, STR("${CMAKE_BUILD_TYPE}"), STR("Debug"));
			cmake_add_target_prop(cmake, props, CMAKE_TARGET_PROP_ARC_OUT_DIR, str_cpy(buf));
			cmake_add_target_prop(cmake, props, CMAKE_TARGET_PROP_LIB_OUT_DIR, str_cpy(buf));
			cmake_add_target_prop(cmake, props, CMAKE_TARGET_PROP_RUN_OUT_DIR, str_cpy(buf));

			if (pgc_get_config(pgc, STR("Debug")) != PGC_END) {
				cmake_add_target_prop(cmake, props, CMAKE_TARGET_PROP_ARC_OUT_DIR_DBG, str_cpy(buf));
				cmake_add_target_prop(cmake, props, CMAKE_TARGET_PROP_LIB_OUT_DIR_DBG, str_cpy(buf));
				cmake_add_target_prop(cmake, props, CMAKE_TARGET_PROP_RUN_OUT_DIR_DBG, str_cpy(buf));
			}

			if (pgc_get_config(pgc, STR("Release")) != PGC_END) {
				str_cpyd(pgc->str[PGC_STR_OUTDIR], &buf);
				str_replace(&buf, STR("${CMAKE_BUILD_TYPE}"), STR("Release"));
				cmake_add_target_prop(cmake, props, CMAKE_TARGET_PROP_ARC_OUT_DIR_RLS, str_cpy(buf));
				cmake_add_target_prop(cmake, props, CMAKE_TARGET_PROP_LIB_OUT_DIR_RLS, str_cpy(buf));
				cmake_add_target_prop(cmake, props, CMAKE_TARGET_PROP_RUN_OUT_DIR_RLS, str_cpy(buf));
			}

			cmake_add_target_prop(cmake, props, CMAKE_TARGET_PROP_BUILD_RPATH, STR("\".\""));
			cmake_add_target_prop(cmake, props, CMAKE_TARGET_PROP_OUTPUT_NAME, str_cpy(pgc->intdir[PGC_INTDIR_STR_NAME][PGC_INTDIR_OBJECT]));
			cmake_add_target_prop(cmake, props, CMAKE_TARGET_PROP_PREFIX, STR("\"\""));
		}
	}

	return cmake;
}

cmake_t *pgc_gen_cm_remote(const pgc_t *pgc, cmake_t *cmake)
{
	cmake_add_custom_target(cmake, str_cpy(pgc->intdir[PGC_INTDIR_STR_NAME][PGC_INTDIR_OBJECT]));

	return cmake;
}

cmake_t *pgc_gen_cm(const pgc_t *pgc, cmake_t *cmake)
{
	if (pgc == NULL || cmake == NULL) {
		return NULL;
	}

	if (pgc->str[PGC_STR_URL].data) {
		return pgc_gen_cm_remote(pgc, cmake);
	}

	return pgc_gen_cm_local(pgc, cmake);
}
