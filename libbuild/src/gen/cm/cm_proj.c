#include "cm_proj.h"

#include "gen/mk/mk_pgen.h"
#include "gen/sln.h"
#include "gen/var.h"

#include "common.h"
/*
static int cm_proj_gen_proj(const proj_t *proj, const dict_t *projects, const prop_t *sln_props, FILE *file, int shared)
{
	switch (type) {
	case PROJ_TYPE_LIB:
		c_fprintf(file, "add_library(%.*s%s %s ${%.*s%s_SOURCE})\n", name->len, name->data, shared ? "_d" : "", shared ? "SHARED" : "STATIC", name->len,
			  name->data, shared ? "_d" : "");

		if (shared && proj->all_depends.cnt > 0) {
			c_fprintf(file, "target_link_libraries(%.*s_d", name->len, name->data);

			for (uint i = 0; i < proj->all_depends.cnt; i++) {
				const proj_dep_t *dep = arr_get(&proj->all_depends, i);

				if (dep->proj->props[PROJ_PROP_TYPE].mask != PROJ_TYPE_EXT) {
					c_fprintf(file, " %.*s%s", dep->proj->name.len, dep->proj->name.data, dep->link_type == LINK_TYPE_SHARED ? "_d" : "");
				}

				if (dep->proj->props[PROJ_PROP_LINK].flags & PROP_SET) {
					const arr_t *links = &dep->proj->props[PROJ_PROP_LINK].arr;
					for (uint j = 0; j < links->cnt; j++) {
						const prop_str_t *link = arr_get(links, j);
						if (link->val.len > 0) {
							c_fprintf(file, " %.*s", link->val.len, link->val.data);
						}
					}
				}
			}
			c_fprintf(file, ")\n");
		}
		break;
	case PROJ_TYPE_EXE: {
		int first = 0;
		for (uint i = 0; i < proj->all_depends.cnt; i++) {
			const proj_dep_t *dep = arr_get(&proj->all_depends, i);

			if ((dep->proj->props[PROJ_PROP_LIBDIRS].flags & PROP_SET) && dep->proj->props[PROJ_PROP_LIBDIRS].arr.cnt > 0) {
				first = 1;
				break;
			}
		}

		if (first) {
			c_fprintf(file, "link_directories(");
			first = 1;
			for (uint i = 0; i < proj->all_depends.cnt; i++) {
				const proj_dep_t *dep = arr_get(&proj->all_depends, i);

				if (dep->proj->props[PROJ_PROP_LIBDIRS].flags & PROP_SET) {
					const arr_t *libdirs = &dep->proj->props[PROJ_PROP_LIBDIRS].arr;
					for (uint j = 0; j < libdirs->cnt; j++) {
						prop_str_t *libdir = arr_get(libdirs, j);
						if (libdir->val.len > 0) {
							if (!first) {
								c_fprintf(file, " ");
							}

							buf_len = resolve(libdir, buf, sizeof(buf), dep->proj);

							print_rel_path(file, dep->proj, buf, buf_len);
							first = 0;
						}
					}
					break;
				}
			}

			c_fprintf(file, ")\n");
		}

		c_fprintf(file, "add_executable(%.*s ${%.*s_SOURCE})\n", name->len, name->data, name->len, name->data);

		if (proj->all_depends.cnt > 0) {
			c_fprintf(file, "target_link_libraries(%.*s", name->len, name->data);

			for (uint i = 0; i < proj->all_depends.cnt; i++) {
				const proj_dep_t *dep = arr_get(&proj->all_depends, i);

				if (dep->proj->props[PROJ_PROP_TYPE].mask != PROJ_TYPE_EXT) {
					c_fprintf(file, " %.*s%s", dep->proj->name.len, dep->proj->name.data, dep->link_type == LINK_TYPE_SHARED ? "_d" : "");
				}

				if (dep->proj->props[PROJ_PROP_LINK].flags & PROP_SET) {
					const arr_t *links = &dep->proj->props[PROJ_PROP_LINK].arr;
					for (uint j = 0; j < links->cnt; j++) {
						const prop_str_t *link = arr_get(links, j);
						if (link->val.len > 0) {
							c_fprintf(file, " %.*s", link->val.len, link->val.data);
						}
					}
				}
			}
			c_fprintf(file, ")\n");
		}

		break;
	}
	case PROJ_TYPE_EXT:
		//c_fprintf(fp, "add_library(%.*s STATIC ${%.*s_SOURCE})\n", name->len, name->data, name->len, name->data);
		goto done;
		break;
	default:
		ERR("unknown project type: %d", type);
		ret = 1;
	}

	if (!proj->props[PROJ_PROP_SOURCE].flags & PROP_SET) {
		if (lang & (1 << LANG_C)) {
			c_fprintf(file, "set_target_properties(%.*s%s PROPERTIES LINKER_LANGUAGE C)\n", name->len, name->data, shared ? "_d" : "");
		}
	}

	if (proj->rel_dir.len > 0 || (outdir->flags & PROP_SET) || (intdir->flags & PROP_SET)) {
		c_fprintf(file, "set_target_properties(%.*s%s\n    PROPERTIES\n", name->len, name->data, shared ? "_d" : "");

		if (proj->rel_dir.len > 0) {
			buf_len = proj->rel_dir.len;
			mem_cpy(buf, sizeof(buf), proj->rel_dir.path, proj->rel_dir.len);
			convert_slash(buf, proj->rel_dir.len);
			c_fprintf(file, "    FOLDER \"%.*s\"\n", buf_len - 1, buf);
		}

		if (outdir->flags & PROP_SET) {
			buf_len = resolve(&outdir->value, CSTR(buf), proj);
			c_fprintf(file, "    ARCHIVE_OUTPUT_DIRECTORY_DEBUG \"");
			print_rel_path(file, proj, buf, buf_len);
			c_fprintf(file, "\"\n    ARCHIVE_OUTPUT_DIRECTORY_RELEASE \"");
			print_rel_path(file, proj, buf, buf_len);
			c_fprintf(file, "\"\n    LIBRARY_OUTPUT_DIRECTORY_DEBUG \"");
			print_rel_path(file, proj, buf, buf_len);
			c_fprintf(file, "\"\n    LIBRARY_OUTPUT_DIRECTORY_RELEASE \"");
			print_rel_path(file, proj, buf, buf_len);
			c_fprintf(file, "\"\n    RUNTIME_OUTPUT_DIRECTORY_DEBUG \"");
			print_rel_path(file, proj, buf, buf_len);
			c_fprintf(file, "\"\n    RUNTIME_OUTPUT_DIRECTORY_RELEASE \"");
			print_rel_path(file, proj, buf, buf_len);
			c_fprintf(file, "\"\n)\n");
		}
	}

	if (proj->props[PROJ_PROP_WDIR].flags & PROP_SET) {
		const prop_str_t *wdir = &proj->props[PROJ_PROP_WDIR].value;

		buf_len = wdir->val.len;
		mem_cpy(buf, sizeof(buf), wdir->val.data, wdir->val.len);
		convert_slash(buf, wdir->val.len);
		if (wdir->val.len > 0) {
			c_fprintf(file, "set_property(TARGET %.*s%s PROPERTY VS_DEBUGGER_WORKING_DIRECTORY \"${CMAKE_SOURCE_DIR}/%.*s\")\n", name->len, name->data,
				  shared ? "_d" : "", buf_len - 1, buf);
		}
	} else {
		buf_len = proj->rel_dir.len;
		mem_cpy(buf, sizeof(buf), proj->rel_dir.path, proj->rel_dir.len);
		convert_slash(buf, proj->rel_dir.len);
		c_fprintf(file, "set_property(TARGET %.*s%s PROPERTY VS_DEBUGGER_WORKING_DIRECTORY \"${CMAKE_SOURCE_DIR}/%.*s\")\n", name->len, name->data,
			  shared ? "_d" : "", proj->rel_dir.len - 1, buf);
	}

#if defined(C_WIN)
	if (type == PROJ_TYPE_EXE) {
		for (uint i = 0; i < proj->all_depends.cnt; i++) {
			const proj_dep_t *dep = arr_get(&proj->all_depends, i);

			if (dep->link_type != LINK_TYPE_SHARED) {
				continue;
			}

			path_t target = { 0 };
			path_init(&target, "", 0);
			target.len = resolve(&dep->proj->props[PROJ_PROP_OUTDIR].value, target.path, sizeof(target.path), dep->proj);
			path_child(&target, dep->proj->name.data, dep->proj->name.len);
			path_child_s(&target, CSTR("_d.dll"), 0);

			c_fprintf(file,
				  "add_custom_command(TARGET %.*s POST_BUILD\n"
				  "    COMMAND ${CMAKE_COMMAND} -E copy_if_different\n"
				  "    \"%.*s\"\n"
				  "    $<TARGET_FILE_DIR:%.*s>\n"
				  ")\n",
				  proj->name.len, proj->name.data, target.len, target.path, proj->name.len, proj->name.data);

			if (dep->proj->props[PROJ_PROP_LINK].flags & PROP_SET) {
				const arr_t *links = &dep->proj->props[PROJ_PROP_LINK].arr;
				for (uint j = 0; j < links->cnt; j++) {
					const prop_str_t *link = arr_get(links, j);
					if (link->val.len > 0) {
						c_fprintf(file, " %.*s", link->val.len, link->val.data);
					}
				}
			}
		}
	}
#endif

done:
	return ret;
}
*/
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

