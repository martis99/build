#include "vs_proj.h"

#include "gen/sln.h"

#include "common.h"

#include "xml.h"

static size_t resolve(const prop_str_t *prop, char *buf, size_t buf_size, const proj_t *proj)
{
	size_t buf_len = prop->val.len;
	mem_cpy(buf, buf_size, prop->val.data, prop->val.len);
	buf_len = cstr_replace(buf, buf_size, buf_len, CSTR("$(SLNDIR)"), CSTR("$(SolutionDir)"), NULL);
	buf_len = cstr_replace(buf, buf_size, buf_len, CSTR("$(PROJDIR)"), proj->rel_dir.path, proj->rel_dir.len, NULL);
	buf_len = cstr_replace(buf, buf_size, buf_len, CSTR("$(PROJNAME)"), CSTR("$(ProjectName)"), NULL);
	buf_len = cstr_replace(buf, buf_size, buf_len, CSTR("$(CONFIG)"), CSTR("$(Configuration)"), NULL);
	buf_len = cstr_replace(buf, buf_size, buf_len, CSTR("$(ARCH)"), CSTR("$(PlatformTarget)"), NULL);
	convert_backslash(buf, buf_len);
	return buf_len;
}

typedef struct add_file_s {
	path_t path;
	xml_t *xml;
	xml_tag_t xml_items;
	uint langs;
} add_file_t;

static int add_src_file(path_t *path, const char *file, void *priv)
{
	add_file_t *data = priv;

	path_t new_path = data->path;
	path_child(&new_path, file, cstr_len(file));

	int add = ((data->langs & (1 << LANG_ASM)) && path_ends(&new_path, CSTR(".asm"))) || ((data->langs & (1 << LANG_NASM)) && path_ends(&new_path, CSTR(".nasm"))) ||
		  ((data->langs & (1 << LANG_C)) && path_ends(&new_path, CSTR(".c"))) || ((data->langs & (1 << LANG_CPP)) && path_ends(&new_path, CSTR(".cpp")));

	if (!add) {
		return 0;
	}

	convert_backslash(new_path.path, new_path.len);

	if (path_ends(&new_path, CSTR(".asm"))) {
		xml_tag_t xml_inc = xml_add_tag(data->xml, data->xml_items, STR("MASM"));
		xml_add_attr(data->xml, xml_inc, STR("Include"), strn(new_path.path, new_path.len, new_path.len + 1));
		xml_add_tag_val(data->xml, xml_inc, STR("FileType"), STR("Document"));
	} else {
		xml_add_attr(data->xml, xml_add_tag(data->xml, data->xml_items, STR("ClCompile")), STR("Include"), strn(new_path.path, new_path.len, new_path.len + 1));
	}
	return 0;
}

static int add_src_folder(path_t *path, const char *folder, void *priv)
{
	add_file_t data = *(add_file_t *)priv;

	path_child_folder(&data.path, folder, cstr_len(folder));

	files_foreach(path, add_src_folder, add_src_file, &data);

	return 0;
}

static int add_inc_file(path_t *path, const char *file, void *priv)
{
	add_file_t *data = priv;

	path_t new_path = data->path;
	path_child(&new_path, file, cstr_len(file));

	int add = ((data->langs & ((1 << LANG_ASM) | (1 << LANG_NASM))) && path_ends(&new_path, CSTR(".inc"))) ||
		  ((data->langs & (1 << LANG_C)) && path_ends(&new_path, CSTR(".h"))) || ((data->langs & (1 << LANG_CPP)) && path_ends(&new_path, CSTR(".h"))) ||
		  ((data->langs & (1 << LANG_CPP)) && path_ends(&new_path, CSTR(".hpp")));

	if (add) {
		convert_backslash(new_path.path, new_path.len);
		xml_add_attr(data->xml, xml_add_tag(data->xml, data->xml_items, STR("ClInclude")), STR("Include"), strn(new_path.path, new_path.len, new_path.len + 1));
	}

	return 0;
}

static int add_inc_folder(path_t *path, const char *folder, void *priv)
{
	add_file_t data = *(add_file_t *)priv;

	path_child_folder(&data.path, folder, cstr_len(folder));

	files_foreach(path, add_inc_folder, add_inc_file, &data);

	return 0;
}

