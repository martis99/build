#include "gen/cm/cmake.h"

#include "log.h"

#include <stdarg.h>

typedef struct cmake_cmd_s {
	cmake_cmd_type_t type;
	uint name;
	int flag;
	lnode_t args;
} cmake_cmd_t;

static const char *cmd_type_str[] = {
	[CMAKE_CMD_FILE]		  = "file",
	[CMAKE_CMD_ADD_EXE]		  = "add_executable",
	[CMAKE_CMD_ADD_LIB]		  = "add_library",
	[CMAKE_CMD_ADD_CUSTOM_TARGET]	  = "add_custom_target",
	[CMAKE_CMD_ADD_CUSTOM_CMD_TARGET] = "add_custom_command",
	[CMAKE_CMD_ADD_DEPENDS]		  = "add_dependencies",
	[CMAKE_CMD_TARGET_DEFINES]	  = "target_compile_definitions",
	[CMAKE_CMD_TARGET_COMPILE_OPTS]	  = "target_compile_options",
	[CMAKE_CMD_TARGET_INCLUDE_DIRS]	  = "target_include_directories",
	[CMAKE_CMD_TARGET_LINK_DIRS]	  = "target_link_directories",
	[CMAKE_CMD_TARGET_LINK_LIBS]	  = "target_link_libraries",
	[CMAKE_CMD_TARGET_LINK_OPTS]	  = "target_link_options",
	[CMAKE_CMD_SET_TARGET_PROPS]	  = "set_target_properties",
};

static const char *file_act_str[] = {
	[CMAKE_FILE_GLOB_RECURSE] = "GLOB_RECURSE",
};

static const char *lib_type_str[] = {
	[CMAKE_LIB_STATIC] = "STATIC",
	[CMAKE_LIB_SHARED] = "SHARED",
};

static const char *custom_cmd_target_run_type_str[] = {
	[CMAKE_CMD_TARGET_PRE_BUILD]  = "PRE_BUILD",
	[CMAKE_CMD_TARGET_PRE_LINK]   = "PRE_LINK",
	[CMAKE_CMD_TARGET_POST_BUILD] = "POST_BUILD",
};

static const char *scope_str[] = {
	[CMAKE_SCOPE_PRIVATE]	= "PRIVATE",
	[CMAKE_SCOPE_PUBLIC]	= "PUBLIC",
	[CMAKE_SCOPE_INTERFACE] = "INTERFACE",
};

static const char *target_prop_str[] = {
	[CMAKE_TARGET_PROP_ARC_OUT_DIR]	    = "ARCHIVE_OUTPUT_DIRECTORY",
	[CMAKE_TARGET_PROP_LIB_OUT_DIR]	    = "LIBRARY_OUTPUT_DIRECTORY",
	[CMAKE_TARGET_PROP_RUN_OUT_DIR]	    = "RUNTIME_OUTPUT_DIRECTORY",
	[CMAKE_TARGET_PROP_ARC_OUT_DIR_DBG] = "ARCHIVE_OUTPUT_DIRECTORY_DEBUG",
	[CMAKE_TARGET_PROP_LIB_OUT_DIR_DBG] = "LIBRARY_OUTPUT_DIRECTORY_DEBUG",
	[CMAKE_TARGET_PROP_RUN_OUT_DIR_DBG] = "RUNTIME_OUTPUT_DIRECTORY_DEBUG",
	[CMAKE_TARGET_PROP_ARC_OUT_DIR_RLS] = "ARCHIVE_OUTPUT_DIRECTORY_RELEASE",
	[CMAKE_TARGET_PROP_LIB_OUT_DIR_RLS] = "LIBRARY_OUTPUT_DIRECTORY_RELEASE",
	[CMAKE_TARGET_PROP_RUN_OUT_DIR_RLS] = "RUNTIME_OUTPUT_DIRECTORY_RELEASE",
	[CMAKE_TARGET_PROP_BUILD_RPATH]	    = "BUILD_RPATH",
	[CMAKE_TARGET_PROP_OUTPUT_NAME]	    = "OUTPUT_NAME",
	[CMAKE_TARGET_PROP_PREFIX]	    = "PREFIX",
};

cmake_t *cmake_init(cmake_t *cmake, uint strs_cap, uint cmds_cap, uint args_cap)
{
	if (cmake == NULL) {
		return NULL;
	}

	if (arr_init(&cmake->strs, strs_cap, sizeof(str_t)) == NULL) {
		return NULL;
	}

	if (arr_init(&cmake->cmds, cmds_cap, sizeof(cmake_cmd_t)) == NULL) {
		return NULL;
	}

	if (list_init(&cmake->args, args_cap, sizeof(cmake_cmd_arg_t)) == NULL) {
		return NULL;
	}

	return cmake;
}

