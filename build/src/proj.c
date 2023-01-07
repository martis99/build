#include "proj.h"

#include "dir.h"
#include "prop.h"
#include "utils.h"

#include "defines.h"
#include "md5.h"
#include "mem.h"
#include "xml.h"

#include <string.h>

#include <Windows.h>

static const prop_pol_t s_proj_props[] = {
	[PROJ_PROP_NAME]     = { .name = "NAME", .parse = prop_parse_word },
	[PROJ_PROP_TYPE]     = { .name = "TYPE", .parse = prop_parse_word, .str_table = s_proj_types, .str_table_len = __PROJ_TYPE_MAX },
	[PROJ_PROP_SOURCE]   = { .name = "SOURCE", .parse = prop_parse_path },
	[PROJ_PROP_INCLUDE]  = { .name = "INCLUDE", .parse = prop_parse_path },
	[PROJ_PROP_DEPENDS]  = { .name = "DEPENDS", .parse = prop_parse_word, .dim = PROP_DIM_ARRAY },
	[PROJ_PROP_INCLUDES] = { .name = "INCLUDES", .parse = prop_parse_word, .dim = PROP_DIM_ARRAY },
	[PROJ_PROP_DEFINES]  = { .name = "DEFINES", .parse = prop_parse_word, .dim = PROP_DIM_ARRAY },
	[PROJ_PROP_LIBDIRS]  = { .name = "LIBDIRS", .parse = prop_parse_path, .dim = PROP_DIM_ARRAY },
	[PROJ_PROP_WDIR]     = { .name = "WDIR", .parse = prop_parse_path },
	[PROJ_PROP_CHARSET]  = { .name = "CHARSET", .parse = prop_parse_word, .str_table = s_charsets, .str_table_len = __CHARSET_MAX },
	[PROJ_PROP_OUTDIR]   = { .name = "OUTDIR", .parse = prop_parse_path },
	[PROJ_PROP_INTDIR]   = { .name = "INTDIR", .parse = prop_parse_path },
};

typedef struct var_pol_s {
	const char *names[64];
	const char *tos[64];
} var_pol_t;

typedef enum var_e {
	VAR_SLN_DIR,
	VAR_SLN_NAME,
	VAR_PROJ_DIR,
	VAR_PROJ_NAME,
	VAR_CONFIG,
	VAR_PLATFORM,

	__VAR_MAX,
} var_t;

static const var_pol_t vs_vars = {
	.names = {
		[VAR_SLN_DIR] = "$(SLN_DIR)\\",
		[VAR_SLN_NAME] = "$(SLN_NAME)",
		[VAR_PROJ_DIR] = "$(PROJ_DIR)\\",
		[VAR_PROJ_NAME] = "$(PROJ_NAME)",
		[VAR_CONFIG] = "$(CONFIG)",
		[VAR_PLATFORM] = "$(PLATFORM)",
	},
	.tos = {
		[VAR_SLN_DIR] = "$(SolutionDir)",
		[VAR_SLN_NAME] = "$(SolutionName)",
		[VAR_PROJ_DIR] = "$(ProjectDir)",
		[VAR_PROJ_NAME] = "$(ProjectName)",
		[VAR_CONFIG] = "$(Configuration)",
		[VAR_PLATFORM] = "$(PlatformTarget)",
	},
};

int proj_read(proj_t *proj, const path_t *sln_path, const path_t *path, const struct dir_s *parent)
{
	proj->file_path = *path;
	pathv_path(&proj->path, &proj->file_path);
	pathv_sub(&proj->rel_path, &proj->file_path, sln_path);
	pathv_folder(&proj->folder, &proj->file_path);

	proj->dir = (pathv_t){
		.len  = (proj->rel_path.len < (proj->folder.len + 1)) ? 0 : (proj->rel_path.len - proj->folder.len - 1),
		.path = proj->rel_path.path,
	};

	proj->parent = parent;

	if (path_child(&proj->file_path, "Project.txt", 11)) {
		return 1;
	}

	if ((proj->data.len = (unsigned int)file_read(proj->file_path.path, 1, proj->file, DATA_LEN)) == -1) {
		return 1;
	}

	proj->data.path = proj->file_path.path;
	proj->data.data = proj->file;
	proj->data.cur	= 0;

	int ret = props_parse_file(&proj->data, proj->props, s_proj_props, sizeof(s_proj_props));

	path_t rel_path_name = { 0 };
	path_init(&rel_path_name, proj->rel_path.path, proj->rel_path.len);
	path_child(&rel_path_name, proj->props[PROJ_PROP_NAME].value.data, proj->props[PROJ_PROP_NAME].value.len);
	path_child_s(&rel_path_name, "vcxproj", 7, '.');
	unsigned char buf[256] = { 0 };
	md5(rel_path_name.path, rel_path_name.len, buf, sizeof(buf), proj->guid, sizeof(proj->guid));

	if (proj->props[PROJ_PROP_SOURCE].set) {
		prop_str_t *source = &proj->props[PROJ_PROP_SOURCE].value;
		path_t path	   = { 0 };
		path_init(&path, proj->path.path, proj->path.len);
		if (path_child(&path, source->data, source->len)) {
			ret++;
		} else {
			if (!folder_exists(path.path)) {
				ERR_LOGICS("source folder does not exists '%.*s'", source->path, source->line + 1, source->start - source->line_start + 1, source->len,
					   source->data);
				ret++;
			}
		}
	}

	if (proj->props[PROJ_PROP_INCLUDE].set) {
		prop_str_t *include = &proj->props[PROJ_PROP_INCLUDE].value;
		path_t path	    = { 0 };
		path_init(&path, proj->path.path, proj->path.len);
		if (path_child(&path, include->data, include->len)) {
			ret++;
		} else {
			if (!folder_exists(path.path)) {
				ERR_LOGICS("include folder does not exists '%.*s'", include->path, include->line + 1, include->start - include->line_start + 1,
					   include->len, include->data);
				ret++;
			}
		}
	}

	return ret;
}