static inline size_t print_includes(char *buf, size_t buf_size, const proj_t *proj, const dict_t *projects)
{
	size_t len = 0;
	bool first = 0;

	char tmp[P_MAX_PATH] = { 0 };
	size_t tmp_len;

	const prop_str_t *enc = &proj->props[PROJ_PROP_ENCLUDE].value;

	if (proj->props[PROJ_PROP_INCLUDE].flags & PROP_SET) {
		const arr_t *includes = &proj->props[PROJ_PROP_INCLUDE].arr;
		for (uint i = 0; i < includes->cnt; i++) {
			prop_str_t *include = arr_get(includes, i);

			tmp_len = resolve(include, CSTR(tmp), proj);
			len += snprintf(buf == NULL ? buf : buf + len, buf_size, "%.*s$(ProjectDir)%.*s", first, ";", (int)tmp_len - 1, tmp);
			first = 1;
		}
	}

	if (proj->props[PROJ_PROP_SOURCE].flags & PROP_SET) {
		const arr_t *sources = &proj->props[PROJ_PROP_SOURCE].arr;
		for (uint i = 0; i < sources->cnt; i++) {
			prop_str_t *source = arr_get(sources, i);

			tmp_len = resolve(source, CSTR(tmp), proj);
			len += snprintf(buf == NULL ? buf : buf + len, buf_size, "%.*s$(ProjectDir)%.*s", first, ";", (int)tmp_len - 1, tmp);
			first = 1;
		}
	}

	if (proj->props[PROJ_PROP_ENCLUDE].flags & PROP_SET) {
		tmp_len = resolve(enc, CSTR(tmp), proj);
		len += snprintf(buf == NULL ? buf : buf + len, buf_size, "%.*s%.*s", first, ";", (int)tmp_len - 1, tmp);
		first = 1;
	}

	for (uint i = 0; i < proj->includes.cnt; i++) {
		const proj_t *iproj = *(proj_t **)arr_get(&proj->includes, i);

		if (iproj->props[PROJ_PROP_INCLUDE].flags & PROP_SET) {
			const arr_t *includes = &iproj->props[PROJ_PROP_INCLUDE].arr;
			for (uint i = 0; i < includes->cnt; i++) {
				prop_str_t *include = arr_get(includes, i);

				tmp_len = resolve(include, CSTR(tmp), iproj);
				len += snprintf(buf == NULL ? buf : buf + len, buf_size, "%.*s$(SolutionDir)%.*s\\%.*s", first, ";", (int)iproj->rel_dir.len - 1,
						iproj->rel_dir.path, (int)tmp_len - 1, tmp);

				first = 1;
			}
		}

		if (iproj->props[PROJ_PROP_ENCLUDE].flags & PROP_SET) {
			tmp_len = resolve(&iproj->props[PROJ_PROP_ENCLUDE].value, CSTR(tmp), iproj);
			len += snprintf(buf == NULL ? buf : buf + len, buf_size, "%.*s%.*s", first, ";", (int)tmp_len - 1, tmp);

			first = 1;
		}
	}

	return len;
}

static inline size_t print_defines(char *buf, size_t buf_size, const proj_t *proj, const dict_t *projects, int shared)
{
	size_t len = 0;
	bool first = 1;

	if (shared) {
		str_t upper = strz(proj->name.len + 1);
		str_to_upper(proj->name, &upper);
		len += snprintf(buf == NULL ? buf : buf + len, buf_size, first ? "%.*s_BUILD_DLL" : ";%.*s_BUILD_DLL", (int)upper.len, upper.data);
		first = 0;
		str_free(&upper);
	}

	for (uint i = 0; i < proj->all_depends.cnt; i++) {
		const proj_dep_t *dep = arr_get(&proj->all_depends, i);

		if (dep->link_type != LINK_TYPE_SHARED) {
			continue;
		}

		str_t upper = strz(dep->proj->name.len + 1);
		str_to_upper(dep->proj->name, &upper);
		len += snprintf(buf == NULL ? buf : buf + len, buf_size, first ? "%.*s_DLL" : ";%.*s_DLL", (int)upper.len, upper.data);
		first = 0;
		str_free(&upper);
	}

	if (proj->props[PROJ_PROP_DEFINES].flags & PROP_SET) {
		const arr_t *defines = &proj->props[PROJ_PROP_DEFINES].arr;

		for (uint k = 0; k < defines->cnt; k++) {
			prop_str_t *define = arr_get(defines, k);

			len += snprintf(buf == NULL ? buf : buf + len, buf_size, first ? "%.*s" : ";%.*s", (int)define->val.len, define->val.data);
			first = 0;
		}
	}

	return len;
}