typedef struct mk_pgen_lib_data_s {
	str_t dir;
	str_t name;
	mk_pgen_link_type_t link_type;
	mk_pgen_lib_type_t lib_type;
} mk_pgen_lib_data_t;

//TODO: Resolve when reading config file
static str_t resolve_path(str_t rel, str_t path, str_t *buf)
{
	if (str_eqn(path, STR("$(SLN_DIR)"), 9)) {
		str_cpyd(path, buf);
		return *buf;
	}

	buf->len = 0;
	str_cat(buf, STR("$(SLN_DIR)"));
	str_cat(buf, rel);
	str_cat(buf, path);
	return *buf;
}

static str_t resolve(str_t str, str_t *buf, const proj_t *proj)
{
	str_cpyd(str, buf);
	str_replace(buf, STR("$(SLN_DIR)"), STR("${CMAKE_SOURCE_DIR}/"));
	str_replace(buf, STR("$(PROJ_DIR)"), strc(proj->rel_dir.path, proj->rel_dir.len));
	str_replace(buf, STR("$(PROJ_NAME)"), strc(proj->name.data, proj->name.len));
	//str_replace(buf, STR("$(CONFIG)"), STR("$(Configuration)"));
	str_replace(buf, STR("$(CONFIG)"), STR("Debug"));
	str_replace(buf, STR("$(ARCH)"), STR("${CMAKE_VS_PLATFORM_NAME}"));
	convert_slash((char *)buf->data, buf->len);
	return *buf;
}