void proj_print(proj_t *proj)
{
	INFP("Project\n"
	     "    Path   : %.*s\n"
	     "    File   : %.*s\n"
	     "    Rel    : %.*s\n"
	     "    Dir    : %.*s\n"
	     "    Folder : %.*s\n"
	     "    GUID   : %s",
	     proj->path.len, proj->path.path, proj->file_path.len, proj->file_path.path, proj->rel_path.len, proj->rel_path.path, proj->dir.len, proj->dir.path,
	     proj->folder.len, proj->folder.path, proj->guid);

	if (proj->parent) {
		INFP("    Parent : %.*s", (unsigned int)proj->parent->folder.len, proj->parent->folder.path);
	} else {
		INFP("    Parent :");
	}

	props_print(proj->props, s_proj_props, sizeof(s_proj_props));

	INFP("    Depends:");
	for (int i = 0; i < proj->all_depends.count; i++) {
		prop_str_t **val = array_get(&proj->all_depends, i);
		INFP("        '%.*s'", (*val)->len, (*val)->data);
	}
	INFF();
}

static void convert_slash(char *dst, unsigned int dst_len, const char *src, size_t len)
{
	memcpy_s(dst, dst_len, src, len);
	for (int i = 0; i < len; i++) {
		if (dst[i] == '\\') {
			dst[i] = '/';
		}
	}
}

static inline void print_rel_path(FILE *fp, proj_t *proj, const char *path, unsigned int path_len)
{
	char path_b[MAX_PATH] = { 0 };
	convert_slash(path_b, sizeof(path_b) - 1, path, path_len);
	char rel_path[MAX_PATH] = { 0 };
	convert_slash(rel_path, sizeof(rel_path) - 1, proj->rel_path.path, proj->rel_path.len);
	fprintf_s(fp, "${CMAKE_SOURCE_DIR}/%.*s/%.*s", proj->rel_path.len, rel_path, path_len, path_b);
}