static inline size_t print_libs(char *buf, size_t buf_size, const proj_t *proj, const dict_t *projects)
{
	size_t len = 0;
	bool first = 1;

	char tmp[P_MAX_PATH] = { 0 };
	size_t tmp_len;

	for (uint i = 0; i < proj->all_depends.cnt; i++) {
		const proj_dep_t *dep = arr_get(&proj->all_depends, i);

		if (dep->proj->props[PROJ_PROP_LIBDIRS].flags & PROP_SET) {
			const arr_t *libdirs = &dep->proj->props[PROJ_PROP_LIBDIRS].arr;
			for (uint j = 0; j < libdirs->cnt; j++) {
				prop_str_t *libdir = arr_get(libdirs, j);
				if (libdir->val.len > 0) {
					tmp_len = resolve(libdir, CSTR(tmp), dep->proj);

					if (cstr_eqn(tmp, tmp_len, CSTR("$(SolutionDir)"), 14)) {
						len += snprintf(buf == NULL ? buf : buf + len, buf_size, first ? "%.*s" : ";%.*s", (int)tmp_len, tmp);
					} else {
						path_t dir = { 0 };
						path_init(&dir, CSTR("$(SolutionDir)"));
						path_child_s(&dir, dep->proj->rel_dir.path, dep->proj->rel_dir.len, 0);
						path_child(&dir, tmp, tmp_len);
#if defined(C_LINUX)
						convert_backslash(dir.path, dir.len);
#endif
						len += snprintf(buf == NULL ? buf : buf + len, buf_size, first ? "%.*s" : ";%.*s", (int)dir.len, dir.path);
					}

					first = 0;
				}
			}
		}
	}

	return len;
}

static inline size_t print_ldflags(char *buf, size_t buf_size, const proj_t *proj, const dict_t *projects)
{
	size_t len = 0;
	bool first = 0;

	const prop_t *flags = &proj->props[PROJ_PROP_FLAGS];

	if ((flags->flags & PROP_SET) && (flags->mask & (1 << FLAG_WHOLEARCHIVE))) {
		len += snprintf(buf == NULL ? buf : buf + len, buf_size, "%*s/WHOLEARCHIVE", first, "");
		first = 1;
	}

	if (first) {
		len += snprintf(buf == NULL ? buf : buf + len, buf_size, "%*s%%(AdditionalOptions)", first, "");
	}

	return len;
}

static inline size_t print_depends(char *buf, size_t buf_size, const proj_t *proj, const dict_t *projects)
{
	size_t len = 0;
	bool first = 1;

	for (uint i = 0; i < proj->all_depends.cnt; i++) {
		const proj_dep_t *dep = arr_get(&proj->all_depends, i);

		if (dep->proj->props[PROJ_PROP_LINK].flags & PROP_SET) {
			const arr_t *links = &dep->proj->props[PROJ_PROP_LINK].arr;
			for (uint j = 0; j < links->cnt; j++) {
				const prop_str_t *link = arr_get(links, j);
				if (link->val.len > 0) {
					len += snprintf(buf == NULL ? buf : buf + len, buf_size, first ? "%.*s.lib" : ";%.*s.lib", (int)link->val.len, link->val.data);
					first = 0;
				}
			}
		}
	}

	return len;
}