void cmake_free(cmake_t *cmake)
{
	if (cmake == NULL) {
		return;
	}

	str_t *str;
	arr_foreach(&cmake->strs, str)
	{
		str_free(str);
	}
	arr_free(&cmake->strs);

	arr_free(&cmake->cmds);
	list_free(&cmake->args);
}

static uint add_str(cmake_t *cmake, str_t str)
{
	arr_add_t(str_t, &cmake->strs, str, id);
	return id;
}

static uint add_cmd(cmake_t *cmake, cmake_cmd_type_t type, uint name, int flag, size_t n, ...)
{
	cmake_cmd_t cmd = {
		.type = type,
		.name = name,
		.flag = flag,
		.args = LIST_END,
	};

	va_list args;
	va_start(args, n);
	for (size_t i = 0; i < n; i++) {
		lnode_t arg;
		list_add_next_node(&cmake->args, cmd.args, arg);
		cmake_cmd_arg_t *data = list_get_data(&cmake->args, arg);

		if (data == NULL) {
			return CMAKE_END;
		}

		*data = va_arg(args, cmake_cmd_arg_t);
	}
	va_end(args);

	arr_add_t(cmake_cmd_t, &cmake->cmds, cmd, id);

	return id;
}

static uint add_target_cmd(cmake_t *cmake, cmake_cmd_type_t cmd, uint target, int flag)
{
	if (cmake == NULL) {
		return CMAKE_END;
	}

	cmake_cmd_t *data = arr_get(&cmake->cmds, target);
	if (data == NULL) {
		log_error("build", "cmake", NULL, "target not found: %d", target);
		return CMAKE_END;
	}

	return add_cmd(cmake, cmd, data->name, flag, 0);
}

static cmake_cmd_arg_t arg_val(uint val)
{
	return (cmake_cmd_arg_t){ .val = val };
}

static cmake_cmd_arg_t arg_str(cmake_t *cmake, str_t str)
{
	return (cmake_cmd_arg_t){ .val = add_str(cmake, str) };
}

uint cmake_cmd_add_str(cmake_t *cmake, uint cmd, str_t str)
{
	return cmake_cmd_add_arg(cmake, cmd, arg_str(cmake, str));
}

uint cmake_cmd_add_arg(cmake_t *cmake, uint cmd, cmake_cmd_arg_t arg)
{
	if (cmake == NULL) {
		return CMAKE_END;
	}

	cmake_cmd_t *data = arr_get(&cmake->cmds, cmd);
	if (data == NULL) {
		log_error("build", "cmake", NULL, "command not found: %d", cmd);
		return CMAKE_END;
	}

	lnode_t node;
	list_add_next_node(&cmake->args, data->args, node);
	cmake_cmd_arg_t *arg_data = list_get_data(&cmake->args, node);

	if (arg_data == NULL) {
		return CMAKE_END;
	}

	*arg_data = arg;
	return cmd;
}

uint cmake_file(cmake_t *cmake, cmake_file_act_t act, str_t name)
{
	if (cmake == NULL) {
		return CMAKE_END;
	}

	return add_cmd(cmake, CMAKE_CMD_FILE, add_str(cmake, name), act, 0);
}

uint cmake_add_exe(cmake_t *cmake, str_t name, uint sources)
{
	if (cmake == NULL) {
		return CMAKE_END;
	}

	cmake_cmd_t *file = arr_get(&cmake->cmds, sources);
	if (file == NULL) {
		log_error("build", "cmake", NULL, "sources not found: %d", sources);
		return CMAKE_END;
	}

	return add_cmd(cmake, CMAKE_CMD_ADD_EXE, add_str(cmake, name), 0, 1, arg_val(file->name));
}

uint cmake_add_lib(cmake_t *cmake, str_t name, cmake_lib_type_t type, uint sources)
{
	if (cmake == NULL) {
		return CMAKE_END;
	}

	cmake_cmd_t *file = arr_get(&cmake->cmds, sources);
	if (file == NULL) {
		log_error("build", "cmake", NULL, "sources not found: %d", sources);
		return CMAKE_END;
	}

	return add_cmd(cmake, CMAKE_CMD_ADD_LIB, add_str(cmake, name), type, 1, arg_val(file->name));
}

