#include "vs_proj.h"

#include "gen/sln.h"
#include "gen/var.h"

#include "common.h"

#include "xml.h"

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

static size_t resolve(const prop_str_t *prop, char *dst, size_t dst_max_len, const proj_t *proj)
{
	char buf[P_MAX_PATH] = { 0 };
	size_t buf_len, dst_len;

	buf_len = cstr_replaces(prop->data, prop->len, CSTR(buf), vars.names, vars.tos, __VAR_MAX);
	dst_len = cstr_replace(buf, buf_len, dst, dst_max_len, CSTR("$(PROJ_FOLDER)"), proj->rel_path.path, (int)proj->rel_path.len);

	return dst_len;
}

typedef struct add_file_s {
	path_t path;
	xml_t *xml;
	xml_tag_t xml_items;
	uint langs;
} add_file_t;

static int add_src_file(path_t *path, const char *folder, void *priv)
{
	add_file_t *data = priv;

	path_t new_path = data->path;
	path_child(&new_path, folder, cstr_len(folder));

	int add = ((data->langs & (1 << LANG_C)) && path_ends(&new_path, CSTR(".c"))) || ((data->langs & (1 << LANG_ASM)) && path_ends(&new_path, CSTR(".asm"))) ||
		  ((data->langs & (1 << LANG_CPP)) && path_ends(&new_path, CSTR(".cpp")));

	if (!add) {
		return 0;
	}

	if (path_ends(&new_path, CSTR(".asm"))) {
		xml_tag_t xml_inc = xml_add_child(data->xml, data->xml_items, CSTR("MASM"));
		xml_add_attr_c(data->xml, xml_inc, CSTR("Include"), new_path.path, new_path.len);
		xml_add_child_val(data->xml, xml_inc, CSTR("FileType"), CSTR("Document"));
	} else {
		xml_add_attr_c(data->xml, xml_add_child(data->xml, data->xml_items, CSTR("ClCompile")), CSTR("Include"), new_path.path, new_path.len);
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

	int add = ((data->langs & (1 << LANG_C)) && path_ends(&new_path, CSTR(".h"))) || ((data->langs & (1 << LANG_ASM)) && path_ends(&new_path, CSTR(".inc"))) ||
		  ((data->langs & (1 << LANG_CPP)) && path_ends(&new_path, CSTR(".h"))) || ((data->langs & (1 << LANG_CPP)) && path_ends(&new_path, CSTR(".hpp")));

	if (add) {
		xml_add_attr_c(data->xml, xml_add_child(data->xml, data->xml_items, CSTR("ClInclude")), CSTR("Include"), new_path.path, new_path.len);
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

static inline size_t print_includes(char *buf, size_t buf_size, const proj_t *proj, const hashmap_t *projects)
{
	size_t len = 0;
	bool first = 0;

	char tmp[P_MAX_PATH] = { 0 };
	size_t tmp_len;

	const prop_str_t *inc = &proj->props[PROJ_PROP_INCLUDE].value;
	const prop_str_t *enc = &proj->props[PROJ_PROP_ENCLUDE].value;

	if (proj->props[PROJ_PROP_INCLUDE].flags & PROP_SET) {
		len += snprintf(buf == NULL ? buf : buf + len, buf_size, "%.*s$(ProjectDir)%.*s", first, ";", (int)inc->len, inc->data);
		first = 1;
	}

	if (proj->props[PROJ_PROP_ENCLUDE].flags & PROP_SET) {
		tmp_len = cstr_replaces(enc->data, enc->len, tmp, sizeof(tmp) - 1, vars.names, vars.tos, __VAR_MAX);
		len += snprintf(buf == NULL ? buf : buf + len, buf_size, "%.*s%.*s", first, ";", (int)tmp_len, tmp);
		first = 1;
	}

	for (int i = 0; i < proj->includes.count; i++) {
		const proj_t *iproj = *(proj_t **)array_get(&proj->includes, i);

		if (iproj->props[PROJ_PROP_INCLUDE].flags & PROP_SET) {
			len += snprintf(buf == NULL ? buf : buf + len, buf_size, "%.*s$(SolutionDir)%.*s\\%.*s", first, ";", (int)iproj->rel_path.len,
					iproj->rel_path.path, (int)iproj->props[PROJ_PROP_INCLUDE].value.len, iproj->props[PROJ_PROP_INCLUDE].value.data);

			first = 1;
		}

		if (iproj->props[PROJ_PROP_ENCLUDE].flags & PROP_SET) {
			tmp_len = cstr_replaces(iproj->props[PROJ_PROP_ENCLUDE].value.data, iproj->props[PROJ_PROP_ENCLUDE].value.len, tmp, sizeof(tmp) - 1, vars.names,
						vars.tos, __VAR_MAX);
			len += snprintf(buf == NULL ? buf : buf + len, buf_size, "%.*s%.*s", first, ";", (int)tmp_len, tmp);

			first = 1;
		}
	}

	return len;
}

static inline size_t print_defines(char *buf, size_t buf_size, const proj_t *proj, const hashmap_t *projects)
{
	size_t len = 0;
	bool first = 1;

	if (proj->props[PROJ_PROP_DEFINES].flags & PROP_SET) {
		const array_t *defines = &proj->props[PROJ_PROP_DEFINES].arr;

		for (int k = 0; k < defines->count; k++) {
			prop_str_t *define = array_get(defines, k);

			len += snprintf(buf == NULL ? buf : buf + len, buf_size, first ? "%.*s" : ";%.*s", define->len, define->data);
			first = 0;
		}
	}

	return len;
}

static inline size_t print_libs(char *buf, size_t buf_size, const proj_t *proj, const hashmap_t *projects)
{
	size_t len = 0;
	bool first = 1;

	char tmp[P_MAX_PATH] = { 0 };
	size_t tmp_len;

	for (int i = 0; i < proj->all_depends.count; i++) {
		const proj_t *dproj = *(proj_t **)array_get(&proj->all_depends, i);

		if (dproj->props[PROJ_PROP_LIBDIRS].flags & PROP_SET) {
			const array_t *libdirs = &dproj->props[PROJ_PROP_LIBDIRS].arr;
			for (int j = 0; j < libdirs->count; j++) {
				prop_str_t *libdir = array_get(libdirs, j);
				if (libdir->len > 0) {
					tmp_len = cstr_replaces(libdir->data, (int)libdir->len, tmp, sizeof(tmp) - 1, vars.names, vars.tos, __VAR_MAX);

					if (cstrn_cmp(tmp, tmp_len, CSTR("$(SolutionDir)"), 14)) {
						len += snprintf(buf == NULL ? buf : buf + len, buf_size, first ? "%.*s" : ";%.*s", tmp_len, tmp);
					} else {
						path_t dir = { 0 };
						path_init(&dir, CSTR("$(SolutionDir)"));
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

static inline size_t print_ldflags(char *buf, size_t buf_size, const proj_t *proj, const hashmap_t *projects)
{
	size_t len = 0;
	bool first = 0;

	const prop_t *ldflags = &proj->props[PROJ_PROP_LDFLAGS];

	if ((ldflags->flags & PROP_SET) && (ldflags->mask & (1 << LDFLAG_WHOLEARCHIVE))) {
		len += snprintf(buf == NULL ? buf : buf + len, buf_size, "%*s/WHOLEARCHIVE", first, "");
		first = 1;
	}

	if (first) {
		len += snprintf(buf == NULL ? buf : buf + len, buf_size, "%*s%%(AdditionalOptions)", first, "");
	}

	return len;
}

//TODO: Make proj const
int vs_proj_gen(proj_t *proj, const hashmap_t *projects, const path_t *path, const prop_t *sln_props)
{
	const prop_str_t *name = proj->name;
	proj_type_t type       = proj->props[PROJ_PROP_TYPE].mask;

	const array_t *platforms = &sln_props[SLN_PROP_PLATFORMS].arr;
	const array_t *configs	 = &sln_props[SLN_PROP_CONFIGS].arr;
	const prop_t *langs	 = &proj->props[PROJ_PROP_LANGS];
	const prop_t *charset	 = &proj->props[PROJ_PROP_CHARSET];
	const prop_t *cflags	 = &proj->props[PROJ_PROP_CFLAGS];
	const prop_t *p_outdir	 = &proj->props[PROJ_PROP_OUTDIR];
	const prop_t *p_intdir	 = &proj->props[PROJ_PROP_INTDIR];

	char outdir[P_MAX_PATH] = { 0 };
	char intdir[P_MAX_PATH] = { 0 };

	size_t outdir_len = 0;
	size_t intdir_len = 0;

	if (p_outdir->flags & PROP_SET) {
		outdir_len = resolve(&p_outdir->value, CSTR(outdir), proj);
	}

	if (p_intdir->flags & PROP_SET) {
		intdir_len = resolve(&p_intdir->value, CSTR(intdir), proj);
	}

	if (charset->mask == CHARSET_UNICODE) {
		if (!(proj->props[PROJ_PROP_DEFINES].flags & PROP_SET)) {
			array_init(&proj->props[PROJ_PROP_DEFINES].arr, 2, sizeof(prop_str_t));
			proj->props[PROJ_PROP_DEFINES].flags |= PROP_SET | PROP_ARR;
		}

		prop_str_t unicode = STR("UNICODE");
		array_add(&proj->props[PROJ_PROP_DEFINES].arr, &unicode);
		prop_str_t unicode2 = STR("_UNICODE");
		array_add(&proj->props[PROJ_PROP_DEFINES].arr, &unicode2);
	}

	int ret = 0;

	xml_t xml = { 0 };
	xml_init(&xml, 256);
	xml_tag_t xml_proj = xml_add_child(&xml, 0, CSTR("Project"));
	xml_add_attr(&xml, xml_proj, CSTR("DefaultTargets"), CSTR("Build"));
	xml_add_attr(&xml, xml_proj, CSTR("xmlns"), CSTR("http://schemas.microsoft.com/developer/msbuild/2003"));
	xml_tag_t xml_proj_confs = xml_add_child(&xml, xml_proj, CSTR("ItemGroup"));
	xml_add_attr(&xml, xml_proj_confs, CSTR("Label"), CSTR("ProjectConfigurations"));

	for (int i = platforms->count - 1; i >= 0; i--) {
		prop_str_t *platform = array_get(platforms, i);
		const char *platf    = platform->data;
		size_t platf_len     = platform->len;
		if (cstr_cmp(platform->data, platform->len, "x86", 3)) {
			platf	  = "Win32";
			platf_len = 5;
		}
		for (int j = 0; j < configs->count; j++) {
			prop_str_t *config = array_get(configs, j);

			xml_tag_t xml_proj_conf = xml_add_child(&xml, xml_proj_confs, CSTR("ProjectConfiguration"));
			xml_add_attr_f(&xml, xml_proj_conf, CSTR("Include"), "%.*s|%.*s", config->len, config->data, platf_len, platf);
			xml_add_child_val(&xml, xml_proj_conf, CSTR("Configuration"), config->data, config->len);
			xml_add_child_val(&xml, xml_proj_conf, CSTR("Platform"), platf, platf_len);
		}
	}

	xml_tag_t xml_globals = xml_add_child(&xml, xml_proj, CSTR("PropertyGroup"));
	xml_add_attr(&xml, xml_globals, CSTR("Label"), CSTR("Globals"));
	xml_add_child_val(&xml, xml_globals, CSTR("VCProjectVersion"), CSTR("16.0"));
	xml_add_child_val(&xml, xml_globals, CSTR("Keyword"), CSTR("Win32Proj"));
	xml_add_child_val_f(&xml, xml_globals, CSTR("ProjectGuid"), "{%s}", proj->guid);
	xml_add_child_val(&xml, xml_globals, CSTR("RootNamespace"), proj->folder.path, proj->folder.len);
	xml_add_child_val(&xml, xml_globals, CSTR("WindowsTargetPlatformVersion"), CSTR("10.0"));

	xml_tag_t xml_import = xml_add_child(&xml, xml_proj, CSTR("Import"));
	xml_add_attr(&xml, xml_import, CSTR("Project"), CSTR("$(VCTargetsPath)\\Microsoft.Cpp.Default.props"));

	const struct {
		const char *name;
		size_t len;
	} config_types[] = {
		[PROJ_TYPE_UNKNOWN] = { CSTR("") },
		[PROJ_TYPE_LIB]	    = { CSTR("StaticLibrary") },
		[PROJ_TYPE_EXE]	    = { CSTR("Application") },
		[PROJ_TYPE_EXT]	    = { CSTR("StaticLibrary") },
	};

	const struct {
		const char *name;
		size_t len;
	} charsets[] = {
		[CHARSET_UNKNOWN]    = { CSTR("") },
		[CHARSET_UNICODE]    = { CSTR("Unicode") },
		[CHARSET_MULTI_BYTE] = { CSTR("MultiByte") },
	};

	for (int i = platforms->count - 1; i >= 0; i--) {
		prop_str_t *platform = array_get(platforms, i);
		const char *platf    = platform->data;
		size_t platf_len     = platform->len;
		if (cstr_cmp(platform->data, platform->len, "x86", 3)) {
			platf	  = "Win32";
			platf_len = 5;
		}
		for (int j = 0; j < configs->count; j++) {
			prop_str_t *config = array_get(configs, j);

			const int debug	  = cstr_cmp(config->data, config->len, CSTR("Debug"));
			const int release = cstr_cmp(config->data, config->len, CSTR("Release"));

			xml_tag_t xml_conf = xml_add_child(&xml, xml_proj, CSTR("PropertyGroup"));
			xml_add_attr_f(&xml, xml_conf, CSTR("Condition"), "'$(Configuration)|$(Platform)'=='%.*s|%.*s'", config->len, config->data, platf_len, platf);
			xml_add_attr(&xml, xml_conf, CSTR("Label"), CSTR("Configuration"));
			xml_add_child_val(&xml, xml_conf, CSTR("ConfigurationType"), config_types[proj->props[PROJ_PROP_TYPE].mask].name,
					  config_types[proj->props[PROJ_PROP_TYPE].mask].len);
			xml_add_child_val(&xml, xml_conf, CSTR("UseDebugLibraries"), debug ? "true" : "false", debug ? 4 : 5);
			xml_add_child_val(&xml, xml_conf, CSTR("PlatformToolset"), CSTR("v143"));

			if (release) {
				xml_add_child_val(&xml, xml_conf, CSTR("WholeProgramOptimization"), CSTR("true"));
			}

			if ((charset->flags & PROP_SET) && charset->mask != CHARSET_UNKNOWN) {
				xml_add_child_val(&xml, xml_conf, CSTR("CharacterSet"), charsets[charset->mask].name, charsets[charset->mask].len);
			}
		}
	}

	xml_add_attr(&xml, xml_add_child(&xml, xml_proj, CSTR("Import")), CSTR("Project"), CSTR("$(VCTargetsPath)\\Microsoft.Cpp.props"));

	if (langs->mask & (1 << LANG_ASM)) {
		xml_tag_t xml_ext_set = xml_add_child(&xml, xml_proj, CSTR("ImportGroup"));
		xml_add_attr(&xml, xml_ext_set, CSTR("Label"), CSTR("ExtensionSettings"));
		xml_add_attr(&xml, xml_add_child(&xml, xml_ext_set, CSTR("Import")), CSTR("Project"), CSTR("$(VCTargetsPath)\\BuildCustomizations\\masm.props"));
	} else {
		xml_add_attr(&xml, xml_add_child_val(&xml, xml_proj, CSTR("ImportGroup"), CSTR("\n")), CSTR("Label"), CSTR("ExtensionSettings"));
	}
	xml_add_attr(&xml, xml_add_child_val(&xml, xml_proj, CSTR("ImportGroup"), CSTR("\n")), CSTR("Label"), CSTR("Shared"));

	for (int i = platforms->count - 1; i >= 0; i--) {
		prop_str_t *platform = array_get(platforms, i);
		const char *platf    = platform->data;
		size_t platf_len     = platform->len;
		if (cstr_cmp(platform->data, platform->len, CSTR("x86"))) {
			platf	  = "Win32";
			platf_len = 5;
		}
		for (int j = 0; j < configs->count; j++) {
			prop_str_t *config = array_get(configs, j);

			xml_tag_t xml_sheets = xml_add_child(&xml, xml_proj, CSTR("ImportGroup"));
			xml_add_attr(&xml, xml_sheets, CSTR("Label"), CSTR("PropertySheets"));
			xml_add_attr_f(&xml, xml_sheets, CSTR("Condition"), "'$(Configuration)|$(Platform)'=='%.*s|%.*s'", config->len, config->data, platf_len, platf);
			xml_tag_t xml_data = xml_add_child(&xml, xml_sheets, CSTR("Import"));
			xml_add_attr(&xml, xml_data, CSTR("Project"), CSTR("$(UserRootDir)\\Microsoft.Cpp.$(Platform).user.props"));
			xml_add_attr(&xml, xml_data, CSTR("Condition"), CSTR("exists('$(UserRootDir)\\Microsoft.Cpp.$(Platform).user.props')"));
			xml_add_attr(&xml, xml_data, CSTR("Label"), CSTR("LocalAppDataPlatform"));
		}
	}

	xml_tag_t xml_macros = xml_add_child(&xml, xml_proj, CSTR("PropertyGroup"));
	xml_add_attr(&xml, xml_macros, CSTR("Label"), CSTR("UserMacros"));

	for (int i = platforms->count - 1; i >= 0; i--) {
		prop_str_t *platform = array_get(platforms, i);
		const char *platf    = platform->data;
		size_t platf_len     = platform->len;
		if (cstr_cmp(platform->data, platform->len, CSTR("x86"))) {
			platf	  = "Win32";
			platf_len = 5;
		}
		for (int j = 0; j < configs->count; j++) {
			prop_str_t *config = array_get(configs, j);

			const int debug = cstr_cmp(config->data, config->len, CSTR("Debug"));

			xml_tag_t xml_plat = xml_add_child(&xml, xml_proj, CSTR("PropertyGroup"));
			xml_add_attr_f(&xml, xml_plat, CSTR("Condition"), "'$(Configuration)|$(Platform)'=='%.*s|%.*s'", config->len, config->data, platf_len, platf);

			if (proj->props[PROJ_PROP_TYPE].mask != PROJ_TYPE_EXE) {
				xml_add_child_val(&xml, xml_plat, CSTR("LinkIncremental"), debug ? "true" : "false", debug ? 4 : 5);
			}

			if ((p_outdir->flags & PROP_SET)) {
				xml_add_child_val(&xml, xml_plat, CSTR("OutDir"), outdir, outdir_len);
			}

			if ((p_intdir->flags & PROP_SET)) {
				xml_add_child_val(&xml, xml_plat, CSTR("IntDir"), intdir, intdir_len);
			}
		}
	}

	for (int i = platforms->count - 1; i >= 0; i--) {
		prop_str_t *platform = array_get(platforms, i);
		const char *platf    = platform->data;
		size_t platf_len     = platform->len;
		if (cstr_cmp(platform->data, platform->len, CSTR("x86"))) {
			platf	  = "Win32";
			platf_len = 5;
		}
		for (int j = 0; j < configs->count; j++) {
			prop_str_t *config = array_get(configs, j);

			const int debug	  = cstr_cmp(config->data, config->len, CSTR("Debug"));
			const int release = cstr_cmp(config->data, config->len, CSTR("Release"));

			xml_tag_t xml_def = xml_add_child(&xml, xml_proj, CSTR("ItemDefinitionGroup"));
			xml_add_attr_f(&xml, xml_def, CSTR("Condition"), "'$(Configuration)|$(Platform)'=='%.*s|%.*s'", config->len, config->data, platf_len, platf);
			xml_tag_t xml_comp = xml_add_child(&xml, xml_def, CSTR("ClCompile"));
			xml_add_child_val(&xml, xml_comp, CSTR("WarningLevel"), CSTR("Level3"));

			if (release) {
				xml_add_child_val(&xml, xml_comp, CSTR("FunctionLevelLinking"), CSTR("true"));
				xml_add_child_val(&xml, xml_comp, CSTR("IntrinsicFunctions"), CSTR("true"));
			}

			xml_add_child_val(&xml, xml_comp, CSTR("SDLCheck"), CSTR("true"));
			xml_add_child_val(&xml, xml_comp, CSTR("ConformanceMode"), CSTR("true"));

			if ((proj->props[PROJ_PROP_INCLUDE].flags & PROP_SET) || (proj->props[PROJ_PROP_INCLUDES].flags & PROP_SET)) {
				size_t inc_len = print_includes(NULL, 0, proj, projects) + 1;
				char *inc_data = m_malloc(inc_len);
				print_includes(inc_data, inc_len, proj, projects);
				xml_add_child_val_r(&xml, xml_comp, CSTR("AdditionalIncludeDirectories"), inc_data, inc_len, 1);
			}

			if (proj->props[PROJ_PROP_DEFINES].flags & PROP_SET) {
				size_t def_len = print_defines(NULL, 0, proj, projects) + 1;
				char *def_data = m_malloc(def_len);
				print_defines(def_data, def_len, proj, projects);
				xml_add_child_val_r(&xml, xml_comp, CSTR("PreprocessorDefinitions"), def_data, def_len, 1);
			}

			xml_tag_t xml_link = xml_add_child(&xml, xml_def, CSTR("Link"));
			xml_add_child_val(&xml, xml_link, CSTR("SubSystem"), CSTR("Console"));

			if (release) {
				xml_add_child_val(&xml, xml_link, CSTR("EnableCOMDATFolding"), CSTR("true"));
				xml_add_child_val(&xml, xml_link, CSTR("OptimizeReferences"), CSTR("true"));
			}

			xml_add_child_val(&xml, xml_link, CSTR("GenerateDebugInformation"), CSTR("true"));

			if (proj->props[PROJ_PROP_LDFLAGS].flags & PROP_SET) {
				size_t ldf_len = print_ldflags(NULL, 0, proj, projects) + 1;
				if (ldf_len > 1) {
					char *ldf_data = m_malloc(ldf_len);
					print_ldflags(ldf_data, ldf_len, proj, projects);
					xml_add_child_val_r(&xml, xml_link, CSTR("AdditionalOptions"), ldf_data, ldf_len, 1);
				}

				if (proj->props[PROJ_PROP_LDFLAGS].mask & (1 << LDFLAG_NONE)) {
					xml_add_child(&xml, xml_link, CSTR("AdditionalDependencies"));
				}
			}

			if (proj->props[PROJ_PROP_TYPE].mask == PROJ_TYPE_EXE) {
				size_t libs_len = print_libs(NULL, 0, proj, projects) + 1;
				if (libs_len > 1) {
					char *libs_data = m_malloc(libs_len);
					print_libs(libs_data, libs_len, proj, projects);
					xml_add_child_val_r(&xml, xml_link, CSTR("AdditionalLibraryDirectories"), libs_data, libs_len, 1);
				}
			}
		}
	}

	add_file_t afd = {
		.xml   = &xml,
		.langs = langs->mask,
	};

	path_init(&afd.path, proj->props[PROJ_PROP_SOURCE].value.data, proj->props[PROJ_PROP_SOURCE].value.len);

	if (proj->props[PROJ_PROP_SOURCE].flags & PROP_SET) {
		xml_tag_t xml_srcs = xml_add_child(&xml, xml_proj, CSTR("ItemGroup"));

		afd.xml_items = xml_srcs;

		path_t src = { 0 };
		path_init(&src, proj->path.path, proj->path.len);
		path_child(&src, proj->props[PROJ_PROP_SOURCE].value.data, proj->props[PROJ_PROP_SOURCE].value.len);

		files_foreach(&src, add_src_folder, add_src_file, &afd);
	}

	if (proj->props[PROJ_PROP_DEPENDS].flags & PROP_SET) {
		const array_t *depends = &proj->all_depends;

		if (depends->count > 0) {
			xml_tag_t xml_refs = xml_add_child(&xml, xml_proj, CSTR("ItemGroup"));

			for (int i = 0; i < depends->count; i++) {
				const proj_t *dproj = *(proj_t **)array_get(depends, i);

				if (dproj->props[PROJ_PROP_TYPE].mask != PROJ_TYPE_EXT) {
					path_t rel_path = { 0 };
					path_calc_rel(proj->path.path, proj->path.len, dproj->path.path, dproj->path.len, &rel_path);

					xml_tag_t xml_ref = xml_add_child(&xml, xml_refs, CSTR("ProjectReference"));
					xml_add_attr_f(&xml, xml_ref, CSTR("Include"), "%.*s\\%.*s.vcxproj", rel_path.len, rel_path.path, dproj->name->len,
						       dproj->name->data);
					xml_add_child_val_f(&xml, xml_ref, CSTR("Project"), "{%s}", dproj->guid);
				}
			}
		}
	}

	if ((proj->props[PROJ_PROP_SOURCE].flags & PROP_SET) || (proj->props[PROJ_PROP_INCLUDE].flags & PROP_SET)) {
		xml_tag_t xml_incs = xml_add_child(&xml, xml_proj, CSTR("ItemGroup"));
		afd.xml_items	   = xml_incs;

		const prop_str_t *srcv = &proj->props[PROJ_PROP_SOURCE].value;
		const prop_str_t *incv = &proj->props[PROJ_PROP_INCLUDE].value;

		if (proj->props[PROJ_PROP_SOURCE].flags & PROP_SET) {
			path_t src = { 0 };
			path_init(&src, proj->path.path, proj->path.len);
			path_child(&src, srcv->data, srcv->len);

			files_foreach(&src, add_inc_folder, add_inc_file, &afd);
		}

		if ((proj->props[PROJ_PROP_INCLUDE].flags & PROP_SET) &&
		    (!(proj->props[PROJ_PROP_SOURCE].flags & PROP_SET) || !cstr_cmp(srcv->data, srcv->len, incv->data, incv->len))) {
			path_init(&afd.path, incv->data, incv->len);

			path_t inc = { 0 };
			path_init(&inc, proj->path.path, proj->path.len);
			path_child(&inc, incv->data, incv->len);

			files_foreach(&inc, add_inc_folder, add_inc_file, &afd);
		}
	}

	xml_add_attr(&xml, xml_add_child(&xml, xml_proj, CSTR("Import")), CSTR("Project"), CSTR("$(VCTargetsPath)\\Microsoft.Cpp.targets"));

	xml_tag_t xml_ext_tar = xml_add_child_val(&xml, xml_proj, CSTR("ImportGroup"), CSTR("\n"));
	xml_add_attr(&xml, xml_ext_tar, CSTR("Label"), CSTR("ExtensionTargets"));
	if (langs->mask & (1 << LANG_ASM)) {
		xml_add_attr(&xml, xml_add_child(&xml, xml_ext_tar, CSTR("Import")), CSTR("Project"), CSTR("$(VCTargetsPath)\\BuildCustomizations\\masm.targets"));
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

	if (path_child_s(&cmake_path, CSTR("vcxproj"), '.')) {
		return 1;
	}

	FILE *file = file_open(cmake_path.path, "w");
	if (file == NULL) {
		return 1;
	}

	MSG("generating project: %s", cmake_path.path);

	xml_print(&xml, file);
	file_close(file);
	xml_free(&xml);

	xml_t xml_user = { 0 };
	xml_init(&xml_user, 16);
	xml_tag_t xml_proj_user = xml_add_child(&xml_user, 0, CSTR("Project"));
	xml_add_attr(&xml_user, xml_proj_user, CSTR("ToolsVersion"), CSTR("Current"));
	xml_add_attr(&xml_user, xml_proj_user, CSTR("xmlns"), CSTR("http://schemas.microsoft.com/developer/msbuild/2003"));

	if (proj->props[PROJ_PROP_ARGS].flags & PROP_SET) {
		for (int i = platforms->count - 1; i >= 0; i--) {
			prop_str_t *platform = array_get(platforms, i);
			const char *platf    = platform->data;
			size_t platf_len     = platform->len;
			if (cstr_cmp(platform->data, platform->len, "x86", 3)) {
				platf	  = "Win32";
				platf_len = 5;
			}
			for (int j = 0; j < configs->count; j++) {
				prop_str_t *config = array_get(configs, j);

				const int debug = cstr_cmp(config->data, config->len, "Debug", 5);

				xml_tag_t xml_group = xml_add_child(&xml_user, xml_proj_user, CSTR("PropertyGroup"));
				xml_add_attr_f(&xml_user, xml_group, CSTR("Condition"), "'$(Configuration)|$(Platform)'=='%.*s|%.*s'", config->len, config->data,
					       platf_len, platf);

				xml_add_child_val(&xml_user, xml_group, CSTR("LocalDebuggerCommandArguments"), proj->props[PROJ_PROP_ARGS].value.data,
						  proj->props[PROJ_PROP_ARGS].value.len);
			}
		}
	}

	xml_tag_t xml_proj_props = xml_add_child(&xml_user, xml_proj_user, CSTR("PropertyGroup"));
	xml_add_child_val(&xml_user, xml_proj_props, CSTR("ShowAllFiles"), CSTR("true"));

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

	file = file_open(cmake_path_user.path, "w");
	if (file == NULL) {
		return 1;
	}

	xml_print(&xml_user, file);
	file_close(file);
	xml_free(&xml_user);

	if (ret == 0) {
		SUC("generating project: %s success", cmake_path.path);
	} else {
		ERR("generating project: %s failed", cmake_path.path);
	}

	return ret;
}