int proj_gen_cmake(const proj_t *proj, const hashmap_t *projects, const path_t *path, int lang, charset_t charset)
{
	const prop_str_t *name = &proj->props[PROJ_PROP_NAME].value;
	proj_type_t type       = proj->props[PROJ_PROP_TYPE].mask;

	charset = proj->props[PROJ_PROP_CHARSET].set ? proj->props[PROJ_PROP_CHARSET].mask : charset;

	char source[MAX_PATH]  = { 0 };
	char include[MAX_PATH] = { 0 };
	int source_len	       = 0;
	int include_len	       = 0;
	int include_diff       = 0;

	if (proj->props[PROJ_PROP_SOURCE].set) {
		source_len = proj->props[PROJ_PROP_SOURCE].value.len;
		convert_slash(source, sizeof(source) - 1, proj->props[PROJ_PROP_SOURCE].value.data, source_len);
	}

	if (proj->props[PROJ_PROP_INCLUDE].set) {
		include_len = proj->props[PROJ_PROP_INCLUDE].value.len;
		convert_slash(include, sizeof(include) - 1, proj->props[PROJ_PROP_INCLUDE].value.data, include_len);

		include_diff = !proj->props[PROJ_PROP_SOURCE].set || !(source_len == include_len && memcmp(source, include, source_len) == 0);
	}

	int ret = 0;

	path_t cmake_path = *path;
	if (path_child(&cmake_path, proj->rel_path.path, proj->rel_path.len)) {
		return 1;
	}

	if (!folder_exists(cmake_path.path)) {
		folder_create(cmake_path.path);
	}

	if (path_child(&cmake_path, "CMakeLists.txt", 14)) {
		return 1;
	}

	FILE *fp = file_open(cmake_path.path, "w", 1);
	if (fp == NULL) {
		return 1;
	}

	MSG("generating project: %s", cmake_path.path);

	if (proj->props[PROJ_PROP_SOURCE].set || proj->props[PROJ_PROP_INCLUDE].set) {
		fprintf_s(fp, "file(GLOB_RECURSE %.*s_SOURCE", name->len, name->data);
		if (proj->props[PROJ_PROP_SOURCE].set) {
			fprintf_s(fp, " %.*s/*.c %.*s/*.h", source_len, source, source_len, source);
		}
		if (include_diff) {
			fprintf_s(fp, " %.*s/*.h", include_len, include);
		}
		fprintf_s(fp, ")\n\n");
	}

	if (proj->props[PROJ_PROP_DEFINES].set || charset == CHARSET_UNICODE) {
		const array_t *defines = &proj->props[PROJ_PROP_DEFINES].arr;

		if (defines->count > 0 || charset == CHARSET_UNICODE) {
			fprintf_s(fp, "add_definitions(");
		}

		int first = 1;

		if (charset == CHARSET_UNICODE) {
			fprintf_s(fp, first ? "-DUNICODE -D_UNICODE" : " -DUNICODE -D_UNICODE");
			first = 0;
		}

		for (int i = 0; i < defines->count; i++) {
			const prop_str_t *define = array_get(defines, i);
			fprintf_s(fp, first ? "-D%.*s" : " -D%.*s", define->len, define->data);
			first = 0;
		}

		if (!first) {
			fprintf_s(fp, ")\n\n");
		}
	}

	switch (type) {
	case PROJ_TYPE_LIB:
		fprintf_s(fp, "add_library(%.*s STATIC ${%.*s_SOURCE})\n", name->len, name->data, name->len, name->data);
		break;
	case PROJ_TYPE_EXE: {
		int first = 0;
		for (int i = 0; i < proj->all_depends.count; i++) {
			prop_str_t **depend = array_get(&proj->all_depends, i);
			proj_t *dproj	    = NULL;
			if(hashmap_get(projects, (*depend)->data, (*depend)->len, &dproj)) {
				ERR("project doesn't exists: '%.*s'", (*depend)->len, (*depend)->data);
				continue;
			}

			if (dproj->props[PROJ_PROP_LIBDIRS].set && dproj->props[PROJ_PROP_LIBDIRS].arr.count > 0) {
				first = 1;
				break;
			}
		}

		if (first) {
			fprintf_s(fp, "link_directories(");
			first = 1;
			for (int i = 0; i < proj->all_depends.count; i++) {
				prop_str_t **depend = array_get(&proj->all_depends, i);
				proj_t *dproj	    = NULL;
				if(hashmap_get(projects, (*depend)->data, (*depend)->len, &dproj)) {
					ERR("project doesn't exists: '%.*s'", (*depend)->len, (*depend)->data);
					continue;
				}

				if (dproj->props[PROJ_PROP_LIBDIRS].set) {
					array_t *libdirs = &dproj->props[PROJ_PROP_LIBDIRS].arr;
					for (int j = 0; j < libdirs->count; j++) {
						prop_str_t *libdir = array_get(libdirs, j);
						if (libdir->len > 0) {
							if (!first) {
								fprintf_s(fp, " ");
							}
							print_rel_path(fp, dproj, libdir->data, libdir->len);
							first = 0;
						}
					}
					break;
				}
			}

			fprintf_s(fp, ")\n");
		}

		fprintf_s(fp, "add_executable(%.*s ${%.*s_SOURCE})\n", name->len, name->data, name->len, name->data);

		fprintf_s(fp, "target_link_libraries(%.*s", name->len, name->data);

		for (int i = 0; i < proj->all_depends.count; i++) {
			prop_str_t **depend = array_get(&proj->all_depends, i);
			fprintf_s(fp, " %.*s", (*depend)->len, (*depend)->data);
		}

		fprintf_s(fp, ")\n");

		break;
	}
	case PROJ_TYPE_EXT:
		fprintf_s(fp, "add_library(%.*s STATIC ${%.*s_SOURCE})\n", name->len, name->data, name->len, name->data);
		break;
	default:
		ERR("unknown project type: %d", type);
		ret = 1;
	}

	int first = 1;
	fprintf_s(fp, "include_directories(");
	if (proj->props[PROJ_PROP_SOURCE].set) {
		fprintf_s(fp, "%.*s", source_len, source);
		first = 0;
	}
	if (include_diff) {
		fprintf_s(fp, first ? "%.*s" : " %.*s", include_len, include);
		first = 0;
	}

	if (proj->props[PROJ_PROP_INCLUDES].set) {
		const array_t *includes = &proj->props[PROJ_PROP_INCLUDES].arr;
		for (int i = 0; i < includes->count; i++) {
			prop_str_t *inc_str = array_get(includes, i);
			proj_t *inc_proj    = NULL;
			if (hashmap_get(projects, inc_str->data, inc_str->len, &inc_proj)) {
				ERR_LOGICS("project '%.*s' doesn't exists", inc_str->path, inc_str->line + 1, inc_str->start - inc_str->line_start + 1, inc_str->len,
					   inc_str->data);
				ret++;
				continue;
			}

			if (inc_proj->props[PROJ_PROP_INCLUDE].set) {
				if (!first) {
					fprintf_s(fp, " ");
				}
				print_rel_path(fp, inc_proj, inc_proj->props[PROJ_PROP_INCLUDE].value.data, inc_proj->props[PROJ_PROP_INCLUDE].value.len);
				first = 0;
			}
		}
	}
	fprintf_s(fp, ")\n");

	if (!proj->props[PROJ_PROP_SOURCE].set) {
		if (lang & LANG_C) {
			fprintf_s(fp, "set_target_properties(%.*s PROPERTIES LINKER_LANGUAGE C)\n", name->len, name->data);
		}
	}

	if (proj->dir.len > 0) {
		char buf[MAX_PATH] = { 0 };
		convert_slash(buf, sizeof(buf) - 1, proj->dir.path, proj->dir.len);
		fprintf_s(fp, "set_target_properties(%.*s PROPERTIES FOLDER \"%.*s\")\n", name->len, name->data, (unsigned int)proj->dir.len, buf);
	}

	if (proj->props[PROJ_PROP_WDIR].set) {
		const prop_str_t *wdir = &proj->props[PROJ_PROP_WDIR].value;
		char wdir_b[MAX_PATH]  = { 0 };
		convert_slash(wdir_b, sizeof(wdir_b) - 1, wdir->data, wdir->len);
		if (wdir->len > 0) {
			fprintf_s(fp, "set_property(TARGET %.*s PROPERTY VS_DEBUGGER_WORKING_DIRECTORY \"${CMAKE_SOURCE_DIR}/%.*s\")\n", name->len, name->data, wdir->len,
				  wdir_b);
		}
	}

	fclose(fp);
	if (ret == 0) {
		SUC("generating project: %s success", cmake_path.path);
	} else {
		ERR("generating project: %s failed", cmake_path.path);
	}

	return ret;
}