uint cmake_add_custom_target(cmake_t *cmake, str_t name)
{
	if (cmake == NULL) {
		return CMAKE_END;
	}

	return add_cmd(cmake, CMAKE_CMD_ADD_CUSTOM_TARGET, add_str(cmake, name), 0, 0);
}

uint cmake_add_custom_cmd_target(cmake_t *cmake, uint target, cmake_cmd_target_run_type_t run_type, str_t cmd)
{
	if (cmake == NULL) {
		return CMAKE_END;
	}

	cmake_cmd_t *data = arr_get(&cmake->cmds, target);
	if (data == NULL) {
		log_error("build", "cmake", NULL, "target not found: %d", target);
		return CMAKE_END;
	}

	return add_cmd(cmake, CMAKE_CMD_ADD_CUSTOM_CMD_TARGET, data->name, run_type, 1, arg_str(cmake, cmd));
}

uint cmake_add_depends(cmake_t *cmake, uint target)
{
	return add_target_cmd(cmake, CMAKE_CMD_ADD_DEPENDS, target, 0);
}

uint cmake_target_cmd(cmake_t *cmake, cmake_cmd_type_t cmd, uint target, cmake_scope_t scope)
{
	return add_target_cmd(cmake, cmd, target, scope);
}

uint cmake_set_target_props(cmake_t *cmake, uint target)
{
	return add_target_cmd(cmake, CMAKE_CMD_SET_TARGET_PROPS, target, 0);
}

uint cmake_add_target_prop(cmake_t *cmake, uint props, cmake_target_prop_t prop, str_t val)
{
	if (cmake == NULL) {
		return CMAKE_END;
	}

	cmake_cmd_t *data = arr_get(&cmake->cmds, props);
	if (data == NULL) {
		log_error("build", "cmake", NULL, "command not found: %d", props);
		return CMAKE_END;
	}

	lnode_t node;
	list_add_next_node(&cmake->args, data->args, node);
	cmake_cmd_arg_t *arg_data = list_get_data(&cmake->args, node);

	if (arg_data == NULL) {
		return CMAKE_END;
	}

	*arg_data = (cmake_cmd_arg_t){
		.flag = prop,
		.val  = add_str(cmake, val),
	};

	return props;
}