static void cm_pgen(mk_pgen_t *gen, print_dst_t dst)
{
	static const char *header_ext[] = {
		[MK_EXT_INC] = "inc",
		[MK_EXT_H]   = "h",
		[MK_EXT_HPP] = "hpp",
	};

	// clang-format off
	static struct {
		str_t name;
		str_t flags;
		const char *ext;
		int cov;
	} src_c[] = {
		[MK_EXT_ASM] = { STRS("SRC_ASM"), STRS("ASFLAGS"),   "nasm", 0 },
		[MK_EXT_S]   = { STRS("SRC_S"),  STRS("ASFLAGS"),    "S",    0 },
		[MK_EXT_C]   = { STRS("SRC_C"),    STRS("CFLAGS"),   "c",    1 },
		[MK_EXT_CPP] = { STRS("SRC_CPP"),  STRS("CXXFLAGS"), "cpp",  1 },
	};

	static struct {
		str_t postfix;
		mk_pgen_intdir_type_t intdir;
	} target_c[] = {
		[MK_BUILD_EXE]	  = { STRS(""),	   MK_INTDIR_OBJECT },
		[MK_BUILD_STATIC] = { STRS("_s"),  MK_INTDIR_STATIC },
		[MK_BUILD_SHARED] = { STRS("_d"),  MK_INTDIR_SHARED },
		[MK_BUILD_ELF]	  = { STRS(""),	   MK_INTDIR_OBJECT },
		[MK_BUILD_BIN]	  = { STRS(""),	   MK_INTDIR_OBJECT },
		[MK_BUILD_FAT12]  = { STRS(""),	   __MK_INTDIR_MAX  },
	};
	// clang-format on

	char buf_d[P_MAX_PATH] = { 0 };

	str_t buf = strb(buf_d, sizeof(buf_d), 0);

	int src[__MK_SRC_MAX]	   = { 0 };
	int target[__MK_BUILD_MAX] = { 0 };

	int is_src = 0;

	if (gen->headers.cnt > 0 || gen->srcs.cnt > 0) {
		dprintf(dst, "file(GLOB_RECURSE %.*s_SOURCE", gen->name.len, gen->name.data);

		const mk_pgen_header_data_t *header;
		arr_foreach(&gen->headers, header)
		{
			for (mk_pgen_header_ext_t ext = 0; ext < __MK_HEADER_MAX; ext++) {
				if ((header->exts & (1 << ext)) == 0) {
					continue;
				}

				dprintf(dst, " %.*s*.%s", header->dir.len, header->dir.data, header_ext[ext]);
			}
		}

		const mk_pgen_src_data_t *data;
		arr_foreach(&gen->srcs, data)
		{
			for (mk_pgen_src_ext_t s = 0; s < __MK_SRC_MAX; s++) {
				if ((data->exts & (1 << s)) == 0) {
					continue;
				}

				is_src = 1;
				src[s] = 1;
				dprintf(dst, " %.*s*.%s", data->dir.len, data->dir.data, src_c[s].ext);
			}
		}

		dprintf(dst, ")\n\n");
	}

	for (mk_pgen_build_type_t b = 0; b < __MK_BUILD_MAX; b++) {
		if ((gen->builds & (1 << b)) == 0) {
			continue;
		}

		switch (b) {
		case MK_BUILD_EXE:
			if (is_src) {
				dprintf(dst, "add_executable(%.*s%.*s ${%.*s_SOURCE})\n", gen->name.len, gen->name.data, target_c[b].postfix.len,
					target_c[b].postfix.data, gen->name.len, gen->name.data);
				target[b] = 1;
			}
			break;
		case MK_BUILD_STATIC:
			if (is_src) {
				dprintf(dst, "add_library(%.*s%.*s STATIC ${%.*s_SOURCE})\n", gen->name.len, gen->name.data, target_c[b].postfix.len,
					target_c[b].postfix.data, gen->name.len, gen->name.data);
				target[b] = 1;
			} else {
				dprintf(dst, "add_custom_target(%.*s%.*s)\n", gen->name.len, gen->name.data, target_c[b].postfix.len, target_c[b].postfix.data);
				target[b] = 1;
				/*dprintf(dst, "add_library(%.*s%.*s STATIC IMPORTED)\n", gen->name.len, gen->name.data, target_c[b].postfix.len, target_c[b].postfix.data);
				target[b] = 1;*/
			}
			break;
		case MK_BUILD_SHARED:
			if (is_src) {
				dprintf(dst, "add_library(%.*s%.*s SHARED ${%.*s_SOURCE})\n", gen->name.len, gen->name.data, target_c[b].postfix.len,
					target_c[b].postfix.data, gen->name.len, gen->name.data);
				target[b] = 1;
			} else {
				dprintf(dst, "add_custom_target(%.*s%.*s)\n", gen->name.len, gen->name.data, target_c[b].postfix.len, target_c[b].postfix.data);
				target[b] = 1;
				/*dprintf(dst, "add_library(%.*s%.*s SHARED IMPORTED)\n", gen->name.len, gen->name.data, target_c[b].postfix.len, target_c[b].postfix.data);
				target[b] = 1;*/
			}
			break;
		default:
			break;
		}

		if (target[b] && gen->copyfiles.cnt > 0) {
			const str_t *copyfile;
			arr_foreach(&gen->copyfiles, copyfile)
			{
				/*pathv_t outputdir = { .path = copyfile->data, .len = copyfile->len };

				str_t output = { 0 };
				pathv_get_dir(outputdir, &output);

				dprintf(dst,
					"add_custom_command(OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/%.*s\n"
					"\tCOMMAND ${CMAKE_COMMAND} -E copy_if_different %.*s ${CMAKE_CURRENT_SOURCE_DIR}\n"
					//"\tCOMMENT \"Copying files from %.*s to ${CMAKE_CURRENT_SOURCE_DIR}\"\n"
					")\n",
					output.len, output.data, copyfile->len, copyfile->data
					//, copyfile->len, copyfile->data
				);
				dprintf(dst, "add_custom_target(copy_files_%.*s%.*s_%d ALL DEPENDS %.*s)\n\n", gen->name.len, gen->name.data, target_c[b].postfix.len,
					target_c[b].postfix.data, _i, copyfile->len, copyfile->data);*/

				dprintf(dst,
					"add_custom_command(TARGET %.*s%.*s PRE_BUILD\n"
					"\tCOMMAND ${CMAKE_COMMAND} -E copy_if_different %.*s ${CMAKE_CURRENT_SOURCE_DIR}\n"
					")\n",
					gen->name.len, gen->name.data, target_c[b].postfix.len, target_c[b].postfix.data, copyfile->len, copyfile->data);
			}
		}

		if (target[b] && (gen->depends.cnt > 0 /* || gen->copyfiles.cnt > 0*/)) {
			dprintf(dst, "add_dependencies(%.*s%.*s", gen->name.len, gen->name.data, target_c[b].postfix.len, target_c[b].postfix.data);

			const str_t *depend;
			arr_foreach(&gen->depends, depend)
			{
				dprintf(dst, " %.*s", depend->len, depend->data);
			}

			/*const str_t *copyfile;
			arr_foreach(&gen->copyfiles, copyfile)
			{
				dprintf(dst, " copy_files_%.*s%.*s_%d", gen->name.len, gen->name.data, target_c[b].postfix.len, target_c[b].postfix.data, _i);
			}*/

			dprintf(dst, ")\n");
		}

		/*if (target[b] && !is_src) {
			dprintf(dst, "set_target_properties(%.*s%.*s PROPERTIES IMPORTED_LOCATION %.*s)\n", gen->name.len, gen->name.data, target_c[b].postfix.len,
				target_c[b].postfix.data, gen->lib[target_c[b].intdir].len, gen->lib[target_c[b].intdir].data);
		}*/

		if (target[b] && is_src && gen->defines[target_c[b].intdir].len > 0) {
			dprintf(dst, "target_compile_definitions(%.*s%.*s PRIVATE %.*s)\n", gen->name.len, gen->name.data, target_c[b].postfix.len,
				target_c[b].postfix.data, gen->defines[target_c[b].intdir].len, gen->defines[target_c[b].intdir].data);
		}

		/*if (target[b]) {
			if (src[MK_EXT_CPP]) {
				dprintf(dst, "set_target_properties(%.*s%.*s PROPERTIES LINKER_LANGUAGE CXX)\n", gen->name.len, gen->name.data, target_c[b].postfix.len,
					target_c[b].postfix.data);
			} else if (src[MK_EXT_C]) {
				dprintf(dst, "set_target_properties(%.*s%.*s PROPERTIES LINKER_LANGUAGE C)\n", gen->name.len, gen->name.data, target_c[b].postfix.len,
					target_c[b].postfix.data);
			} else if (src[MK_EXT_S]) {
				dprintf(dst, "set_target_properties(%.*s%.*s PROPERTIES LINKER_LANGUAGE ASM)\n", gen->name.len, gen->name.data, target_c[b].postfix.len,
					target_c[b].postfix.data);
			}
		}*/

		if (target[b] && is_src && gen->includes.cnt > 0) {
			dprintf(dst, "target_include_directories(%.*s%.*s PRIVATE", gen->name.len, gen->name.data, target_c[b].postfix.len, target_c[b].postfix.data);

			const str_t *include;
			arr_foreach(&gen->includes, include)
			{
				dprintf(dst, " %.*s", include->len, include->data);
			}

			dprintf(dst, ")\n");
		}

		if (target[b] && is_src && gen->libs.cnt > 0) {
			dprintf(dst, "target_link_directories(%.*s%.*s PRIVATE", gen->name.len, gen->name.data, target_c[b].postfix.len, target_c[b].postfix.data);

			const mk_pgen_lib_data_t *lib;
			arr_foreach(&gen->libs, lib)
			{
				if (lib->link_type == MK_INTDIR_SHARED) {
					dprintf(dst, " %.*s", lib->dir.len, lib->dir.data);
				} else {
					dprintf(dst, " %.*s", lib->dir.len, lib->dir.data);
				}
			}

			dprintf(dst, ")\n");
		}

		if (target[b] && is_src && gen->libs.cnt > 0) {
			dprintf(dst, "target_link_libraries(%.*s%.*s PRIVATE", gen->name.len, gen->name.data, target_c[b].postfix.len, target_c[b].postfix.data);

			const mk_pgen_lib_data_t *lib;
			arr_foreach(&gen->libs, lib)
			{
				if (lib->lib_type == MK_LIB_INT) {
					if (lib->link_type == MK_LINK_SHARED) {
						dprintf(dst, " %.*s_d", lib->name.len, lib->name.data);
					} else {
						dprintf(dst, " %.*s_s", lib->name.len, lib->name.data);
					}
				} else {
					if (lib->link_type == MK_LINK_SHARED) {
						dprintf(dst, " -l:%.*s.so", lib->name.len, lib->name.data);
					} else {
						dprintf(dst, " -l:%.*s.a", lib->name.len, lib->name.data);
					}
				}
			}

			dprintf(dst, ")\n");
		}

		if (target[b] && is_src && gen->ldflags.len > 0) {
			dprintf(dst, "target_link_options(%.*s%.*s PRIVATE %.*s)\n", gen->name.len, gen->name.data, target_c[b].postfix.len, target_c[b].postfix.data,
				gen->ldflags.len, gen->ldflags.data);
		}

		if (target[b] && gen->outdir.data) {
			dprintf(dst, "set_target_properties(%.*s%.*s\n\tPROPERTIES\n", gen->name.len, gen->name.data, target_c[b].postfix.len, target_c[b].postfix.data);

			dprintf(dst, "\tARCHIVE_OUTPUT_DIRECTORY %.*s\n", gen->outdir.len, gen->outdir.data);
			dprintf(dst, "\tLIBRARY_OUTPUT_DIRECTORY %.*s\n", gen->outdir.len, gen->outdir.data);
			dprintf(dst, "\tRUNTIME_OUTPUT_DIRECTORY %.*s\n", gen->outdir.len, gen->outdir.data);
			dprintf(dst, "\tOUTPUT_NAME %.*s\n", gen->name.len, gen->name.data);
			dprintf(dst, "\tPREFIX \"\"\n");

			dprintf(dst, "\n)\n");
		}
	}
}