//TODO: Make proj const
int vs_proj_gen(proj_t *proj, const dict_t *projects, const prop_t *sln_props, int shared)
{
	const str_t *name = &proj->name;
	proj_type_t type  = proj->props[PROJ_PROP_TYPE].mask;

	const arr_t *archs     = &sln_props[SLN_PROP_ARCHS].arr;
	const arr_t *configs   = &sln_props[SLN_PROP_CONFIGS].arr;
	const prop_t *langs    = &proj->props[PROJ_PROP_LANGS];
	const prop_t *charset  = &proj->props[PROJ_PROP_CHARSET];
	const prop_t *flags    = &proj->props[PROJ_PROP_FLAGS];
	const prop_t *p_outdir = &proj->props[PROJ_PROP_OUTDIR];
	const prop_t *p_intdir = &proj->props[PROJ_PROP_INTDIR];

	char outdir[P_MAX_PATH] = { 0 };
	char intdir[P_MAX_PATH] = { 0 };

	size_t outdir_len = 0;
	size_t intdir_len = 0;

	if (p_outdir->flags & PROP_SET) {
		outdir_len = resolve(&p_outdir->value, CSTR(outdir), proj);
	}

	if (p_intdir->flags & PROP_SET) {
		intdir_len = resolve(&p_intdir->value, CSTR(intdir), proj);
		if (shared) {
			intdir_len = cstr_cat(intdir, sizeof(intdir), intdir_len, CSTR("dll\\"));
		}
	}

	if (charset->mask == CHARSET_UNICODE) {
		if (!(proj->props[PROJ_PROP_DEFINES].flags & PROP_SET)) {
			arr_init(&proj->props[PROJ_PROP_DEFINES].arr, 2, sizeof(prop_str_t));
			proj->props[PROJ_PROP_DEFINES].flags |= PROP_SET | PROP_ARR;
		}

		prop_str_t unicode = PSTR("UNICODE");
		arr_app(&proj->props[PROJ_PROP_DEFINES].arr, &unicode);
		prop_str_t unicode2 = PSTR("_UNICODE");
		arr_app(&proj->props[PROJ_PROP_DEFINES].arr, &unicode2);
	}

	int ret = 0;

	xml_t xml = { 0 };
	xml_init(&xml, 256, 256);
	xml_tag_t xml_proj = xml_add_tag(&xml, -1, STR("Project"));
	xml_add_attr(&xml, xml_proj, STR("DefaultTargets"), STR("Build"));
	xml_add_attr(&xml, xml_proj, STR("xmlns"), STR("http://schemas.microsoft.com/developer/msbuild/2003"));
	xml_tag_t xml_proj_confs = xml_add_tag(&xml, xml_proj, STR("ItemGroup"));
	xml_add_attr(&xml, xml_proj_confs, STR("Label"), STR("ProjectConfigurations"));

	for (int i = archs->cnt - 1; i >= 0; i--) {
		prop_str_t *arch  = arr_get(archs, i);
		const char *platf = arch->val.data;
		size_t platf_len  = arch->val.len;
		if (cstr_eq(arch->val.data, arch->val.len, CSTR("i386"))) {
			platf	  = "Win32";
			platf_len = 5;
		}
		for (uint j = 0; j < configs->cnt; j++) {
			prop_str_t *config = arr_get(configs, j);

			xml_tag_t xml_proj_conf = xml_add_tag(&xml, xml_proj_confs, STR("ProjectConfiguration"));
			xml_add_attr(&xml, xml_proj_conf, STR("Include"), strf("%.*s|%.*s", config->val.len, config->val.data, platf_len, platf));
			xml_add_tag_val(&xml, xml_proj_conf, STR("Configuration"), strs(config->val));
			xml_add_tag_val(&xml, xml_proj_conf, STR("Platform"), strc(platf, platf_len));
		}
	}

	xml_tag_t xml_globals = xml_add_tag(&xml, xml_proj, STR("PropertyGroup"));
	xml_add_attr(&xml, xml_globals, STR("Label"), STR("Globals"));
	xml_add_tag_val(&xml, xml_globals, STR("VCProjectVersion"), STR("16.0"));
	xml_add_tag_val(&xml, xml_globals, STR("Keyword"), STR("Win32Proj"));
	xml_add_tag_val(&xml, xml_globals, STR("ProjectGuid"), strf("{%s}", shared ? proj->guid2 : proj->guid));
	xml_add_tag_val(&xml, xml_globals, STR("RootNamespace"), strs(proj->name));
	xml_add_tag_val(&xml, xml_globals, STR("WindowsTargetPlatformVersion"), STR("10.0"));

	xml_tag_t xml_import = xml_add_tag(&xml, xml_proj, STR("Import"));
	xml_add_attr(&xml, xml_import, STR("Project"), STR("$(VCTargetsPath)\\Microsoft.Cpp.Default.props"));

	typedef struct lib_type_s {
		const char *name;
		size_t len;
	} lib_type_t;

	lib_type_t config_types[__PROJ_TYPE_MAX] = {
		[PROJ_TYPE_UNKNOWN] = { CSTR("") },
		[PROJ_TYPE_EXE]	    = { CSTR("Application") },
	};

	if (shared) {
		config_types[PROJ_TYPE_LIB] = (lib_type_t){ CSTR("DynamicLibrary") };
	} else {
		config_types[PROJ_TYPE_LIB] = (lib_type_t){ CSTR("StaticLibrary") };
	}

	const struct {
		const char *name;
		size_t len;
	} charsets[] = {
		[CHARSET_UNKNOWN]    = { CSTR("") },
		[CHARSET_UNICODE]    = { CSTR("Unicode") },
		[CHARSET_MULTI_BYTE] = { CSTR("MultiByte") },
	};

	for (int i = archs->cnt - 1; i >= 0; i--) {
		prop_str_t *arch  = arr_get(archs, i);
		const char *platf = arch->val.data;
		size_t platf_len  = arch->val.len;
		if (cstr_eq(arch->val.data, arch->val.len, "i386", 3)) {
			platf	  = "Win32";
			platf_len = 5;
		}
		for (uint j = 0; j < configs->cnt; j++) {
			prop_str_t *config = arr_get(configs, j);

			const int debug	  = cstr_eq(config->val.data, config->val.len, CSTR("Debug"));
			const int release = cstr_eq(config->val.data, config->val.len, CSTR("Release"));

			xml_tag_t xml_conf = xml_add_tag(&xml, xml_proj, STR("PropertyGroup"));
			xml_add_attr(&xml, xml_conf, STR("Condition"),
				     strf("'$(Configuration)|$(Platform)'=='%.*s|%.*s'", config->val.len, config->val.data, platf_len, platf));
			xml_add_attr(&xml, xml_conf, STR("Label"), STR("Configuration"));
			xml_add_tag_val(&xml, xml_conf, STR("ConfigurationType"),
					strc(config_types[proj->props[PROJ_PROP_TYPE].mask].name, config_types[proj->props[PROJ_PROP_TYPE].mask].len));
			xml_add_tag_val(&xml, xml_conf, STR("UseDebugLibraries"), strc(debug ? "true" : "false", debug ? 4 : 5));
			xml_add_tag_val(&xml, xml_conf, STR("PlatformToolset"), STR("v143"));

			if (release) {
				xml_add_tag_val(&xml, xml_conf, STR("WholeProgramOptimization"), STR("true"));
			}

			if ((charset->flags & PROP_SET) && charset->mask != CHARSET_UNKNOWN) {
				xml_add_tag_val(&xml, xml_conf, STR("CharacterSet"), strc(charsets[charset->mask].name, charsets[charset->mask].len));
			}
		}
	}

	xml_add_attr(&xml, xml_add_tag(&xml, xml_proj, STR("Import")), STR("Project"), STR("$(VCTargetsPath)\\Microsoft.Cpp.props"));

	if (langs->mask & (1 << LANG_NASM)) {
		xml_tag_t xml_ext_set = xml_add_tag(&xml, xml_proj, STR("ImportGroup"));
		xml_add_attr(&xml, xml_ext_set, STR("Label"), STR("ExtensionSettings"));
		xml_add_attr(&xml, xml_add_tag(&xml, xml_ext_set, STR("Import")), STR("Project"), STR("$(VCTargetsPath)\\BuildCustomizations\\masm.props"));
	} else {
		xml_add_attr(&xml, xml_add_tag_val(&xml, xml_proj, STR("ImportGroup"), STR("\n")), STR("Label"), STR("ExtensionSettings"));
	}
	xml_add_attr(&xml, xml_add_tag_val(&xml, xml_proj, STR("ImportGroup"), STR("\n")), STR("Label"), STR("Shared"));

	for (int i = archs->cnt - 1; i >= 0; i--) {
		prop_str_t *arch  = arr_get(archs, i);
		const char *platf = arch->val.data;
		size_t platf_len  = arch->val.len;
		if (cstr_eq(arch->val.data, arch->val.len, CSTR("i386"))) {
			platf	  = "Win32";
			platf_len = 5;
		}
		for (uint j = 0; j < configs->cnt; j++) {
			prop_str_t *config = arr_get(configs, j);

			xml_tag_t xml_sheets = xml_add_tag(&xml, xml_proj, STR("ImportGroup"));
			xml_add_attr(&xml, xml_sheets, STR("Label"), STR("PropertySheets"));
			xml_add_attr(&xml, xml_sheets, STR("Condition"),
				     strf("'$(Configuration)|$(Platform)'=='%.*s|%.*s'", config->val.len, config->val.data, platf_len, platf));
			xml_tag_t xml_data = xml_add_tag(&xml, xml_sheets, STR("Import"));
			xml_add_attr(&xml, xml_data, STR("Project"), STR("$(UserRootDir)\\Microsoft.Cpp.$(Platform).user.props"));
			xml_add_attr(&xml, xml_data, STR("Condition"), STR("exists('$(UserRootDir)\\Microsoft.Cpp.$(Platform).user.props')"));
			xml_add_attr(&xml, xml_data, STR("Label"), STR("LocalAppDataPlatform"));
		}
	}

	xml_tag_t xml_macros = xml_add_tag(&xml, xml_proj, STR("PropertyGroup"));
	xml_add_attr(&xml, xml_macros, STR("Label"), STR("UserMacros"));

	for (int i = archs->cnt - 1; i >= 0; i--) {
		prop_str_t *arch  = arr_get(archs, i);
		const char *platf = arch->val.data;
		size_t platf_len  = arch->val.len;
		if (cstr_eq(arch->val.data, arch->val.len, CSTR("i386"))) {
			platf	  = "Win32";
			platf_len = 5;
		}
		for (uint j = 0; j < configs->cnt; j++) {
			prop_str_t *config = arr_get(configs, j);

			const int debug = cstr_eq(config->val.data, config->val.len, CSTR("Debug"));

			xml_tag_t xml_plat = xml_add_tag(&xml, xml_proj, STR("PropertyGroup"));
			xml_add_attr(&xml, xml_plat, STR("Condition"),
				     strf("'$(Configuration)|$(Platform)'=='%.*s|%.*s'", config->val.len, config->val.data, platf_len, platf));

			if (proj->props[PROJ_PROP_TYPE].mask != PROJ_TYPE_EXE) {
				xml_add_tag_val(&xml, xml_plat, STR("LinkIncremental"), strc(debug ? "true" : "false", debug ? 4 : 5));
			}

			if ((p_outdir->flags & PROP_SET)) {
				xml_add_tag_val(&xml, xml_plat, STR("OutDir"), strc(outdir, outdir_len));
			}

			if ((p_intdir->flags & PROP_SET)) {
				xml_add_tag_val(&xml, xml_plat, STR("IntDir"), strc(intdir, intdir_len));
			}
		}
	}

	for (int i = archs->cnt - 1; i >= 0; i--) {
		prop_str_t *arch  = arr_get(archs, i);
		const char *platf = arch->val.data;
		size_t platf_len  = arch->val.len;
		if (cstr_eq(arch->val.data, arch->val.len, CSTR("i386"))) {
			platf	  = "Win32";
			platf_len = 5;
		}
		for (uint j = 0; j < configs->cnt; j++) {
			prop_str_t *config = arr_get(configs, j);

			const int release = cstr_eq(config->val.data, config->val.len, CSTR("Release"));

			xml_tag_t xml_def = xml_add_tag(&xml, xml_proj, STR("ItemDefinitionGroup"));
			xml_add_attr(&xml, xml_def, STR("Condition"),
				     strf("'$(Configuration)|$(Platform)'=='%.*s|%.*s'", config->val.len, config->val.data, platf_len, platf));
			xml_tag_t xml_comp = xml_add_tag(&xml, xml_def, STR("ClCompile"));
			xml_add_tag_val(&xml, xml_comp, STR("WarningLevel"), STR("Level3"));

			if (release) {
				xml_add_tag_val(&xml, xml_comp, STR("FunctionLevelLinking"), STR("true"));
				xml_add_tag_val(&xml, xml_comp, STR("IntrinsicFunctions"), STR("true"));
			}

			xml_add_tag_val(&xml, xml_comp, STR("SDLCheck"), STR("true"));
			xml_add_tag_val(&xml, xml_comp, STR("ConformanceMode"), STR("true"));

			if ((proj->props[PROJ_PROP_INCLUDE].flags & PROP_SET) || (proj->props[PROJ_PROP_SOURCE].flags & PROP_SET) ||
			    (proj->props[PROJ_PROP_INCLUDES].flags & PROP_SET)) {
				size_t inc_len = print_includes(NULL, 0, proj, projects) + 1;
				str_t inc_data = strz(inc_len);
				inc_data.len   = print_includes((char *)inc_data.data, inc_data.size, proj, projects);
				xml_add_tag_val(&xml, xml_comp, STR("AdditionalIncludeDirectories"), inc_data);
			}

			if (proj->props[PROJ_PROP_DEFINES].flags & PROP_SET) {
				size_t def_len = print_defines(NULL, 0, proj, projects, shared) + 1;
				str_t def_data = strz(def_len);
				def_data.len   = print_defines((char *)def_data.data, def_data.size, proj, projects, shared);
				xml_add_tag_val(&xml, xml_comp, STR("PreprocessorDefinitions"), def_data);
			}

			xml_tag_t xml_link = xml_add_tag(&xml, xml_def, STR("Link"));
			xml_add_tag_val(&xml, xml_link, STR("SubSystem"), STR("Console"));

			if (release) {
				xml_add_tag_val(&xml, xml_link, STR("EnableCOMDATFolding"), STR("true"));
				xml_add_tag_val(&xml, xml_link, STR("OptimizeReferences"), STR("true"));
			}

			xml_add_tag_val(&xml, xml_link, STR("GenerateDebugInformation"), STR("true"));

			size_t dep_len = print_depends(NULL, 0, proj, projects) + 1;

			if (flags->flags & PROP_SET) {
				size_t ldf_len = print_ldflags(NULL, 0, proj, projects) + 1;
				if (ldf_len > 1) {
					str_t ldf_data = strz(ldf_len);
					ldf_data.len   = print_ldflags((char *)ldf_data.data, ldf_data.size, proj, projects);
					xml_add_tag_val(&xml, xml_link, STR("AdditionalOptions"), ldf_data);
				}

				if (dep_len <= 1 && flags->mask & (1 << FLAG_NONE)) {
					xml_add_tag(&xml, xml_link, STR("AdditionalDependencies"));
				}
			}

			if (dep_len > 1) {
				str_t dep_data = strz(dep_len);
				dep_data.len   = print_depends((char *)dep_data.data, dep_data.size, proj, projects);
				xml_add_tag_val(&xml, xml_link, STR("AdditionalDependencies"), dep_data);
			}

			if (proj->props[PROJ_PROP_TYPE].mask == PROJ_TYPE_EXE) {
				size_t libs_len = print_libs(NULL, 0, proj, projects) + 1;
				if (libs_len > 1) {
					str_t libs_data = strz(libs_len);
					libs_data.len	= print_libs((char *)libs_data.data, libs_data.size, proj, projects);
					xml_add_tag_val(&xml, xml_link, STR("AdditionalLibraryDirectories"), libs_data);
				}
			}
		}
	}

	add_file_t afd = {
		.xml   = &xml,
		.langs = langs->mask,
	};

	if (proj->props[PROJ_PROP_SOURCE].flags & PROP_SET) {
		xml_tag_t xml_srcs = xml_add_tag(&xml, xml_proj, STR("ItemGroup"));

		afd.xml_items = xml_srcs;

		const arr_t *sources = &proj->props[PROJ_PROP_SOURCE].arr;
		for (uint i = 0; i < sources->cnt; i++) {
			prop_str_t *source = arr_get(sources, i);

			path_init(&afd.path, source->val.data, source->val.len);

			path_t src = { 0 };
			path_init(&src, proj->dir.path, proj->dir.len);
			path_child_dir(&src, source->val.data, source->val.len);

			files_foreach(&src, add_src_folder, add_src_file, &afd);
		}
	}

	if (proj->props[PROJ_PROP_DEPENDS].flags & PROP_SET) {
		const arr_t *depends = &proj->all_depends;

		if (depends->cnt > 0) {
			xml_tag_t xml_refs = xml_add_tag(&xml, xml_proj, STR("ItemGroup"));

			for (uint i = 0; i < depends->cnt; i++) {
				const proj_dep_t *dep = arr_get(depends, i);

				if (dep->proj->props[PROJ_PROP_SOURCE].mask & PROP_SET) {
					path_t rel_path = { 0 };
					if (dep->link_type == LINK_TYPE_SHARED) {
						path_calc_rel(proj->gen_path.path, proj->gen_path.len, dep->proj->gen_path_d.path, dep->proj->gen_path_d.len, &rel_path);
					} else {
						path_calc_rel(proj->gen_path.path, proj->gen_path.len, dep->proj->gen_path.path, dep->proj->gen_path.len, &rel_path);
					}

					str_t inc = str_cpy(strc(rel_path.path, rel_path.len));
					convert_backslash((char *)inc.data, inc.size);

					xml_tag_t xml_ref = xml_add_tag(&xml, xml_refs, STR("ProjectReference"));
					xml_add_attr(&xml, xml_ref, STR("Include"), inc);
					xml_add_tag_val(&xml, xml_ref, STR("Project"),
							strf("{%s}", dep->link_type == LINK_TYPE_SHARED ? dep->proj->guid2 : dep->proj->guid));
				}
			}
		}
	}

	if ((proj->props[PROJ_PROP_SOURCE].flags & PROP_SET) || (proj->props[PROJ_PROP_INCLUDE].flags & PROP_SET)) {
		xml_tag_t xml_incs = xml_add_tag(&xml, xml_proj, STR("ItemGroup"));
		afd.xml_items	   = xml_incs;

		//TODO: Remove duplicating folders
		if (proj->props[PROJ_PROP_SOURCE].flags & PROP_SET) {
			const arr_t *sources = &proj->props[PROJ_PROP_SOURCE].arr;

			for (uint i = 0; i < sources->cnt; i++) {
				prop_str_t *source = arr_get(sources, i);

				path_init(&afd.path, source->val.data, source->val.len);

				path_t src = { 0 };
				path_init(&src, proj->dir.path, proj->dir.len);
				path_child(&src, source->val.data, source->val.len);

				files_foreach(&src, add_inc_folder, add_inc_file, &afd);
			}
		}

		if ((proj->props[PROJ_PROP_INCLUDE].flags & PROP_SET)) {
			const arr_t *includes = &proj->props[PROJ_PROP_INCLUDE].arr;

			for (uint i = 0; i < includes->cnt; i++) {
				prop_str_t *include = arr_get(includes, i);

				path_init(&afd.path, include->val.data, include->val.len);

				path_t inc = { 0 };
				path_init(&inc, proj->dir.path, proj->dir.len);
				path_child(&inc, include->val.data, include->val.len);

				files_foreach(&inc, add_inc_folder, add_inc_file, &afd);
			}
		}

		if (!xml_has_child(&xml, xml_incs)) {
			xml_remove_tag(&xml, xml_incs);
		}
	}

	xml_add_attr(&xml, xml_add_tag(&xml, xml_proj, STR("Import")), STR("Project"), STR("$(VCTargetsPath)\\Microsoft.Cpp.targets"));

	xml_tag_t xml_ext_tar = xml_add_tag_val(&xml, xml_proj, STR("ImportGroup"), STR("\n"));
	xml_add_attr(&xml, xml_ext_tar, STR("Label"), STR("ExtensionTargets"));
	if (langs->mask & (1 << LANG_NASM)) {
		xml_add_attr(&xml, xml_add_tag(&xml, xml_ext_tar, STR("Import")), STR("Project"), STR("$(VCTargetsPath)\\BuildCustomizations\\masm.targets"));
	}

	int dll = 0;
	for (uint i = 0; i < proj->all_depends.cnt; i++) {
		const proj_dep_t *dep = arr_get(&proj->all_depends, i);

		if (dep->link_type == LINK_TYPE_SHARED) {
			dll = 1;
			break;
		}
	}

	if (dll) {
		xml_tag_t xml_copy = xml_add_tag_val(&xml, xml_proj, STR("ItemGroup"), STR("\n"));

		for (uint i = 0; i < proj->all_depends.cnt; i++) {
			const proj_dep_t *dep = arr_get(&proj->all_depends, i);

			if (dep->link_type != LINK_TYPE_SHARED) {
				continue;
			}

			char doutdir[P_MAX_PATH] = { 0 };
			size_t doutdir_len	 = resolve(&dep->proj->props[PROJ_PROP_OUTDIR].value, CSTR(doutdir), dep->proj);
			xml_add_attr(&xml, xml_add_tag(&xml, xml_copy, STR("CopyFileToFolders")), STR("Include"),
				     strf("%.*s%.*s.d.dll", doutdir_len, doutdir, dep->proj->name.len, dep->proj->name.data));
		}
	}

	path_t gen_path = { 0 };
	path_init(&gen_path, proj->dir.path, proj->dir.len);

	if (!folder_exists(gen_path.path)) {
		folder_create(gen_path.path);
	}

	if (path_child(&gen_path, name->data, name->len) == NULL) {
		return 1;
	}

	if ((shared ? path_child_s(&gen_path, CSTR("d.vcxproj"), '.') : path_child_s(&gen_path, CSTR("vcxproj"), '.')) == NULL) {
		return 1;
	}

	FILE *file = file_open(gen_path.path, "w");
	if (file == NULL) {
		return 1;
	}

	MSG("generating project: %s", gen_path.path);

	xml_print(&xml, xml_proj, PRINT_DST_FILE(file));
	file_close(file);
	xml_free(&xml);

	xml_t xml_user = { 0 };
	xml_init(&xml_user, 16, 16);
	xml_tag_t xml_proj_user = xml_add_tag(&xml_user, -1, STR("Project"));
	xml_add_attr(&xml_user, xml_proj_user, STR("ToolsVersion"), STR("Current"));
	xml_add_attr(&xml_user, xml_proj_user, STR("xmlns"), STR("http://schemas.microsoft.com/developer/msbuild/2003"));

	if (proj->props[PROJ_PROP_ARGS].flags & PROP_SET) {
		for (int i = archs->cnt - 1; i >= 0; i--) {
			prop_str_t *arch  = arr_get(archs, i);
			const char *platf = arch->val.data;
			size_t platf_len  = arch->val.len;
			if (cstr_eq(arch->val.data, arch->val.len, CSTR("i386"))) {
				platf	  = "Win32";
				platf_len = 5;
			}
			for (uint j = 0; j < configs->cnt; j++) {
				prop_str_t *config = arr_get(configs, j);

				xml_tag_t xml_group = xml_add_tag(&xml_user, xml_proj_user, STR("PropertyGroup"));
				xml_add_attr(&xml_user, xml_group, STR("Condition"),
					     strf("'$(Configuration)|$(Platform)'=='%.*s|%.*s'", config->val.len, config->val.data, platf_len, platf));

				xml_add_tag_val(&xml_user, xml_group, STR("LocalDebuggerCommandArguments"), strs(proj->props[PROJ_PROP_ARGS].value.val));
			}
		}
	}

	xml_tag_t xml_proj_props = xml_add_tag(&xml_user, xml_proj_user, STR("PropertyGroup"));
	xml_add_tag_val(&xml_user, xml_proj_props, STR("ShowAllFiles"), STR("true"));

	path_t gen_path_user = gen_path;
	path_child_s(&gen_path_user, CSTR("user"), '.');

	file = file_open(gen_path_user.path, "w");
	if (file == NULL) {
		return 1;
	}

	ret |= xml_print(&xml_user, xml_proj_user, PRINT_DST_FILE(file)) == 0;

	file_close(file);
	xml_free(&xml_user);

	if (ret == 0) {
		SUC("generating project: %s success", gen_path_user.path);
	} else {
		ERR("generating project: %s failed", gen_path_user.path);
	}

	return ret;
}
