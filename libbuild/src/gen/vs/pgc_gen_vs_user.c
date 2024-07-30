#include "gen/vs/pgc_gen_vs_user.h"

#include "gen/pgc_common.h"
#include "pgc_gen_vs_common.h"

xml_tag_t pgc_gen_vs_user(const pgc_t *pgc, xml_t *xml)
{
	xml_tag_t proj = xml_add_tag(xml, -1, STR("Project"));
	xml_add_attr(xml, proj, STR("ToolsVersion"), STR("Current"));
	xml_add_attr(xml, proj, STR("xmlns"), STR("http://schemas.microsoft.com/developer/msbuild/2003"));

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

			xml_tag_t group = xml_add_tag(xml, proj, STR("PropertyGroup"));
			xml_add_attr(xml, group, STR("Condition"), strf("'$(Configuration)|$(Platform)'=='%.*s|%.*s'", conf->len, conf->data, plat.len, plat.data));

			xml_add_tag_val(xml, group, STR("LocalDebuggerCommandArguments"), str_cpy(pgc->str[PGC_STR_ARGS]));
		}
	}

	xml_tag_t props = xml_add_tag(xml, proj, STR("PropertyGroup"));
	xml_add_tag_val(xml, props, STR("ShowAllFiles"), STR("true"));

	return proj;
}