int proj_gen_make(const proj_t *proj, const hashmap_t *projects, const path_t *path)
{
	const prop_str_t *name = &proj->props[PROJ_PROP_NAME].value;
	proj_type_t type       = proj->props[PROJ_PROP_TYPE].mask;

	int ret = 0;

	path_t cmake_path = *path;
	if (path_child(&cmake_path, proj->rel_path.path, proj->rel_path.len)) {
		return 1;
	}

	if (!folder_exists(cmake_path.path)) {
		folder_create(cmake_path.path);
	}

	if (path_child(&cmake_path, "Makefile", 8)) {
		return 1;
	}

	FILE *fp = file_open(cmake_path.path, "w", 1);
	if (fp == NULL) {
		return 1;
	}

	MSG("generating project: %s", cmake_path.path);

	switch (type) {
	case PROJ_TYPE_EXE:
		fprintf_s(fp, "BIN:=%.*s\n", name->len, name->data);
		break;
	default:
		fprintf_s(fp, "BIN:=%.*s.a\n", name->len, name->data);
		break;
	}

	fprintf_s(fp, "SRC:=$(shell find -name '*.c')\n"
		      "OBJS:=$(SRC:.c=.o)\n");

	if (proj->all_depends.count > 0) {
		fprintf_s(fp, "DEPENDS =");

		for (int i = 0; i < proj->all_depends.count; i++) {
			prop_str_t **depend = array_get(&proj->all_depends, i);
			proj_t *dproj	    = NULL;
			if (hashmap_get(projects, (*depend)->data, (*depend)->len, &dproj)) {
				ERR("project doesn't exists: '%.*s'", (*depend)->len, (*depend)->data);
				continue;
			}
			char buf[MAX_PATH] = { 0 };
			convert_slash(buf, sizeof(buf) - 1, dproj->rel_path.path, dproj->rel_path.len);
			fprintf_s(fp, " $(SLNDIR)/%.*s", dproj->rel_path.len, buf);
		}
	}

	fprintf_s(fp, "\n"
		      "\n"
		      ".PHONY: all clean depends");

	if (proj->all_depends.count > 0) {
		fprintf_s(fp, " $(DEPENDS)");
	}

	fprintf_s(fp, "\n"
		      "\n"
		      "all: $(BIN)");

	if (proj->all_depends.count > 0) {
		fprintf_s(fp, " $(DEPENDS)");
	}

	fprintf_s(fp, "\n"
		      "\n"
		      "$(BIN): $(OBJS)\n"
		      "\tar rcs $@ $^\n"
		      "\n"
		      "%%.o: %%.c\n"
		      "\t$(CC)");

	int include_diff = proj->props[PROJ_PROP_INCLUDE].set &&
			   (!proj->props[PROJ_PROP_SOURCE].set ||
			    !(proj->props[PROJ_PROP_INCLUDE].value.len == proj->props[PROJ_PROP_SOURCE].value.len &&
			      memcmp(proj->props[PROJ_PROP_SOURCE].value.data, proj->props[PROJ_PROP_INCLUDE].value.data, proj->props[PROJ_PROP_SOURCE].value.len) == 0));

	if (proj->props[PROJ_PROP_SOURCE].set) {
		char buf[MAX_PATH] = { 0 };
		convert_slash(buf, sizeof(buf) - 1, proj->props[PROJ_PROP_SOURCE].value.data, proj->props[PROJ_PROP_SOURCE].value.len);
		fprintf_s(fp, " -I%.*s", proj->props[PROJ_PROP_SOURCE].value.len, buf);
	}

	if (include_diff) {
		char buf[MAX_PATH] = { 0 };
		convert_slash(buf, sizeof(buf) - 1, proj->props[PROJ_PROP_INCLUDE].value.data, proj->props[PROJ_PROP_INCLUDE].value.len);
		fprintf_s(fp, " -I%.*s", proj->props[PROJ_PROP_INCLUDE].value.len, buf);
	}

	if (proj->props[PROJ_PROP_INCLUDES].set) {
		const array_t *includes = &proj->props[PROJ_PROP_INCLUDES].arr;

		for (int k = 0; k < includes->count; k++) {
			prop_str_t *include = array_get(includes, k);
			proj_t *iproj	    = { 0 };
			if (hashmap_get(projects, include->data, include->len, &iproj)) {
				ERR("project doesn't exists: '%.*s'", include->len, include->data);
				continue;
			}
			char buf[MAX_PATH] = { 0 };
			convert_slash(buf, sizeof(buf) - 1, iproj->rel_path.path, iproj->rel_path.len);
			fprintf_s(fp, " -I$(SLNDIR)/%.*s/%.*s", iproj->rel_path.len, buf, iproj->props[PROJ_PROP_INCLUDE].value.len,
				  iproj->props[PROJ_PROP_INCLUDE].value.data);
		}
	}

	fprintf_s(fp, " -c -o $@ $^\n"
		      "\n");

	if (proj->all_depends.count > 0) {
		fprintf_s(fp, "$(DEPENDS):\n"
			      "\t$(MAKE) -C $@\n"
			      "\n");
	}

	fprintf_s(fp, "clean:\n"
		      "\t$(RM) $(BIN) $(OBJS)\n");

	fclose(fp);
	if (ret == 0) {
		SUC("generating project: %s success", cmake_path.path);
	} else {
		ERR("generating project: %s failed", cmake_path.path);
	}

	return ret;
}

typedef struct add_file_s {
	path_t path;
	xml_tag_t *xml_items;
} add_file_t;

static int add_src_file(path_t *path, const char *folder, void *usr)
{
	add_file_t *data = usr;

	path_t new_path = data->path;
	path_child(&new_path, folder, cstr_len(folder));

	if (!path_ends(&new_path, ".c")) {
		return 0;
	}

	xml_add_attr_c(xml_add_child(data->xml_items, "ClCompile", 9), "Include", 7, new_path.path, new_path.len);

	return 0;
}

static int add_src_folder(path_t *path, const char *folder, void *usr)
{
	add_file_t *data = usr;

	add_file_t newdata = {
		.path	   = data->path,
		.xml_items = data->xml_items,
	};

	path_child(&newdata.path, folder, cstr_len(folder));

	files_foreach(path, add_src_folder, add_src_file, &newdata);

	return 0;
}