static int cm_proj_gen_proj2(const proj_t *proj, const dict_t *projects, const prop_t *sln_props, FILE *file, int shared)
{
	mk_pgen_t lgen = { 0 };
	mk_pgen_init(&lgen);

	mk_pgen_t *gen = &lgen;

	char buf_d[P_MAX_PATH] = { 0 };

	str_t buf = strb(buf_d, sizeof(buf_d), 0);

	int ret = 0;

	const str_t *filename = &proj->name;

	const prop_t *pfilename = &proj->props[PROJ_PROP_FILENAME];
	if (pfilename->flags & PROP_SET) {
		filename = &pfilename->value.val;
	}
	gen->name = str_cpy(*filename);

	switch (proj->props[PROJ_PROP_TYPE].mask) {
	case PROJ_TYPE_EXE:
		gen->builds = F_MK_BUILD_EXE;
		break;
	case PROJ_TYPE_BIN:
		gen->builds = F_MK_BUILD_BIN;
		break;
	case PROJ_TYPE_ELF:
		gen->builds = F_MK_BUILD_ELF;
		break;
	case PROJ_TYPE_FAT12:
		gen->builds = F_MK_BUILD_FAT12;
		break;
	case PROJ_TYPE_LIB:
		gen->builds = F_MK_BUILD_STATIC | F_MK_BUILD_SHARED;
		break;
	case PROJ_TYPE_EXT:
		break;
	}

	const prop_t *outdir = &proj->props[PROJ_PROP_OUTDIR];
	if (outdir->flags & PROP_SET) {
		gen->outdir = str_cpy(resolve(outdir->value.val, &buf, proj));
		switch (proj->props[PROJ_PROP_TYPE].mask) {
		case PROJ_TYPE_EXE: {
			str_t c = strn(buf.data, buf.len, buf.len + 5);
			str_cat(&c, STR("cov/"));
			gen->covdir = c;
			break;
		}
		default:
			break;
		}
	}

	if (proj->props[PROJ_PROP_SOURCE].arr.cnt > 0) {
		const prop_t *intdir = &proj->props[PROJ_PROP_INTDIR];
		if (intdir->flags & PROP_SET) {
			resolve(intdir->value.val, &buf, proj);
			switch (proj->props[PROJ_PROP_TYPE].mask) {
			case PROJ_TYPE_EXE:
			case PROJ_TYPE_BIN:
			case PROJ_TYPE_ELF:
				gen->intdir[MK_INTDIR_OBJECT] = str_cpy(buf);
				break;
			case PROJ_TYPE_LIB: {
				resolve(intdir->value.val, &buf, proj);
				str_t s = strn(buf.data, buf.len, buf.len + 8);
				str_cat(&s, STR("static/"));
				gen->intdir[MK_INTDIR_STATIC] = s;

				str_t d = strn(buf.data, buf.len, buf.len + 8);
				str_cat(&d, STR("shared/"));
				gen->intdir[MK_INTDIR_SHARED] = d;
				break;
			}
			default:
				break;
			}
		}
	}

	const prop_str_t *config;
	arr_foreach(&sln_props[SLN_PROP_CONFIGS].arr, config)
	{
		mk_pgen_add_config(gen, strs(config->val));
	}

	uint lang = proj->props[PROJ_PROP_LANGS].mask;
	const prop_str_t *include;
	arr_foreach(&proj->props[PROJ_PROP_INCLUDE].arr, include)
	{
		int exts = 0;
		exts |= lang & (1 << LANG_NASM) ? F_MK_EXT_INC : 0;
		exts |= lang & (1 << LANG_ASM) ? F_MK_EXT_INC : 0;
		exts |= lang & (1 << LANG_C) || lang & (1 << LANG_CPP) ? F_MK_EXT_H : 0;
		exts |= lang & (1 << LANG_CPP) ? F_MK_EXT_HPP : 0;

		if (exts) {
			mk_pgen_add_header(gen, strs(include->val), exts);
		}
	}

	const prop_str_t *source;
	arr_foreach(&proj->props[PROJ_PROP_SOURCE].arr, source)
	{
		int headers = 0;
		headers |= lang & (1 << LANG_NASM) ? F_MK_EXT_INC : 0;
		headers |= lang & (1 << LANG_ASM) ? F_MK_EXT_INC : 0;
		headers |= lang & (1 << LANG_C) || lang & (1 << LANG_CPP) ? F_MK_EXT_H : 0;
		headers |= lang & (1 << LANG_CPP) ? F_MK_EXT_HPP : 0;

		if (headers) {
			mk_pgen_add_header(gen, strs(source->val), headers);
		}

		int srcs = 0;
		srcs |= lang & (1 << LANG_NASM) ? F_MK_EXT_ASM : 0;
		srcs |= lang & (1 << LANG_ASM) ? F_MK_EXT_S : 0;
		srcs |= lang & (1 << LANG_C) ? F_MK_EXT_C : 0;
		srcs |= lang & (1 << LANG_CPP) ? F_MK_EXT_CPP : 0;

		if (srcs) {
			mk_pgen_add_src(gen, strs(source->val), srcs);
		}
	}

	const prop_t *partifact = &proj->props[PROJ_PROP_ARTIFACT];
	if (partifact->flags & PROP_SET) {
		//TODO: Different names for different build types
		gen->artifact[MK_BUILD_EXE]    = strs(partifact->value.val);
		gen->artifact[MK_BUILD_BIN]    = strs(partifact->value.val);
		gen->artifact[MK_BUILD_ELF]    = strs(partifact->value.val);
		gen->artifact[MK_BUILD_FAT12]  = strs(partifact->value.val);
		gen->artifact[MK_BUILD_STATIC] = strs(partifact->value.val);
		gen->artifact[MK_BUILD_SHARED] = strs(partifact->value.val);
	}

	if (proj->props[PROJ_PROP_ARGS].flags & PROP_SET) {
		gen->args = str_cpy(resolve(proj->props[PROJ_PROP_ARGS].value.val, &buf, proj));
	}

	arr_foreach(&proj->props[PROJ_PROP_SOURCE].arr, source)
	{
		mk_pgen_add_include(gen, str_cpy(resolve(source->val, &buf, proj)));
	}

	arr_foreach(&proj->props[PROJ_PROP_INCLUDE].arr, include)
	{
		mk_pgen_add_include(gen, str_cpy(resolve(include->val, &buf, proj)));
	}

	for (uint i = 0; i < proj->includes.cnt; i++) {
		const proj_t *iproj = *(proj_t **)arr_get(&proj->includes, i);

		str_t rel = strc(iproj->rel_dir.path, iproj->rel_dir.len);

		arr_foreach(&iproj->props[PROJ_PROP_INCLUDE].arr, include)
		{
			mk_pgen_add_include(gen, str_cpy(resolve(resolve_path(rel, include->val, &buf), &buf, iproj)));
		}

		if (iproj->props[PROJ_PROP_ENCLUDE].flags & PROP_SET) {
			mk_pgen_add_include(gen, str_cpy(resolve(resolve_path(rel, iproj->props[PROJ_PROP_ENCLUDE].value.val, &buf), &buf, iproj)));
		}
	}

	if (proj->props[PROJ_PROP_DEFINES].flags & PROP_SET) {
		const arr_t *defines = &proj->props[PROJ_PROP_DEFINES].arr;

		for (uint k = 0; k < defines->cnt; k++) {
			const prop_str_t *define = arr_get(defines, k);
			mk_pgen_add_define(gen, define->val, F_MK_INTDIR_OBJECT | F_MK_BUILD_STATIC | F_MK_INTDIR_SHARED);
		}
	}

	const prop_t *flags = &proj->props[PROJ_PROP_FLAGS];
	if ((flags->flags & PROP_SET)) {
		if (flags->mask & (1 << FLAG_STD_C99)) {
			mk_pgen_add_flag(gen, STR("-std=c99"), MK_EXT_C);
		}
		if (flags->mask & (1 << FLAG_FREESTANDING)) {
			mk_pgen_add_flag(gen, STR("-ffreestanding"), MK_EXT_C);
		}
	}

	mk_pgen_add_flag(gen, STR("-Wall -Wextra -Werror -pedantic"), F_MK_EXT_C | F_MK_EXT_CPP);

	const proj_dep_t *dep;
	arr_foreach(&proj->all_depends, dep)
	{
		if (dep->link_type != LINK_TYPE_SHARED || !(dep->proj->props[PROJ_PROP_SOURCE].flags & PROP_SET)) {
			continue;
		}

		//TODO: cleanup
		str_t define = strz(dep->proj->name.len + 5);
		str_to_upper(dep->proj->name, &define);
		str_cat(&define, STR("_DLL"));
		mk_pgen_add_define(gen, define, F_MK_INTDIR_OBJECT | F_MK_INTDIR_SHARED);
		str_free(&define);
	}

	//TODO: cleanup
	str_t define = strz(proj->name.len + 11);
	str_to_upper(proj->name, &define);
	str_cat(&define, STR("_BUILD_DLL"));
	mk_pgen_add_define(gen, define, F_MK_INTDIR_SHARED);
	str_free(&define);

	if (flags->flags & PROP_SET) {
		if (flags->mask & (1 << FLAG_WHOLEARCHIVE)) {
			mk_pgen_add_ldflag(gen, STR("-Wl,--whole-archive"));
		}
		if (flags->mask & (1 << FLAG_ALLOWMULTIPLEDEFINITION)) {
			mk_pgen_add_ldflag(gen, STR("-Wl,--allow-multiple-definition"));
		}
	}

	for (int i = proj->all_depends.cnt - 1; i >= 0; i--) {
		proj_dep_t *dep = arr_get(&proj->all_depends, i);

		str_t rel = strc(dep->proj->rel_dir.path, dep->proj->rel_dir.len);

		if (dep->proj->props[PROJ_PROP_TYPE].mask == PROJ_TYPE_LIB) {
			if (dep->proj->props[PROJ_PROP_SOURCE].flags & PROP_SET) {
				const prop_t *doutdir = &dep->proj->props[PROJ_PROP_OUTDIR];

				if (doutdir->flags & PROP_SET) {
					resolve(resolve_path(rel, doutdir->value.val, &buf), &buf, dep->proj);

					if (dep->link_type == LINK_TYPE_SHARED) {
						mk_pgen_add_lib(gen, str_cpy(buf), str_cpy(dep->proj->name), MK_LINK_SHARED, MK_LIB_INT);
						mk_pgen_add_copyfile(gen, str_cpy(resolve(strf("%.*s%.*s.so", doutdir->value.val.len, doutdir->value.val.data,
											       dep->proj->name.len, dep->proj->name.data),
											  &buf, dep->proj)));
					} else {
						mk_pgen_add_lib(gen, str_cpy(buf), str_cpy(dep->proj->name), MK_LINK_STATIC, MK_LIB_INT);
					}
				}
			} else {
				const prop_t *lib = &dep->proj->props[PROJ_PROP_LIB];
				if (dep->link_type == LINK_TYPE_STATIC && lib->flags & PROP_SET) {
					resolve(resolve_path(rel, STR(""), &buf), &buf, dep->proj);
					mk_pgen_add_lib(gen, str_cpy(buf), strs(lib->value.val), MK_LINK_STATIC, MK_LIB_EXT);
				}

				const prop_t *dlib = &dep->proj->props[PROJ_PROP_DLIB];
				if (dep->link_type == LINK_TYPE_SHARED && dlib->flags & PROP_SET) {
					resolve(resolve_path(rel, STR(""), &buf), &buf, dep->proj);
					mk_pgen_add_lib(gen, str_cpy(buf), str_cpy(dlib->value.val), MK_LINK_SHARED, MK_LIB_EXT);
				}
			}
		}

		const prop_str_t *libdir;
		arr_foreach(&dep->proj->props[PROJ_PROP_LIBDIRS].arr, libdir)
		{
			resolve(resolve_path(rel, libdir->val, &buf), &buf, dep->proj);
			mk_pgen_add_lib(gen, str_cpy(buf), str_null(), MK_LINK_SHARED, MK_LIB_EXT);
		}

		//TODO: Not needed anymore? Replaced with LIB
		const prop_str_t *link;
		arr_foreach(&dep->proj->props[PROJ_PROP_LINK].arr, link)
		{
			mk_pgen_add_lib(gen, str_null(), str_cpy(link->val), MK_LINK_SHARED, MK_LIB_EXT);
		}
	}

	arr_foreach(&proj->depends, dep)
	{
		if (!(dep->proj->props[PROJ_PROP_SOURCE].flags & PROP_SET)) {
			if (dep->link_type == LINK_TYPE_STATIC) {
				mk_pgen_add_depend(gen, strf("%.*s_s", dep->proj->name.len, dep->proj->name.data));
			} else {
				mk_pgen_add_depend(gen, strf("%.*s_d", dep->proj->name.len, dep->proj->name.data));
			}
		}
	}

	if (!(proj->props[PROJ_PROP_SOURCE].flags & PROP_SET)) {
		arr_foreach(&proj->depends, dep)
		{
			if (dep->link_type == LINK_TYPE_STATIC) {
				mk_pgen_add_depend(gen, strf("%.*s_s", dep->proj->name.len, dep->proj->name.data));
			} else {
				mk_pgen_add_depend(gen, strf("%.*s_d", dep->proj->name.len, dep->proj->name.data));
			}
		}
	}

	if ((flags->flags & PROP_SET) && (flags->mask & (1 << FLAG_WHOLEARCHIVE))) {
		mk_pgen_add_ldflag(gen, STR("-Wl,--no-whole-archive"));
	}

	if (lang & (1 << LANG_CPP)) {
		mk_pgen_add_ldflag(gen, STR("-lstdc++"));
	}

	//TODO: Think of better solution
	if (flags->flags & PROP_SET) {
		if (flags->mask & (1 << FLAG_STATIC)) {
			mk_pgen_add_ldflag(gen, STR("-static"));
		}
		if (flags->mask & (1 << FLAG_NOSTARTFILES)) {
			mk_pgen_add_ldflag(gen, STR("-nostartfiles"));
		}
		if (flags->mask & (1 << FLAG_MATH)) {
			mk_pgen_add_ldflag(gen, STR("-lm"));
		}

		if (flags->mask & (1 << FLAG_X11)) {
			mk_pgen_add_ldflag(gen, STR("-lX11"));
		}

		if (flags->mask & (1 << FLAG_GL)) {
			mk_pgen_add_ldflag(gen, STR("-lGL"));
		}

		if (flags->mask & (1 << FLAG_GLX)) {
			mk_pgen_add_ldflag(gen, STR("-lGLX"));
		}
	}

	const prop_str_t *require;
	arr_foreach(&proj->props[PROJ_PROP_REQUIRE].arr, require)
	{
		mk_pgen_add_require(gen, str_cpy(require->val));
	}

	str_t rel = strc(proj->rel_dir.path, proj->rel_dir.len);

	const prop_t *lib = &proj->props[PROJ_PROP_LIB];
	if (lib->flags & PROP_SET) {
		resolve(resolve_path(rel, lib->value.val, &buf), &buf, proj);
		gen->lib[MK_INTDIR_STATIC] = strf("%.*s.a", buf.len, buf.data);
	}

	const prop_t *dlib = &proj->props[PROJ_PROP_DLIB];
	if (dlib->flags & PROP_SET) {
		resolve(resolve_path(rel, dlib->value.val, &buf), &buf, proj);
		gen->lib[MK_INTDIR_SHARED] = strf("%.*s.so", buf.len, buf.data);
	}

	const prop_str_t *copyfile;
	arr_foreach(&proj->props[PROJ_PROP_COPYFILES].arr, copyfile)
	{
		mk_pgen_add_copyfile(gen, str_cpy(resolve(copyfile->val, &buf, proj)));
	}

	cm_pgen(gen, PRINT_DST_FILE(file));

	mk_pgen_free(&lgen);
}

int cm_proj_gen(const proj_t *proj, const dict_t *projects, const prop_t *sln_props)
{
	path_t gen_path = { 0 };
	if (path_init(&gen_path, proj->dir.path, proj->dir.len) == NULL) {
		return 1;
	}

	if (!folder_exists(gen_path.path)) {
		folder_create(gen_path.path);
	}

	if (path_child(&gen_path, CSTR("CMakeLists.txt")) == NULL) {
		return 1;
	}

	FILE *file = file_open(gen_path.path, "w");
	if (file == NULL) {
		return 1;
	}

	int ret = 0;

	MSG("generating project: %s", gen_path.path);

	cm_proj_gen_proj2(proj, projects, sln_props, file, 0);

	file_close(file);
	if (ret == 0) {
		SUC("generating project: %s success", gen_path.path);
	} else {
		ERR("generating project: %s failed", gen_path.path);
	}

	return ret;
}
//467