int cmake_print(const cmake_t *cmake, print_dst_t dst)
{
	if (cmake == NULL) {
		return 0;
	}

	int off = dst.off;

	const cmake_cmd_t *cmd;
	arr_foreach(&cmake->cmds, cmd)
	{
		dst.off += dprintf(dst, cmd_type_str[cmd->type]);
		dst.off += dprintf(dst, "(");
		switch (cmd->type) {
		case CMAKE_CMD_FILE: {
			str_t *name = arr_get(&cmake->strs, cmd->name);
			dst.off += dprintf(dst, "%s %.*s", file_act_str[cmd->flag], name->len, name->data);
			const cmake_cmd_arg_t *arg;
			list_foreach(&cmake->args, cmd->args, arg)
			{
				str_t *str = arr_get(&cmake->strs, arg->val);
				dst.off += dprintf(dst, " %.*s", str->len, str->data);
			}
			break;
		}
		case CMAKE_CMD_ADD_EXE: {
			str_t *name = arr_get(&cmake->strs, cmd->name);
			dst.off += dprintf(dst, "%.*s", name->len, name->data);
			const cmake_cmd_arg_t *arg;
			list_foreach(&cmake->args, cmd->args, arg)
			{
				str_t *str = arr_get(&cmake->strs, arg->val);
				dst.off += dprintf(dst, " ${%.*s}", str->len, str->data);
			}
			break;
		}
		case CMAKE_CMD_ADD_LIB: {
			str_t *name = arr_get(&cmake->strs, cmd->name);
			dst.off += dprintf(dst, "%.*s %s", name->len, name->data, lib_type_str[cmd->flag]);
			const cmake_cmd_arg_t *arg;
			list_foreach(&cmake->args, cmd->args, arg)
			{
				str_t *str = arr_get(&cmake->strs, arg->val);
				dst.off += dprintf(dst, " ${%.*s}", str->len, str->data);
			}
			break;
		}
		case CMAKE_CMD_ADD_CUSTOM_TARGET: {
			str_t *name = arr_get(&cmake->strs, cmd->name);
			dst.off += dprintf(dst, "%.*s", name->len, name->data);
			break;
		}
		case CMAKE_CMD_ADD_CUSTOM_CMD_TARGET: {
			str_t *target	     = arr_get(&cmake->strs, cmd->name);
			cmake_cmd_arg_t *arg = list_get_data(&cmake->args, cmd->args);
			str_t *arg_val	     = arr_get(&cmake->strs, arg->val);
			if (arg_val == NULL || arg_val->data == NULL) {
				log_warn("build", "cmake", NULL, "%s: argument is NULL", cmd_type_str[cmd->type]);
				break;
			}

			dst.off += dprintf(dst,
					   "TARGET %.*s %s\n"
					   "\tCOMMAND %.*s\n",
					   target->len, target->data, custom_cmd_target_run_type_str[cmd->flag], arg_val->len, arg_val->data);
			break;
		}
		case CMAKE_CMD_ADD_DEPENDS: {
			str_t *name = arr_get(&cmake->strs, cmd->name);
			dst.off += dprintf(dst, "%.*s", name->len, name->data);
			const cmake_cmd_arg_t *arg;
			list_foreach(&cmake->args, cmd->args, arg)
			{
				str_t *str = arr_get(&cmake->strs, arg->val);
				if (str == NULL || str->data == NULL) {
					log_warn("build", "cmake", NULL, "%s: argument is NULL", cmd_type_str[cmd->type]);
					continue;
				}

				dst.off += dprintf(dst, " %.*s", str->len, str->data);
			}
			break;
		}
		case CMAKE_CMD_TARGET_DEFINES: {
			str_t *name = arr_get(&cmake->strs, cmd->name);
			dst.off += dprintf(dst, "%.*s %s", name->len, name->data, scope_str[cmd->flag]);
			const cmake_cmd_arg_t *arg;
			list_foreach(&cmake->args, cmd->args, arg)
			{
				str_t *str = arr_get(&cmake->strs, arg->val);
				if (str == NULL || str->data == NULL) {
					log_warn("build", "cmake", NULL, "%s: argument is NULL", cmd_type_str[cmd->type]);
					continue;
				}

				dst.off += dprintf(dst, " %.*s", str->len, str->data);
			}
			break;
		}
		case CMAKE_CMD_TARGET_COMPILE_OPTS: {
			str_t *name = arr_get(&cmake->strs, cmd->name);
			dst.off += dprintf(dst, "%.*s %s\n", name->len, name->data, scope_str[cmd->flag]);
			const cmake_cmd_arg_t *arg;
			list_foreach(&cmake->args, cmd->args, arg)
			{
				str_t *str = arr_get(&cmake->strs, arg->val);
				if (str == NULL || str->data == NULL) {
					log_warn("build", "cmake", NULL, "%s: argument is NULL", cmd_type_str[cmd->type]);
					continue;
				}

				dst.off += dprintf(dst, "\t%.*s\n", str->len, str->data);
			}
			break;
		}
		case CMAKE_CMD_TARGET_INCLUDE_DIRS:
		case CMAKE_CMD_TARGET_LINK_DIRS:
		case CMAKE_CMD_TARGET_LINK_LIBS:
		case CMAKE_CMD_TARGET_LINK_OPTS: {
			str_t *name = arr_get(&cmake->strs, cmd->name);
			dst.off += dprintf(dst, "%.*s %s", name->len, name->data, scope_str[cmd->flag]);
			const cmake_cmd_arg_t *arg;
			list_foreach(&cmake->args, cmd->args, arg)
			{
				str_t *str = arr_get(&cmake->strs, arg->val);
				if (str == NULL || str->data == NULL) {
					log_warn("build", "cmake", NULL, "%s: argument is NULL", cmd_type_str[cmd->type]);
					continue;
				}

				dst.off += dprintf(dst, " %.*s", str->len, str->data);
			}
			break;
		}
		case CMAKE_CMD_SET_TARGET_PROPS: {
			str_t *name = arr_get(&cmake->strs, cmd->name);
			dst.off += dprintf(dst, "%.*s PROPERTIES\n", name->len, name->data);
			const cmake_cmd_arg_t *arg;
			list_foreach(&cmake->args, cmd->args, arg)
			{
				str_t *val = arr_get(&cmake->strs, arg->val);
				if (val == NULL || val->data == NULL) {
					log_warn("build", "cmake", NULL, "%s: argument is NULL", cmd_type_str[cmd->type]);
					continue;
				}

				dst.off += dprintf(dst, "\t%s %.*s\n", target_prop_str[arg->flag], val->len, val->data);
			}
			break;
		}
		}
		dst.off += dprintf(dst, ")\n");
		if (cmd->type == CMAKE_CMD_FILE) {
			dst.off += dprintf(dst, "\n");
		}
	}

	return dst.off - off;
}