static int add_inc_file(path_t *path, const char *folder, void *usr)
{
	add_file_t *data = usr;

	path_t new_path = data->path;
	path_child(&new_path, folder, cstr_len(folder));

	if (!path_ends(&new_path, ".h")) {
		return 0;
	}

	xml_add_attr_c(xml_add_child(data->xml_items, "ClInclude", 9), "Include", 7, new_path.path, new_path.len);

	return 0;
}

static int add_inc_folder(path_t *path, const char *folder, void *usr)
{
	add_file_t *data = usr;

	add_file_t newdata = {
		.path	   = data->path,
		.xml_items = data->xml_items,
	};

	path_child(&newdata.path, folder, cstr_len(folder));

	files_foreach(path, add_inc_folder, add_inc_file, &newdata);

	return 0;
}

static inline int print_includes(char *buf, unsigned int buf_size, const proj_t *proj, const hashmap_t *projects)
{
	unsigned int len = 0;
	int first	 = 1;

	const prop_str_t *src = &proj->props[PROJ_PROP_SOURCE].value;
	const prop_str_t *inc = &proj->props[PROJ_PROP_INCLUDE].value;

	if (proj->props[PROJ_PROP_SOURCE].set) {
		len += snprintf(buf == NULL ? buf : buf + len, buf_size, "$(ProjectDir)%.*s", src->len, src->data);
		first = 0;
	}

	if (proj->props[PROJ_PROP_INCLUDE].set && (!proj->props[PROJ_PROP_SOURCE].set || !cstr_cmp(src->data, src->len, inc->data, inc->len))) {
		len += snprintf(buf == NULL ? buf : buf + len, buf_size, first ? "$(ProjectDir)%.*s" : ";$(ProjectDir)%.*s", proj->props[PROJ_PROP_INCLUDE].value.len,
				proj->props[PROJ_PROP_INCLUDE].value.data);
		first = 0;
	}

	if (proj->props[PROJ_PROP_INCLUDES].set) {
		const array_t *depends = &proj->props[PROJ_PROP_INCLUDES].arr;

		for (int k = 0; k < depends->count; k++) {
			prop_str_t *depend = array_get(depends, k);
			proj_t *dproj	   = NULL;
			if (hashmap_get(projects, depend->data, depend->len, &dproj)) {
				ERR("project doesn't exists: '%.*s'", depend->len, depend->data);
				continue;
			}

			len += snprintf(buf == NULL ? buf : buf + len, buf_size, first ? "$(SolutionDir)%.*s\\%.*s" : ";$(SolutionDir)%.*s\\%.*s", dproj->rel_path.len,
					dproj->rel_path.path, dproj->props[PROJ_PROP_INCLUDE].value.len, dproj->props[PROJ_PROP_INCLUDE].value.data);
			first = 0;
		}
	}

	return len;
}

static inline int print_defines(char *buf, unsigned int buf_size, const proj_t *proj, const hashmap_t *projects)
{
	unsigned int len = 0;
	int first	 = 1;

	if (proj->props[PROJ_PROP_DEFINES].set) {
		const array_t *defines = &proj->props[PROJ_PROP_DEFINES].arr;

		for (int k = 0; k < defines->count; k++) {
			prop_str_t *define = array_get(defines, k);

			len += snprintf(buf == NULL ? buf : buf + len, buf_size, first ? "%.*s" : ";%.*s", define->len, define->data);
			first = 0;
		}
	}

	return len;
}

