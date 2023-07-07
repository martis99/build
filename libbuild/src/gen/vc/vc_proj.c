#include "vc_proj.h"

#include "gen/sln.h"
#include "gen/var.h"

#include "common.h"

static const var_pol_t vars = {
	.old = {
		[VAR_SLN_DIR] = "$(SLN_DIR)",
	},
	.new = {
		[VAR_SLN_DIR] = "${workspaceFolder}",
	},
};

static size_t resolve(const prop_str_t *prop, char *dst, size_t dst_size, const proj_t *proj, const prop_str_t *config, const prop_str_t *platform, const char *outdir,
		      size_t outdir_len)
{
	size_t dst_len = prop->len;
	m_memcpy(CSTR(dst), prop->data, prop->len);

	dst_len = invert_slash(dst, dst_len);
	dst_len = cstr_inplaces(dst, dst_size, dst_len, vars.old, vars.new, __VAR_MAX);
	dst_len = cstr_inplace(dst, dst_size, dst_len, CSTR("$(PROJ_NAME)"), proj->name->data, proj->name->len);
	dst_len = cstr_inplace(dst, dst_size, dst_len, CSTR("$(PROJ_FOLDER)"), proj->rel_path.path, proj->rel_path.len);
	dst_len = cstr_inplace(dst, dst_size, dst_len, CSTR("$(CONFIG)"), config ? config->data : "", config ? config->len : 0);
	dst_len = cstr_inplace(dst, dst_size, dst_len, CSTR("$(PLATFORM)"), platform ? platform->data : "", platform ? platform->len : 0);
	dst_len = cstr_inplace(dst, dst_size, dst_len, CSTR("$(BIN)"), CSTR("$(BIN_PATH)$(BIN_FILE)"));
	dst_len = cstr_inplace(dst, dst_size, dst_len, CSTR("$(BIN_PATH)"), outdir, outdir_len);
	dst_len = cstr_inplace(dst, dst_size, dst_len, CSTR("$(BIN_FILE)"), proj->name->data, proj->name->len);

	return dst_len;
}

#define NAME_PATTERN		 "%.*s%s%.*s%s%.*s"
#define NAME_VAL(_val)		 _val ? "-" : "", _val ? ((const prop_str_t *)_val)->len : 0, _val ? ((const prop_str_t *)_val)->data : ""
#define NAME(_config, _platform) name->len, name->data, NAME_VAL(_config), NAME_VAL(_platform)

