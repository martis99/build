#include "gen/vs/pgc_gen_vs_proj.h"

#include "gen/pgc_common.h"
#include "pgc_gen_vs_common.h"

#include "cstr.h"
#include "file.h"
#include "path.h"

typedef struct add_file_s {
	path_t path;
	xml_t *xml;
	xml_tag_t xml_items;
	uint exts;
} add_file_t;

static int add_src_file(path_t *path, const char *file, void *priv)
{
	add_file_t *data = priv;

	path_t new_path = data->path;
	path_child(&new_path, file, cstr_len(file));

	int add = ((data->exts & F_PGC_SRC_S) && path_ends(&new_path, CSTR(".S"))) || ((data->exts & F_PGC_SRC_NASM) && path_ends(&new_path, CSTR(".nasm"))) ||
		  ((data->exts & F_PGC_SRC_C) && path_ends(&new_path, CSTR(".c"))) || ((data->exts & F_PGC_SRC_CPP) && path_ends(&new_path, CSTR(".cpp")));

	if (!add) {
		return 0;
	}

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

	int add = ((data->exts & F_PGC_HEADER_INC) && path_ends(&new_path, CSTR(".inc"))) || ((data->exts & F_PGC_HEADER_H) && path_ends(&new_path, CSTR(".h"))) ||
		  ((data->exts & F_PGC_HEADER_HPP) && path_ends(&new_path, CSTR(".hpp")));

	if (add) {
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

xml_tag_t pgc_gen_vs_local(const pgc_t *pgc, xml_t *xml, pgc_build_type_t b)
{
	xml_tag_t proj = xml_add_tag(xml, -1, STR("Project"));
	xml_add_attr(xml, proj, STR("DefaultTargets"), STR("Build"));
	xml_add_attr(xml, proj, STR("xmlns"), STR("http://schemas.microsoft.com/developer/msbuild/2003"));
	xml_tag_t proj_confs = xml_add_tag(xml, proj, STR("ItemGroup"));
	xml_add_attr(xml, proj_confs, STR("Label"), STR("ProjectConfigurations"));

	const str_t *arch;
	arr_foreach(&pgc->arr[PGC_ARR_ARCHS], arch)
	{
		if (arch->data == NULL) {
			continue;
		}

		str_t plat = arch_to_plat(*arch);

		const str_t *conf;
		arr_foreach(&pgc->arr[PGC_ARR_CONFIGS], conf)
		{
			if (conf->data == NULL) {
				continue;
			}

			xml_tag_t proj_conf = xml_add_tag(xml, proj_confs, STR("ProjectConfiguration"));
			xml_add_attr(xml, proj_conf, STR("Include"), strf("%.*s|%.*s", conf->len, conf->data, plat.len, plat.data));
			xml_add_tag_val(xml, proj_conf, STR("Configuration"), str_cpy(*conf));
			xml_add_tag_val(xml, proj_conf, STR("Platform"), str_cpy(plat));
		}
	}

	xml_tag_t globals = xml_add_tag(xml, proj, STR("PropertyGroup"));
	xml_add_attr(xml, globals, STR("Label"), STR("Globals"));
	xml_add_tag_val(xml, globals, STR("VCProjectVersion"), STR("17.0"));
	xml_add_tag_val(xml, globals, STR("Keyword"), STR("Win32Proj"));
	xml_add_tag_val(xml, globals, STR("ProjectGuid"), strf("{%s}", pgc->str[PGC_STR_GUID]));
	xml_add_tag_val(xml, globals, STR("RootNamespace"), str_cpy(pgc->intdir[PGC_INTDIR_STR_NAME][s_build_c[b].intdir]));
	xml_add_tag_val(xml, globals, STR("WindowsTargetPlatformVersion"), STR("10.0"));

	xml_tag_t import = xml_add_tag(xml, proj, STR("Import"));
	xml_add_attr(xml, import, STR("Project"), STR("$(VCTargetsPath)\\Microsoft.Cpp.Default.props"));

	static const struct {
		str_t config;
	} build_c[__PGC_BUILD_TYPE_MAX] = {
		[PGC_BUILD_EXE]	   = { STRS("Application") },
		[PGC_BUILD_STATIC] = { STRS("StaticLibrary") },
		[PGC_BUILD_SHARED] = { STRS("DynamicLibrary") },
	};

	arr_foreach(&pgc->arr[PGC_ARR_ARCHS], arch)
	{
		if (arch->data == NULL) {
			continue;
		}

		str_t plat = arch_to_plat(*arch);

		const str_t *conf;
		arr_foreach(&pgc->arr[PGC_ARR_CONFIGS], conf)
		{
			if (conf->data == NULL) {
				continue;
			}

			const int debug	  = str_eq(*conf, STR("Debug"));
			const int release = str_eq(*conf, STR("Release"));

			xml_tag_t xconf = xml_add_tag(xml, proj, STR("PropertyGroup"));
			xml_add_attr(xml, xconf, STR("Condition"), strf("'$(Configuration)|$(Platform)'=='%.*s|%.*s'", conf->len, conf->data, plat.len, plat.data));
			xml_add_attr(xml, xconf, STR("Label"), STR("Configuration"));
			xml_add_tag_val(xml, xconf, STR("ConfigurationType"), build_c[b].config);
			xml_add_tag_val(xml, xconf, STR("UseDebugLibraries"), strc(debug ? "true" : "false", debug ? 4 : 5));
			xml_add_tag_val(xml, xconf, STR("PlatformToolset"), STR("v143"));

			if (release) {
				xml_add_tag_val(xml, xconf, STR("WholeProgramOptimization"), STR("true"));
			}

			xml_add_tag_val(xml, xconf, STR("CharacterSet"), STR("Unicode"));
		}
	}

	xml_add_attr(xml, xml_add_tag(xml, proj, STR("Import")), STR("Project"), STR("$(VCTargetsPath)\\Microsoft.Cpp.props"));

	int is_nasm = 0;

	const pgc_str_flags_t *data;
	arr_foreach(&pgc->arr[PGC_ARR_SRCS], data)
	{
		if (!(data->flags & F_PGC_SRC_NASM)) {
			continue;
		}

		is_nasm = 1;
	}

	if (is_nasm) {
		xml_tag_t ext_set = xml_add_tag(xml, proj, STR("ImportGroup"));
		xml_add_attr(xml, ext_set, STR("Label"), STR("ExtensionSettings"));
		xml_add_attr(xml, xml_add_tag(xml, ext_set, STR("Import")), STR("Project"), STR("$(VCTargetsPath)\\BuildCustomizations\\masm.props"));
	} else {
		xml_add_attr(xml, xml_add_tag_val(xml, proj, STR("ImportGroup"), STR("\n")), STR("Label"), STR("ExtensionSettings"));
	}

	xml_add_attr(xml, xml_add_tag_val(xml, proj, STR("ImportGroup"), STR("\n")), STR("Label"), STR("Shared"));

	arr_foreach(&pgc->arr[PGC_ARR_ARCHS], arch)
	{
		if (arch->data == NULL) {
			continue;
		}

		str_t plat = arch_to_plat(*arch);

		const str_t *conf;
		arr_foreach(&pgc->arr[PGC_ARR_CONFIGS], conf)
		{
			if (conf->data == NULL) {
				continue;
			}

			xml_tag_t sheets = xml_add_tag(xml, proj, STR("ImportGroup"));
			xml_add_attr(xml, sheets, STR("Label"), STR("PropertySheets"));
			xml_add_attr(xml, sheets, STR("Condition"), strf("'$(Configuration)|$(Platform)'=='%.*s|%.*s'", conf->len, conf->data, plat.len, plat.data));
			xml_tag_t data = xml_add_tag(xml, sheets, STR("Import"));
			xml_add_attr(xml, data, STR("Project"), STR("$(UserRootDir)\\Microsoft.Cpp.$(Platform).user.props"));
			xml_add_attr(xml, data, STR("Condition"), STR("exists('$(UserRootDir)\\Microsoft.Cpp.$(Platform).user.props')"));
			xml_add_attr(xml, data, STR("Label"), STR("LocalAppDataPlatform"));
		}
	}

	xml_tag_t macros = xml_add_tag(xml, proj, STR("PropertyGroup"));
	xml_add_attr(xml, macros, STR("Label"), STR("UserMacros"));

	arr_foreach(&pgc->arr[PGC_ARR_ARCHS], arch)
	{
		if (arch->data == NULL) {
			continue;
		}

		str_t plat = arch_to_plat(*arch);

		const str_t *conf;
		arr_foreach(&pgc->arr[PGC_ARR_CONFIGS], conf)
		{
			if (conf->data == NULL) {
				continue;
			}

			const int debug = str_eq(*conf, STR("Debug"));

			xml_tag_t xplat = xml_add_tag(xml, proj, STR("PropertyGroup"));
			xml_add_attr(xml, xplat, STR("Condition"), strf("'$(Configuration)|$(Platform)'=='%.*s|%.*s'", conf->len, conf->data, plat.len, plat.data));

			if (b != PGC_BUILD_EXE) {
				xml_add_tag_val(xml, xplat, STR("LinkIncremental"), strc(debug ? "true" : "false", debug ? 4 : 5));
			}

			if (pgc->str[PGC_STR_OUTDIR].data) {
				xml_add_tag_val(xml, xplat, STR("OutDir"), str_cpy(pgc->str[PGC_STR_OUTDIR]));
			}

			if (pgc->str[PGC_INTDIR_STR_INTDIR].data) {
				xml_add_tag_val(xml, xplat, STR("IntDir"), str_cpy(pgc->str[PGC_INTDIR_STR_INTDIR]));
			}
		}
	}

	arr_foreach(&pgc->arr[PGC_ARR_ARCHS], arch)
	{
		if (arch->data == NULL) {
			continue;
		}

		str_t plat = arch_to_plat(*arch);

		const str_t *conf;
		arr_foreach(&pgc->arr[PGC_ARR_CONFIGS], conf)
		{
			if (conf->data == NULL) {
				continue;
			}

			const int release = str_eq(*conf, STR("Release"));

			xml_tag_t def = xml_add_tag(xml, proj, STR("ItemDefinitionGroup"));
			xml_add_attr(xml, def, STR("Condition"), strf("'$(Configuration)|$(Platform)'=='%.*s|%.*s'", conf->len, conf->data, plat.len, plat.data));
			xml_tag_t comp = xml_add_tag(xml, def, STR("ClCompile"));
			xml_add_tag_val(xml, comp, STR("WarningLevel"), STR("Level3"));

			if (release) {
				xml_add_tag_val(xml, comp, STR("FunctionLevelLinking"), STR("true"));
				xml_add_tag_val(xml, comp, STR("IntrinsicFunctions"), STR("true"));
			}

			xml_add_tag_val(xml, comp, STR("SDLCheck"), STR("true"));
			xml_add_tag_val(xml, comp, STR("ConformanceMode"), STR("true"));

			str_t includes = { 0 };
			const pgc_str_flags_t *include;
			arr_foreach(&pgc->arr[PGC_ARR_INCLUDES], include)
			{
				if (include->str.data == NULL) {
					continue;
				}

				switch (include->flags) {
				case PGC_SCOPE_PRIVATE:
				case PGC_SCOPE_PUBLIC:
					if (includes.data == 0) {
						includes = strz(16);
					} else {
						str_cat(&includes, STR(";"));
					}
					str_cat(&includes, include->str);

					break;
				default:
					break;
				}
			}
			if (includes.data) {
				xml_add_tag_val(xml, comp, STR("AdditionalIncludeDirectories"), includes);
			}

			str_t defines = { 0 };
			const pgc_str_flags_t *define;
			arr_foreach(&pgc->arr[PGC_ARR_DEFINES], define)
			{
				if (!(define->flags & (1 << s_build_c[b].intdir)) || define->str.data == NULL) {
					continue;
				}

				if (defines.data == 0) {
					defines = strz(16);
				} else {
					str_cat(&defines, STR(";"));
				}
				str_cat(&defines, define->str);
			}
			if (defines.data) {
				xml_add_tag_val(xml, comp, STR("PreprocessorDefinitions"), defines);
			}

			xml_tag_t link = xml_add_tag(xml, def, STR("Link"));
			xml_add_tag_val(xml, link, STR("SubSystem"), STR("Console"));

			if (release) {
				xml_add_tag_val(xml, link, STR("EnableCOMDATFolding"), STR("true"));
				xml_add_tag_val(xml, link, STR("OptimizeReferences"), STR("true"));
			}

			xml_add_tag_val(xml, link, STR("GenerateDebugInformation"), STR("true"));

			str_t libdirs = { 0 };
			const pgc_str_flags_t *libdir;
			arr_foreach(&pgc->arr[PGC_ARR_LIBS], libdir)
			{
				if (!(libdir->flags & (1 << s_build_c[b].intdir)) || libdir->str.data == NULL) {
					continue;
				}

				if (libdirs.data == 0) {
					libdirs = strz(16);
				} else {
					str_cat(&libdirs, STR(";"));
				}
				str_cat(&libdirs, libdir->str);
			}
			if (libdirs.data) {
				xml_add_tag_val(xml, link, STR("AdditionalLibraryDirectories"), libdirs);
			}
		}
	}

	add_file_t afd = {
		.xml = xml,
	};

	if (pgc->arr[PGC_ARR_SRCS].cnt > 0) {
		xml_tag_t srcs = xml_add_tag(xml, proj, STR("ItemGroup"));

		afd.xml_items = srcs;

		const pgc_str_flags_t *data;
		arr_foreach(&pgc->arr[PGC_ARR_SRCS], data)
		{
			afd.exts = data->flags;

			path_init(&afd.path, data->str.data, data->str.len);

			path_t src = { 0 };
			path_init(&src, pgc->str[PGC_STR_DIR].data, pgc->str[PGC_STR_DIR].len);
			path_child_dir(&src, data->str.data, data->str.len);

			files_foreach(&src, add_src_folder, add_src_file, &afd);
		}
	}

	if (pgc->arr[PGC_ARR_DEPENDS].cnt > 0) {
		xml_tag_t refs = xml_add_tag(xml, proj, STR("ItemGroup"));
		const pgc_depend_data_t *depend;
		arr_foreach(&pgc->arr[PGC_ARR_DEPENDS], depend)
		{
			path_t inc = { 0 };
			path_calc_rel(pgc->str[PGC_STR_RELDIR].data, pgc->str[PGC_STR_RELDIR].len, depend->rel_dir.data, depend->rel_dir.len, &inc);

			xml_tag_t xml_ref = xml_add_tag(xml, refs, STR("ProjectReference"));
			xml_add_attr(xml, xml_ref, STR("Include"), str_cpy(strc(inc.path, inc.len)));
			xml_add_tag_val(xml, xml_ref, STR("Project"), strf("{%s}", depend->guid));
		}
	}

	if (pgc->arr[PGC_ARR_INCLUDES].cnt > 0) {
		xml_tag_t incs = xml_add_tag(xml, proj, STR("ItemGroup"));

		afd.xml_items = incs;

		const pgc_str_flags_t *data;
		arr_foreach(&pgc->arr[PGC_ARR_INCLUDES], data)
		{
			afd.exts = data->flags;

			path_init(&afd.path, data->str.data, data->str.len);

			path_t src = { 0 };
			path_init(&src, pgc->str[PGC_STR_DIR].data, pgc->str[PGC_STR_DIR].len);
			path_child_dir(&src, data->str.data, data->str.len);

			files_foreach(&src, add_inc_folder, add_inc_file, &afd);
		}
	}

	xml_add_attr(xml, xml_add_tag(xml, proj, STR("Import")), STR("Project"), STR("$(VCTargetsPath)\\Microsoft.Cpp.targets"));

	xml_tag_t ext_tar = xml_add_tag_val(xml, proj, STR("ImportGroup"), STR("\n"));
	xml_add_attr(xml, ext_tar, STR("Label"), STR("ExtensionTargets"));
	if (is_nasm) {
		xml_add_attr(xml, xml_add_tag(xml, ext_tar, STR("Import")), STR("Project"), STR("$(VCTargetsPath)\\BuildCustomizations\\masm.targets"));
	}

	if (pgc->arr[PGC_ARR_COPYFILES].cnt > 0) {
		xml_tag_t copy = xml_add_tag_val(xml, proj, STR("ItemGroup"), STR("\n"));

		const pgc_str_flags_t *copyfile;
		arr_foreach(&pgc->arr[PGC_ARR_COPYFILES], copyfile)
		{
			if (!(copyfile->flags & (1 << s_build_c[b].intdir)) || copyfile->str.data == NULL) {
				continue;
			}

			xml_add_attr(xml, xml_add_tag(xml, copy, STR("CopyFileToFolders")), STR("Include"), str_cpy(copyfile->str));
		}
	}

	return proj;
}

xml_tag_t pgc_gen_vs_remote(const pgc_t *pgc, xml_t *xml)
{
	xml_tag_t proj = xml_add_tag(xml, -1, STR("Project"));
	xml_add_attr(xml, proj, STR("DefaultTargets"), STR("Build"));
	xml_add_attr(xml, proj, STR("xmlns"), STR("http://schemas.microsoft.com/developer/msbuild/2003"));

	return proj;
}

xml_tag_t pgc_gen_vs_proj(const pgc_t *pgc, xml_t *xml, pgc_build_type_t type)
{
	if (pgc == NULL || xml == NULL) {
		return XML_END;
	}

	if (pgc->str[PGC_STR_URL].data) {
		return pgc_gen_vs_remote(pgc, xml);
	}

	return pgc_gen_vs_local(pgc, xml, type);
}