//TODO: Make proj const
int proj_gen_vs(proj_t *proj, const hashmap_t *projects, const path_t *path, const array_t *configs, const array_t *platforms, const prop_t *charset,
		const prop_t *outdir, const prop_t *intdir)
{
	const prop_str_t *name = &proj->props[PROJ_PROP_NAME].value;
	proj_type_t type       = proj->props[PROJ_PROP_TYPE].mask;

	charset = proj->props[PROJ_PROP_CHARSET].set ? &proj->props[PROJ_PROP_CHARSET] : charset;
	outdir	= proj->props[PROJ_PROP_OUTDIR].set ? &proj->props[PROJ_PROP_OUTDIR] : outdir;
	intdir	= proj->props[PROJ_PROP_INTDIR].set ? &proj->props[PROJ_PROP_INTDIR] : intdir;

	if (charset->mask == CHARSET_UNICODE) {
		if (!proj->props[PROJ_PROP_DEFINES].set) {
			array_init(&proj->props[PROJ_PROP_DEFINES].arr, 2, sizeof(prop_str_t));
			proj->props[PROJ_PROP_DEFINES].set = 1;
		}

		prop_str_t unicode = {
			.data = "UNICODE",
			.len  = 7,
		};
		array_add(&proj->props[PROJ_PROP_DEFINES].arr, &unicode);
		prop_str_t unicode2 = {
			.data = "_UNICODE",
			.len  = 8,
		};
		array_add(&proj->props[PROJ_PROP_DEFINES].arr, &unicode2);
	}

	int ret = 0;

	xml_t xml = { 0 };
	xml_init(&xml);
	xml_tag_t *xml_proj = xml_add_child(&xml.root, "Project", 7);
	xml_add_attr(xml_proj, "DefaultTargets", 14, "Build", 5);
	xml_add_attr(xml_proj, "xmlns", 5, "http://schemas.microsoft.com/developer/msbuild/2003", 51);
	xml_tag_t *xml_proj_confs = xml_add_child(xml_proj, "ItemGroup", 9);
	xml_add_attr(xml_proj_confs, "Label", 5, "ProjectConfigurations", 21);

	for (int i = platforms->count - 1; i >= 0; i--) {
		prop_str_t *platform   = array_get(platforms, i);
		const char *platf      = platform->data;
		unsigned int platf_len = platform->len;
		if (platform->len == 3 && strncmp(platform->data, "x86", 3) == 0) {
			platf	  = "Win32";
			platf_len = 5;
		}
		for (int j = 0; j < configs->count; j++) {
			prop_str_t *config = array_get(configs, j);

			xml_tag_t *xml_proj_conf = xml_add_child(xml_proj_confs, "ProjectConfiguration", 20);
			xml_add_attr_f(xml_proj_conf, "Include", 7, "%.*s|%.*s", config->len, config->data, platf_len, platf);
			xml_add_child_val(xml_proj_conf, "Configuration", 13, config->data, config->len);
			xml_add_child_val(xml_proj_conf, "Platform", 8, platf, platf_len);
		}
	}

	xml_tag_t *xml_globals = xml_add_child(xml_proj, "PropertyGroup", 13);
	xml_add_attr(xml_globals, "Label", 5, "Globals", 7);
	xml_add_child_val(xml_globals, "VCProjectVersion", 16, "16.0", 4);
	xml_add_child_val(xml_globals, "Keyword", 7, "Win32Proj", 9);
	xml_add_child_val_f(xml_globals, "ProjectGuid", 11, "{%s}", proj->guid);
	xml_add_child_val(xml_globals, "RootNamespace", 13, proj->folder.path, proj->folder.len);
	xml_add_child_val(xml_globals, "WindowsTargetPlatformVersion", 28, "10.0", 4);

	xml_tag_t *xml_import = xml_add_child(xml_proj, "Import", 6);
	xml_add_attr(xml_import, "Project", 7, "$(VCTargetsPath)\\Microsoft.Cpp.Default.props", 44);

	const struct {
		const char *name;
		unsigned int len;
	} config_types[] = {
		[PROJ_TYPE_UNKNOWN] = { "", 0 },
		[PROJ_TYPE_LIB]	    = { "StaticLibrary", 13 },
		[PROJ_TYPE_EXE]	    = { "Application", 11 },
		[PROJ_TYPE_EXT]	    = { "StaticLibrary", 13 },
	};

	const struct {
		const char *name;
		unsigned int len;
	} charsets[] = {
		[CHARSET_UNKNOWN]    = { "", 0 },
		[CHARSET_UNICODE]    = { "Unicode", 7 },
		[CHARSET_MULTI_BYTE] = { "MultiByte", 9 },
	};

	for (int i = platforms->count - 1; i >= 0; i--) {
		prop_str_t *platform   = array_get(platforms, i);
		const char *platf      = platform->data;
		unsigned int platf_len = platform->len;
		if (platform->len == 3 && strncmp(platform->data, "x86", 3) == 0) {
			platf	  = "Win32";
			platf_len = 5;
		}
		for (int j = 0; j < configs->count; j++) {
			prop_str_t *config = array_get(configs, j);
			int debug	   = 0;
			if (config->len == 5 && strncmp(config->data, "Debug", 5) == 0) {
				debug = 1;
			}

			int release = 0;
			if (config->len == 7 && strncmp(config->data, "Release", 7) == 0) {
				release = 1;
			}

			xml_tag_t *xml_conf = xml_add_child(xml_proj, "PropertyGroup", 13);
			xml_add_attr_f(xml_conf, "Condition", 9, "'$(Configuration)|$(Platform)'=='%.*s|%.*s'", config->len, config->data, platf_len, platf);
			xml_add_attr(xml_conf, "Label", 5, "Configuration", 13);
			xml_add_child_val(xml_conf, "ConfigurationType", 17, config_types[proj->props[PROJ_PROP_TYPE].mask].name,
					  config_types[proj->props[PROJ_PROP_TYPE].mask].len);
			xml_add_child_val(xml_conf, "UseDebugLibraries", 17, debug ? "true" : "false", debug ? 4 : 5);
			xml_add_child_val(xml_conf, "PlatformToolset", 15, "v143", 4);

			if (release) {
				xml_add_child_val(xml_conf, "WholeProgramOptimization", 24, "true", 4);
			}

			if (charset->set && charset->mask != CHARSET_UNKNOWN) {
				xml_add_child_val(xml_conf, "CharacterSet", 12, charsets[charset->mask].name, charsets[charset->mask].len);
			}
		}
	}

	xml_add_attr(xml_add_child(xml_proj, "Import", 6), "Project", 7, "$(VCTargetsPath)\\Microsoft.Cpp.props", 36);
	xml_add_attr(xml_add_child_val(xml_proj, "ImportGroup", 11, "\n", 0), "Label", 5, "ExtensionSettings", 17);
	xml_add_attr(xml_add_child_val(xml_proj, "ImportGroup", 11, "\n", 0), "Label", 5, "Shared", 6);

	for (int i = platforms->count - 1; i >= 0; i--) {
		prop_str_t *platform   = array_get(platforms, i);
		const char *platf      = platform->data;
		unsigned int platf_len = platform->len;
		if (platform->len == 3 && strncmp(platform->data, "x86", 3) == 0) {
			platf	  = "Win32";
			platf_len = 5;
		}
		for (int j = 0; j < configs->count; j++) {
			prop_str_t *config = array_get(configs, j);

			xml_tag_t *xml_sheets = xml_add_child(xml_proj, "ImportGroup", 11);
			xml_add_attr(xml_sheets, "Label", 5, "PropertySheets", 14);
			xml_add_attr_f(xml_sheets, "Condition", 9, "'$(Configuration)|$(Platform)'=='%.*s|%.*s'", config->len, config->data, platf_len, platf);
			xml_tag_t *xml_data = xml_add_child(xml_sheets, "Import", 6);
			xml_add_attr(xml_data, "Project", 7, "$(UserRootDir)\\Microsoft.Cpp.$(Platform).user.props", 51);
			xml_add_attr(xml_data, "Condition", 9, "exists('$(UserRootDir)\\Microsoft.Cpp.$(Platform).user.props')", 61);
			xml_add_attr(xml_data, "Label", 5, "LocalAppDataPlatform", 20);
		}
	}

	xml_tag_t *xml_macros = xml_add_child(xml_proj, "PropertyGroup", 13);
	xml_add_attr(xml_macros, "Label", 5, "UserMacros", 10);

	for (int i = platforms->count - 1; i >= 0; i--) {
		prop_str_t *platform   = array_get(platforms, i);
		const char *platf      = platform->data;
		unsigned int platf_len = platform->len;
		if (platform->len == 3 && strncmp(platform->data, "x86", 3) == 0) {
			platf	  = "Win32";
			platf_len = 5;
		}
		for (int j = 0; j < configs->count; j++) {
			prop_str_t *config = array_get(configs, j);

			int debug = 0;
			if (config->len == 5 && strncmp(config->data, "Debug", 5) == 0) {
				debug = 1;
			}

			xml_tag_t *xml_plat = xml_add_child(xml_proj, "PropertyGroup", 13);
			xml_add_attr_f(xml_plat, "Condition", 9, "'$(Configuration)|$(Platform)'=='%.*s|%.*s'", config->len, config->data, platf_len, platf);

			if (proj->props[PROJ_PROP_TYPE].mask != PROJ_TYPE_EXE) {
				xml_add_child_val(xml_plat, "LinkIncremental", 15, debug ? "true" : "false", debug ? 4 : 5);
			}

			if (outdir->set) {
				char buf[MAX_PATH]   = { 0 };
				unsigned int buf_len = cstr_replaces(outdir->value.data, outdir->value.len, buf, MAX_PATH, vs_vars.names, vs_vars.tos, __VAR_MAX);

				if (buf_len >= 0) {
					xml_add_child_val(xml_plat, "OutDir", 6, buf, buf_len);
				}
			}

			if (intdir->set) {
				char buf[MAX_PATH]   = { 0 };
				unsigned int buf_len = cstr_replaces(intdir->value.data, intdir->value.len, buf, MAX_PATH, vs_vars.names, vs_vars.tos, __VAR_MAX);

				if (buf_len >= 0) {
					xml_add_child_val(xml_plat, "IntDir", 6, buf, buf_len);
				}
			}
		}
	}

	for (int i = platforms->count - 1; i >= 0; i--) {
		prop_str_t *platform   = array_get(platforms, i);
		const char *platf      = platform->data;
		unsigned int platf_len = platform->len;
		if (platform->len == 3 && strncmp(platform->data, "x86", 3) == 0) {
			platf	  = "Win32";
			platf_len = 5;
		}
		for (int j = 0; j < configs->count; j++) {
			prop_str_t *config = array_get(configs, j);

			int debug = 0;
			if (config->len == 5 && strncmp(config->data, "Debug", 5) == 0) {
				debug = 1;
			}

			int release = 0;
			if (config->len == 7 && strncmp(config->data, "Release", 7) == 0) {
				release = 1;
			}

			xml_tag_t *xml_def = xml_add_child(xml_proj, "ItemDefinitionGroup", 19);
			xml_add_attr_f(xml_def, "Condition", 9, "'$(Configuration)|$(Platform)'=='%.*s|%.*s'", config->len, config->data, platf_len, platf);
			xml_tag_t *xml_comp = xml_add_child(xml_def, "ClCompile", 9);
			xml_add_child_val(xml_comp, "WarningLevel", 12, "Level3", 6);

			if (release) {
				xml_add_child_val(xml_comp, "FunctionLevelLinking", 20, "true", 4);
				xml_add_child_val(xml_comp, "IntrinsicFunctions", 18, "true", 4);
			}

			xml_add_child_val(xml_comp, "SDLCheck", 8, "true", 4);
			xml_add_child_val(xml_comp, "ConformanceMode", 15, "true", 4);

			if (proj->props[PROJ_PROP_INCLUDE].set || proj->props[PROJ_PROP_INCLUDES].set) {
				xml_tag_t *xml_inc_dirs = xml_add_child(xml_comp, "AdditionalIncludeDirectories", 28);
				xml_inc_dirs->val.len	= print_includes(NULL, 0, proj, projects);
				xml_inc_dirs->val.data	= m_calloc((size_t)xml_inc_dirs->val.len + 1, sizeof(char));
				print_includes(xml_inc_dirs->val.tdata, xml_inc_dirs->val.len + 1, proj, projects);
				xml_inc_dirs->val.mem = 1;
			}

			if (proj->props[PROJ_PROP_DEFINES].set) {
				xml_tag_t *xml_defines = xml_add_child(xml_comp, "PreprocessorDefinitions", 23);
				xml_defines->val.len   = print_defines(NULL, 0, proj, projects);
				xml_defines->val.data  = m_calloc((size_t)xml_defines->val.len + 1, sizeof(char));
				print_defines(xml_defines->val.tdata, xml_defines->val.len + 1, proj, projects);
				xml_defines->val.mem = 1;
			}

			xml_tag_t *xml_link = xml_add_child(xml_def, "Link", 4);
			xml_add_child_val(xml_link, "SubSystem", 9, "Console", 7);

			if (release) {
				xml_add_child_val(xml_link, "EnableCOMDATFolding", 19, "true", 4);
				xml_add_child_val(xml_link, "OptimizeReferences", 18, "true", 4);
			}

			xml_add_child_val(xml_link, "GenerateDebugInformation", 24, "true", 4);
		}
	}

	add_file_t afd = { 0 };

	path_init(&afd.path, proj->props[PROJ_PROP_SOURCE].value.data, proj->props[PROJ_PROP_SOURCE].value.len);

	if (proj->props[PROJ_PROP_SOURCE].set) {
		xml_tag_t *xml_srcs = xml_add_child(xml_proj, "ItemGroup", 9);

		afd.xml_items = xml_srcs;

		path_t src = { 0 };
		path_init(&src, proj->path.path, proj->path.len);
		path_child(&src, proj->props[PROJ_PROP_SOURCE].value.data, proj->props[PROJ_PROP_SOURCE].value.len);

		files_foreach(&src, add_src_folder, add_src_file, &afd);
	}

	if (proj->props[PROJ_PROP_DEPENDS].set) {
		const array_t *depends = &proj->props[PROJ_PROP_DEPENDS].arr;

		if (depends->count > 0) {
			xml_tag_t *xml_refs = xml_add_child(xml_proj, "ItemGroup", 9);

			for (int i = 0; i < depends->count; i++) {
				prop_str_t *depend = array_get(depends, i);
				proj_t *dproj	   = { 0 };
				if(hashmap_get(projects, depend->data, depend->len, &dproj)) {
					ERR("project doesn't exists: '%.*s'", depend->len, depend->data);
					continue;
				}

				path_t rel_path = { 0 };
				path_calc_rel(proj->path.path, proj->path.len, dproj->path.path, dproj->path.len, &rel_path);

				xml_tag_t *xml_ref = xml_add_child(xml_refs, "ProjectReference", 16);
				xml_add_attr_f(xml_ref, "Include", 7, "%.*s\\%.*s.vcxproj", rel_path.len, rel_path.path, dproj->props[PROJ_PROP_NAME].value.len,
					       dproj->props[PROJ_PROP_NAME].value.data);
				xml_add_child_val_f(xml_ref, "Project", 7, "{%s}", dproj->guid);
			}
		}
	}

	if (proj->props[PROJ_PROP_SOURCE].set || proj->props[PROJ_PROP_INCLUDE].set) {
		xml_tag_t *xml_incs = xml_add_child(xml_proj, "ItemGroup", 9);
		afd.xml_items	    = xml_incs;

		const prop_str_t *srcv = &proj->props[PROJ_PROP_SOURCE].value;
		const prop_str_t *incv = &proj->props[PROJ_PROP_INCLUDE].value;

		if (proj->props[PROJ_PROP_SOURCE].set) {
			path_t src = { 0 };
			path_init(&src, proj->path.path, proj->path.len);
			path_child(&src, srcv->data, srcv->len);

			files_foreach(&src, add_inc_folder, add_inc_file, &afd);
		}

		if (proj->props[PROJ_PROP_INCLUDE].set && (!proj->props[PROJ_PROP_SOURCE].set || !cstr_cmp(srcv->data, srcv->len, incv->data, incv->len))) {
			path_init(&afd.path, incv->data, incv->len);

			path_t inc = { 0 };
			path_init(&inc, proj->path.path, proj->path.len);
			path_child(&inc, incv->data, incv->len);

			files_foreach(&inc, add_inc_folder, add_inc_file, &afd);
		}
	}

	xml_add_attr(xml_add_child(xml_proj, "Import", 6), "Project", 7, "$(VCTargetsPath)\\Microsoft.Cpp.targets", 38);
	xml_add_attr(xml_add_child_val(xml_proj, "ImportGroup", 11, "\n", 1), "Label", 5, "ExtensionTargets", 16);

	path_t cmake_path = *path;
	if (path_child(&cmake_path, proj->rel_path.path, proj->rel_path.len)) {
		return 1;
	}

	if (!folder_exists(cmake_path.path)) {
		folder_create(cmake_path.path);
	}

	if (path_child(&cmake_path, name->data, name->len)) {
		return 1;
	}

	if (path_child_s(&cmake_path, "vcxproj", 7, '.')) {
		return 1;
	}

	FILE *fp = file_open(cmake_path.path, "w", 1);
	if (fp == NULL) {
		return 1;
	}

	MSG("generating project: %s", cmake_path.path);

	xml_save(&xml, fp);
	fclose(fp);

	xml_free(&xml);

	xml_t xml_user = { 0 };
	xml_init(&xml_user);
	xml_tag_t *xml_proj_user = xml_add_child(&xml_user.root, "Project", 7);
	xml_add_attr(xml_proj_user, "ToolsVersion", 14, "Current", 7);
	xml_add_attr(xml_proj_user, "xmlns", 5, "http://schemas.microsoft.com/developer/msbuild/2003", 51);
	xml_tag_t *xml_proj_props = xml_add_child(xml_proj_user, "PropertyGroup", 13);
	xml_add_child_val(xml_proj_props, "ShowAllFiles", 12, "true", 4);

	path_t cmake_path_user = *path;
	if (path_child(&cmake_path_user, proj->rel_path.path, proj->rel_path.len)) {
		return 1;
	}

	if (!folder_exists(cmake_path_user.path)) {
		folder_create(cmake_path_user.path);
	}

	if (path_child(&cmake_path_user, name->data, name->len)) {
		return 1;
	}

	if (path_child_s(&cmake_path_user, "vcxproj.user", 12, '.')) {
		return 1;
	}

	FILE *fpu = file_open(cmake_path_user.path, "w", 1);
	if (fpu == NULL) {
		return 1;
	}

	xml_save(&xml_user, fpu);
	fclose(fpu);
	xml_free(&xml_user);

	if (ret == 0) {
		SUC("generating project: %s success", cmake_path.path);
	} else {
		ERR("generating project: %s failed", cmake_path.path);
	}

	return ret;
}

void proj_free(proj_t *proj)
{
	props_free(proj->props, s_proj_props, sizeof(s_proj_props));
	array_free(&proj->all_depends);
}