int vc_proj_gen_build(const proj_t *proj, const prop_t *sln_props, FILE *f)
{
	const prop_str_t *name = proj->name;
	proj_type_t type       = proj->props[PROJ_PROP_TYPE].mask;

	const prop_t *configs	= &sln_props[SLN_PROP_CONFIGS];
	const prop_t *platforms = &sln_props[SLN_PROP_PLATFORMS];

	if (!(configs->flags & PROP_SET) && !(platforms->flags & PROP_SET)) {
		p_fprintf(f,
			  "                {\n"
			  "                        \"label\": \"Build-" NAME_PATTERN "\",\n"
			  "                        \"type\": \"shell\",\n"
			  "                        \"command\": \"make\",\n"
			  "                        \"args\": [\n"
			  "                                \"clean\",\n"
			  "                                \"%.*s/compile\"\n"
			  "                        ],\n"
			  "                        \"group\": {\n"
			  "                                \"kind\": \"build\",\n"
			  "                                \"isDefault\": true\n"
			  "                        }\n"
			  "                }",
			  NAME(NULL, NULL), name->len, name->data);
	} else if ((configs->flags & PROP_SET) && !(platforms->flags & PROP_SET)) {
		for (uint i = 0; i < configs->arr.cnt; i++) {
			const prop_str_t *config = arr_get(&configs->arr, i);

			p_fprintf(f,
				  "                {\n"
				  "                        \"label\": \"Build-" NAME_PATTERN "\",\n"
				  "                        \"type\": \"shell\",\n"
				  "                        \"command\": \"make\",\n"
				  "                        \"args\": [\n"
				  "                                \"clean\",\n"
				  "                                \"%.*s/compile\",\n"
				  "                                \"CONFIG=%.*s\"\n"
				  "                        ],\n"
				  "                        \"group\": {\n"
				  "                                \"kind\": \"build\",\n"
				  "                                \"isDefault\": true\n"
				  "                        }\n"
				  "                }",
				  NAME(config, NULL), name->len, name->data, config->len, config->data);

			if (i < configs->arr.cnt - 1) {
				p_fprintf(f, ",\n");
			}
		}
	} else if (!(configs->flags & PROP_SET) && (platforms->flags & PROP_SET)) {
		for (uint i = 0; i < platforms->arr.cnt; i++) {
			const prop_str_t *platform = arr_get(&platforms->arr, i);

			p_fprintf(f,
				  "                {\n"
				  "                        \"label\": \"Build-" NAME_PATTERN "\",\n"
				  "                        \"type\": \"shell\",\n"
				  "                        \"command\": \"make\",\n"
				  "                        \"args\": [\n"
				  "                                \"clean\",\n"
				  "                                \"%.*s/compile\",\n"
				  "                                \"PLATFORM=%.*s\"\n"
				  "                        ],\n"
				  "                        \"group\": {\n"
				  "                                \"kind\": \"build\",\n"
				  "                                \"isDefault\": true\n"
				  "                        }\n"
				  "                }",
				  NAME(NULL, platform), name->len, name->data, platform->len, platform->data);

			if (i < platforms->arr.cnt - 1) {
				p_fprintf(f, ",\n");
			}
		}
	} else if ((configs->flags & PROP_SET) && (platforms->flags & PROP_SET)) {
		for (uint i = 0; i < configs->arr.cnt; i++) {
			const prop_str_t *config = arr_get(&configs->arr, i);

			for (uint j = 0; j < platforms->arr.cnt; j++) {
				const prop_str_t *platform = arr_get(&platforms->arr, j);

				p_fprintf(f,
					  "                {\n"
					  "                        \"label\": \"Build-" NAME_PATTERN "\",\n"
					  "                        \"type\": \"shell\",\n"
					  "                        \"command\": \"make\",\n"
					  "                        \"args\": [\n"
					  "                                \"clean\",\n"
					  "                                \"%.*s/compile\",\n"
					  "                                \"CONFIG=%.*s\",\n"
					  "                                \"PLATFORM=%.*s\"\n"
					  "                        ],\n"
					  "                        \"group\": {\n"
					  "                                \"kind\": \"build\",\n"
					  "                                \"isDefault\": true\n"
					  "                        }\n"
					  "                }",
					  NAME(config, platform), name->len, name->data, config->len, config->data, platform->len, platform->data);

				if (i < configs->arr.cnt - 1 || j < platforms->arr.cnt - 1) {
					p_fprintf(f, ",\n");
				}
			}
		}
	}

	return 0;
}

static int cppdbg(const proj_t *proj, const prop_str_t *outdir, const prop_str_t *config, const prop_str_t *platform, const prop_str_t *cwd, const prop_t *args, FILE *f)
{
	const prop_str_t *name = proj->name;

	char out[P_MAX_PATH] = { 0 };
	char buf[P_MAX_PATH] = { 0 };

	size_t out_len;
	size_t buf_len;

	out_len = resolve(outdir, CSTR(out), proj, config, platform, "", 0);

	p_fprintf(f,
		  "                {\n"
		  "                        \"name\": \"" NAME_PATTERN "\",\n"
		  "                        \"type\": \"cppdbg\",\n"
		  "                        \"request\": \"launch\",\n"
		  "                        \"program\": \"%.*s%.*s\",\n"
		  "                        \"args\": [",
		  NAME(config, platform), out_len, out, name->len, name->data);

	if (args->flags & PROP_SET) {
		p_fprintf(f, "\n");
		int end = 0;

		str_t arg = {
			.data = args->value.data,
			.len  = args->value.len,
		};
		str_t next = { 0 };

		while (!end) {
			if (str_chr(&arg, &arg, &next, ' ')) {
				str_chr(&arg, &arg, &next, '\n');
				end = 1;
			}

			const prop_str_t argp = {
				.cdata = arg.data,
				.len   = arg.len,
			};

			buf_len = resolve(&argp, CSTR(buf), proj, config, platform, out, out_len);

			p_fprintf(f, "                                \"%.*s\"%.*s\n", buf_len, buf, !end, ",");

			arg = next;
		}

		p_fprintf(f, "                        ");
	}

	p_fprintf(f,
		  "],\n"
		  "                        \"preLaunchTask\": \"Build-" NAME_PATTERN "\",\n"
		  "                        \"stopAtEntry\": false,\n"
		  "                        \"cwd\": \"${workspaceFolder}/%.*s\",\n"
		  "                        \"environment\": [],\n"
		  "                        \"externalConsole\": false,\n"
		  "                        \"MIMode\": \"gdb\",\n"
		  "                        \"setupCommands\": [\n"
		  "                                {\n"
		  "                                        \"description\": \"Enable pretty-printing for gdb\",\n"
		  "                                        \"text\": \"-enable-pretty-printing\",\n"
		  "                                        \"ignoreFailures\": true\n"
		  "                                },\n"
		  "                                {\n"
		  "                                        \"description\": \"Set Disassembly Flavor to Intel\",\n"
		  "                                        \"text\": \"-gdb-set disassembly-flavor intel\",\n"
		  "                                        \"ignoreFailures\": true\n"
		  "                                }\n"
		  "                        ]\n"
		  "                }",
		  NAME(config, platform), cwd->len, cwd->data);
}

