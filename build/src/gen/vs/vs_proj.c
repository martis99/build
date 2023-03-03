#include "vs_proj.h"

#include "gen/var.h"

#include "data/xml.h"

#include "defines.h"
#include "mem.h"
#include "utils.h"

static const var_pol_t vars = {
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

typedef struct add_file_s {
	path_t path;
	xml_tag_t *xml_items;
	unsigned int langs;
} add_file_t;

static int add_src_file(path_t *path, const char *folder, void *priv)
{
	add_file_t *data = priv;

	path_t new_path = data->path;
	path_child(&new_path, folder, cstr_len(folder));

	int add = ((data->langs & (1 << LANG_C)) && path_ends(&new_path, ".c", 2)) || ((data->langs & (1 << LANG_ASM)) && path_ends(&new_path, ".asm", 4)) ||
		  ((data->langs & (1 << LANG_CPP)) && path_ends(&new_path, ".cpp", 4));

	if (!add) {
		return 0;
	}

	if (path_ends(&new_path, ".asm", 4)) {
		xml_tag_t *xml_inc = xml_add_child(data->xml_items, "MASM", 9);
		xml_add_attr_c(xml_inc, "Include", 7, new_path.path, new_path.len);
		xml_add_child_val(xml_inc, "FileType", 8, "Document", 8);
	} else {
		xml_add_attr_c(xml_add_child(data->xml_items, "ClCompile", 9), "Include", 7, new_path.path, new_path.len);
	}
	return 0;
}

static int add_src_folder(path_t *path, const char *folder, void *priv)
{
	add_file_t data = *(add_file_t *)priv;

	path_child(&data.path, folder, cstr_len(folder));

	files_foreach(path, add_src_folder, add_src_file, &data);

	return 0;
}

static int add_inc_file(path_t *path, const char *folder, void *priv)
{
	add_file_t *data = priv;

	path_t new_path = data->path;
	path_child(&new_path, folder, cstr_len(folder));

	int add = ((data->langs & (1 << LANG_C)) && path_ends(&new_path, ".h", 2)) || ((data->langs & (1 << LANG_ASM)) && path_ends(&new_path, ".inc", 4)) ||
		  ((data->langs & (1 << LANG_CPP)) && path_ends(&new_path, ".h", 2)) || ((data->langs & (1 << LANG_CPP)) && path_ends(&new_path, ".hpp", 4));

	if (add) {
		xml_add_attr_c(xml_add_child(data->xml_items, "ClInclude", 9), "Include", 7, new_path.path, new_path.len);
	}

	return 0;
}

static int add_inc_folder(path_t *path, const char *folder, void *priv)
{
	add_file_t data = *(add_file_t *)priv;

	path_child(&data.path, folder, cstr_len(folder));

	files_foreach(path, add_inc_folder, add_inc_file, &data);

	return 0;
}

static inline int print_includes(char *buf, unsigned int buf_size, const proj_t *proj, const hashmap_t *projects)
{
	unsigned int len = 0;
	int first	 = 1;

	char tmp[B_MAX_PATH] = { 0 };
	unsigned int tmp_len;

	const prop_str_t *src = &proj->props[PROJ_PROP_SOURCE].value;
	const prop_str_t *inc = &proj->props[PROJ_PROP_INCLUDE].value;
	const prop_str_t *enc = &proj->props[PROJ_PROP_ENCLUDE].value;

	if (proj->props[PROJ_PROP_SOURCE].set) {
		len += snprintf(buf == NULL ? buf : buf + len, buf_size, "$(ProjectDir)%.*s", src->len, src->data);
		first = 0;
	}

	if (proj->props[PROJ_PROP_INCLUDE].set && (!proj->props[PROJ_PROP_SOURCE].set || !cstr_cmp(src->data, src->len, inc->data, inc->len))) {
		len += snprintf(buf == NULL ? buf : buf + len, buf_size, first ? "$(ProjectDir)%.*s" : ";$(ProjectDir)%.*s", inc->len, inc->data);
		first = 0;
	}

	if (proj->props[PROJ_PROP_ENCLUDE].set) {
		tmp_len = cstr_replaces(enc->data, enc->len, tmp, sizeof(tmp) - 1, vars.names, vars.tos, __VAR_MAX);
		len += snprintf(buf == NULL ? buf : buf + len, buf_size, first ? "$(ProjectDir)%.*s" : ";$(ProjectDir)%.*s", tmp_len, tmp);
		first = 0;
	}

	for (int i = 0; i < proj->includes.count; i++) {
		const proj_t *iproj = *(proj_t **)array_get(&proj->includes, i);

		if (iproj->props[PROJ_PROP_INCLUDE].set) {
			len += snprintf(buf == NULL ? buf : buf + len, buf_size, first ? "$(SolutionDir)%.*s\\%.*s" : ";$(SolutionDir)%.*s\\%.*s", iproj->rel_path.len,
					iproj->rel_path.path, iproj->props[PROJ_PROP_INCLUDE].value.len, iproj->props[PROJ_PROP_INCLUDE].value.data);

			first = 0;
		}

		if (iproj->props[PROJ_PROP_ENCLUDE].set) {
			tmp_len = cstr_replaces(iproj->props[PROJ_PROP_ENCLUDE].value.data, iproj->props[PROJ_PROP_ENCLUDE].value.len, tmp, sizeof(tmp) - 1, vars.names,
						vars.tos, __VAR_MAX);
			len += snprintf(buf == NULL ? buf : buf + len, buf_size, first ? "%.*s" : ";%.*s", tmp_len, tmp);

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

static inline int print_libs(char *buf, unsigned int buf_size, const proj_t *proj, const hashmap_t *projects)
{
	unsigned int len = 0;
	int first	 = 1;

	char tmp[B_MAX_PATH] = { 0 };
	unsigned int tmp_len;

	for (int i = 0; i < proj->all_depends.count; i++) {
		const proj_t *dproj = *(proj_t **)array_get(&proj->all_depends, i);

		if (dproj->props[PROJ_PROP_LIBDIRS].set) {
			const array_t *libdirs = &dproj->props[PROJ_PROP_LIBDIRS].arr;
			for (int j = 0; j < libdirs->count; j++) {
				prop_str_t *libdir = array_get(libdirs, j);
				if (libdir->len > 0) {
					tmp_len = cstr_replaces(libdir->data, libdir->len, tmp, sizeof(tmp) - 1, vars.names, vars.tos, __VAR_MAX);

					if (cstrn_cmp(tmp, tmp_len, "$(SolutionDir)", 14, 14)) {
						len += snprintf(buf == NULL ? buf : buf + len, buf_size, first ? "%.*s" : ";%.*s", tmp_len, tmp);
					} else {
						path_t dir = { 0 };
						path_init(&dir, "$(SolutionDir)", 14);
						path_child_s(&dir, dproj->rel_path.path, dproj->rel_path.len, 0);
						path_child(&dir, tmp, tmp_len);

						len += snprintf(buf == NULL ? buf : buf + len, buf_size, first ? "%.*s" : ";%.*s", dir.len, dir.path);
					}

					first = 0;
				}
			}
		}
	}

	return len;
}

static inline int print_ldflags(char *buf, unsigned int buf_size, const proj_t *proj, const hashmap_t *projects)
{
	unsigned int len = 0;
	int first	 = 0;

	ldflag_t ldflags = proj->props[PROJ_PROP_LDFLAGS].mask;
	if (ldflags & (1 << LDFLAG_WHOLEARCHIVE)) {
		len += snprintf(buf == NULL ? buf : buf + len, buf_size, "%*s/WHOLEARCHIVE", first, "");
		first = 1;
	}

	if (first) {
		len += snprintf(buf == NULL ? buf : buf + len, buf_size, "%*s%%(AdditionalOptions)", first, "");
	}

	return len;
}

//TODO: Make proj const
int vs_proj_gen(proj_t *proj, const hashmap_t *projects, const path_t *path, const array_t *configs, const array_t *platforms, const prop_t *langs, const prop_t *charset,
		const prop_t *cflags, const prop_t *outdir, const prop_t *intdir)
{
	const prop_str_t *name = proj->name;
	proj_type_t type       = proj->props[PROJ_PROP_TYPE].mask;

	langs	= proj->props[PROJ_PROP_LANGS].set ? &proj->props[PROJ_PROP_LANGS] : langs;
	charset = proj->props[PROJ_PROP_CHARSET].set ? &proj->props[PROJ_PROP_CHARSET] : charset;
	cflags	= proj->props[PROJ_PROP_CFLAGS].set ? &proj->props[PROJ_PROP_CFLAGS] : cflags;
	outdir	= proj->props[PROJ_PROP_OUTDIR].set ? &proj->props[PROJ_PROP_OUTDIR] : outdir;
	intdir	= proj->props[PROJ_PROP_INTDIR].set ? &proj->props[PROJ_PROP_INTDIR] : intdir;

	char buf[B_MAX_PATH] = { 0 };
	unsigned int buf_len;

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
		if (cstr_cmp(platform->data, platform->len, "x86", 3)) {
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
		if (cstr_cmp(platform->data, platform->len, "x86", 3)) {
			platf	  = "Win32";
			platf_len = 5;
		}
		for (int j = 0; j < configs->count; j++) {
			prop_str_t *config = array_get(configs, j);

			const int debug	  = cstr_cmp(config->data, config->len, "Debug", 5);
			const int release = cstr_cmp(config->data, config->len, "Release", 7);

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

	if (langs->mask & (1 << LANG_ASM)) {
		xml_tag_t *xml_ext_set = xml_add_child(xml_proj, "ImportGroup", 11);
		xml_add_attr(xml_ext_set, "Label", 5, "ExtensionSettings", 17);
		xml_add_attr(xml_add_child(xml_ext_set, "Import", 6), "Project", 7, "$(VCTargetsPath)\\BuildCustomizations\\masm.props", 47);
	} else {
		xml_add_attr(xml_add_child_val(xml_proj, "ImportGroup", 11, "\n", 1), "Label", 5, "ExtensionSettings", 17);
	}
	xml_add_attr(xml_add_child_val(xml_proj, "ImportGroup", 11, "\n", 1), "Label", 5, "Shared", 6);

	for (int i = platforms->count - 1; i >= 0; i--) {
		prop_str_t *platform   = array_get(platforms, i);
		const char *platf      = platform->data;
		unsigned int platf_len = platform->len;
		if (cstr_cmp(platform->data, platform->len, "x86", 3)) {
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
		if (cstr_cmp(platform->data, platform->len, "x86", 3)) {
			platf	  = "Win32";
			platf_len = 5;
		}
		for (int j = 0; j < configs->count; j++) {
			prop_str_t *config = array_get(configs, j);

			const int debug = cstr_cmp(config->data, config->len, "Debug", 5);

			xml_tag_t *xml_plat = xml_add_child(xml_proj, "PropertyGroup", 13);
			xml_add_attr_f(xml_plat, "Condition", 9, "'$(Configuration)|$(Platform)'=='%.*s|%.*s'", config->len, config->data, platf_len, platf);

			if (proj->props[PROJ_PROP_TYPE].mask != PROJ_TYPE_EXE) {
				xml_add_child_val(xml_plat, "LinkIncremental", 15, debug ? "true" : "false", debug ? 4 : 5);
			}

			if (outdir->set) {
				buf_len = cstr_replaces(outdir->value.data, outdir->value.len, buf, sizeof(buf) - 1, vars.names, vars.tos, __VAR_MAX);

				if (buf_len >= 0) {
					xml_add_child_val(xml_plat, "OutDir", 6, buf, buf_len);
				}
			}

			if (intdir->set) {
				buf_len = cstr_replaces(intdir->value.data, intdir->value.len, buf, sizeof(buf) - 1, vars.names, vars.tos, __VAR_MAX);

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
		if (cstr_cmp(platform->data, platform->len, "x86", 3)) {
			platf	  = "Win32";
			platf_len = 5;
		}
		for (int j = 0; j < configs->count; j++) {
			prop_str_t *config = array_get(configs, j);

			const int debug	  = cstr_cmp(config->data, config->len, "Debug", 5);
			const int release = cstr_cmp(config->data, config->len, "Release", 7);

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

			if (proj->props[PROJ_PROP_LDFLAGS].set) {
				unsigned int len = print_ldflags(NULL, 0, proj, projects);
				if (len > 0) {
					xml_tag_t *xml_ldflags = xml_add_child(xml_link, "AdditionalOptions", 17);
					xml_ldflags->val.len   = len;
					xml_ldflags->val.data  = m_calloc((size_t)xml_ldflags->val.len + 1, sizeof(char));
					print_ldflags(xml_ldflags->val.tdata, xml_ldflags->val.len + 1, proj, projects);
					xml_ldflags->val.mem = 1;
				}

				if (proj->props[PROJ_PROP_LDFLAGS].mask & (1 << LDFLAG_NONE)) {
					xml_add_child(xml_link, "AdditionalDependencies", 22);
				}
			}

			if (proj->props[PROJ_PROP_TYPE].mask == PROJ_TYPE_EXE) {
				unsigned int len = print_libs(NULL, 0, proj, projects);
				if (len > 0) {
					xml_tag_t *xml_libs = xml_add_child(xml_link, "AdditionalLibraryDirectories", 28);
					xml_libs->val.len   = len;
					xml_libs->val.data  = m_calloc((size_t)xml_libs->val.len + 1, sizeof(char));
					print_libs(xml_libs->val.tdata, xml_libs->val.len + 1, proj, projects);
					xml_libs->val.mem = 1;
				}
			}
		}
	}

	add_file_t afd = {
		.langs = langs->mask,
	};

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
				if (hashmap_get(projects, depend->data, depend->len, (void **)&dproj)) {
					ERR("project doesn't exists: '%.*s'", depend->len, depend->data);
					continue;
				}

				if (dproj->props[PROJ_PROP_TYPE].mask != PROJ_TYPE_EXT) {
					path_t rel_path = { 0 };
					path_calc_rel(proj->path.path, proj->path.len, dproj->path.path, dproj->path.len, &rel_path);

					xml_tag_t *xml_ref = xml_add_child(xml_refs, "ProjectReference", 16);
					xml_add_attr_f(xml_ref, "Include", 7, "%.*s\\%.*s.vcxproj", rel_path.len, rel_path.path, dproj->name->len, dproj->name->data);
					xml_add_child_val_f(xml_ref, "Project", 7, "{%s}", dproj->guid);
				}
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

	xml_tag_t *xml_ext_tar = xml_add_child_val(xml_proj, "ImportGroup", 11, "\n", 1);
	xml_add_attr(xml_ext_tar, "Label", 5, "ExtensionTargets", 16);
	if (langs->mask & (1 << LANG_ASM)) {
		xml_add_attr(xml_add_child(xml_ext_tar, "Import", 6), "Project", 7, "$(VCTargetsPath)\\BuildCustomizations\\masm.targets", 49);
	}

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

	if (proj->props[PROJ_PROP_ARGS].set) {
		for (int i = platforms->count - 1; i >= 0; i--) {
			prop_str_t *platform   = array_get(platforms, i);
			const char *platf      = platform->data;
			unsigned int platf_len = platform->len;
			if (cstr_cmp(platform->data, platform->len, "x86", 3)) {
				platf	  = "Win32";
				platf_len = 5;
			}
			for (int j = 0; j < configs->count; j++) {
				prop_str_t *config = array_get(configs, j);

				const int debug = cstr_cmp(config->data, config->len, "Debug", 5);

				xml_tag_t *xml_group = xml_add_child(xml_proj_user, "PropertyGroup", 13);
				xml_add_attr_f(xml_group, "Condition", 9, "'$(Configuration)|$(Platform)'=='%.*s|%.*s'", config->len, config->data, platf_len, platf);

				xml_add_child_val(xml_group, "LocalDebuggerCommandArguments", 29, proj->props[PROJ_PROP_ARGS].value.data,
						  proj->props[PROJ_PROP_ARGS].value.len);
			}
		}
	}

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