static int f5anything(const proj_t *proj, const prop_str_t *outdir, const prop_str_t *config, const prop_str_t *platform, const prop_str_t *run, FILE *f)
{
	const prop_str_t *name = proj->name;

	char out[P_MAX_PATH] = { 0 };
	char buf[P_MAX_PATH] = { 0 };

	const size_t out_len = resolve(outdir, CSTR(out), proj, config, platform, "", 0);
	const size_t buf_len = resolve(run, CSTR(buf), proj, config, platform, out, out_len);

	p_fprintf(f,
		  "                {\n"
		  "                        \"name\": \"" NAME_PATTERN "\",\n"
		  "                        \"type\": \"f5anything\",\n"
		  "                        \"request\": \"launch\",\n"
		  "                        \"preLaunchTask\": \"Build-" NAME_PATTERN "\",\n"
		  "                        \"command\": \"%.*s\",\n"
		  "                }",
		  NAME(config, platform), NAME(config, platform), buf_len, buf);
}

int vc_proj_gen_launch(const proj_t *proj, const hashmap_t *projects, const prop_t *sln_props, FILE *f)
{
	const prop_str_t *name = proj->name;
	proj_type_t type       = proj->props[PROJ_PROP_TYPE].mask;

	if (type != PROJ_TYPE_EXE) {
		return 0;
	}

	const prop_t *configs	= &sln_props[SLN_PROP_CONFIGS];
	const prop_t *platforms = &sln_props[SLN_PROP_PLATFORMS];
	const prop_t *run	= &proj->props[PROJ_PROP_RUN];
	const prop_t *wdir	= &proj->props[PROJ_PROP_WDIR];
	const prop_t *args	= &proj->props[PROJ_PROP_ARGS];

	prop_str_t rel_path = {
		.cdata = proj->rel_path.path,
		.len   = proj->rel_path.len,
	};

	const prop_str_t *outdir = &proj->props[PROJ_PROP_OUTDIR].value;
	const prop_str_t *cwd	 = (wdir->flags & PROP_SET) ? &wdir->value : &rel_path;

	if (!(configs->flags & PROP_SET) && !(platforms->flags & PROP_SET)) {
		if (run->flags & PROP_SET) {
			f5anything(proj, outdir, NULL, NULL, &run->value, f);
		} else {
			cppdbg(proj, outdir, NULL, NULL, cwd, args, f);
		}
	} else if ((configs->flags & PROP_SET) && !(platforms->flags & PROP_SET)) {
		for (uint i = 0; i < configs->arr.cnt; i++) {
			const prop_str_t *config = arr_get(&configs->arr, i);

			if (run->flags & PROP_SET) {
				f5anything(proj, outdir, config, NULL, &run->value, f);
			} else {
				cppdbg(proj, outdir, config, NULL, cwd, args, f);
			}

			if (i < configs->arr.cnt - 1) {
				p_fprintf(f, ",\n");
			}
		}
	} else if (!(configs->flags & PROP_SET) && (platforms->flags & PROP_SET)) {
		for (uint i = 0; i < platforms->arr.cnt; i++) {
			const prop_str_t *platform = arr_get(&platforms->arr, i);

			if (run->flags & PROP_SET) {
				f5anything(proj, outdir, NULL, platform, &run->value, f);
			} else {
				cppdbg(proj, outdir, NULL, platform, cwd, args, f);
			}

			if (i < platforms->arr.cnt - 1) {
				p_fprintf(f, ",\n");
			}
		}
	} else if ((platforms->flags & PROP_SET) && (configs->flags & PROP_SET)) {
		for (uint i = 0; i < configs->arr.cnt; i++) {
			const prop_str_t *config = arr_get(&configs->arr, i);

			for (uint j = 0; j < platforms->arr.cnt; j++) {
				const prop_str_t *platform = arr_get(&platforms->arr, j);

				if (run->flags & PROP_SET) {
					f5anything(proj, outdir, config, platform, &run->value, f);
				} else {
					cppdbg(proj, outdir, config, platform, cwd, args, f);
				}

				if (i < configs->arr.cnt - 1 || j < platforms->arr.cnt - 1) {
					p_fprintf(f, ",\n");
				}
			}
		}
	}

	return 0;
}
