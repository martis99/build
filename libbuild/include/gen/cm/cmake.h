#ifndef CMAKE_H
#define CMAKE_H

#include "arr.h"
#include "list.h"
#include "print.h"
#include "str.h"

#define CMAKE_END ARR_END

typedef enum cmake_cmd_type_e {
	CMAKE_CMD_FILE,
	CMAKE_CMD_ADD_EXE,
	CMAKE_CMD_ADD_LIB,
	CMAKE_CMD_ADD_CUSTOM_TARGET,
	CMAKE_CMD_ADD_CUSTOM_CMD_TARGET,
	CMAKE_CMD_ADD_DEPENDS,
	CMAKE_CMD_TARGET_DEFINES,
	CMAKE_CMD_TARGET_COMPILE_OPTS,
	CMAKE_CMD_TARGET_INCLUDE_DIRS,
	CMAKE_CMD_TARGET_LINK_DIRS,
	CMAKE_CMD_TARGET_LINK_LIBS,
	CMAKE_CMD_TARGET_LINK_OPTS,
	CMAKE_CMD_SET_TARGET_PROPS,
} cmake_cmd_type_t;

typedef enum cmake_file_act_e {
	CMAKE_FILE_GLOB_RECURSE,
} cmake_file_act_t;

typedef enum cmake_lib_type_e {
	CMAKE_LIB_STATIC,
	CMAKE_LIB_SHARED,
} cmake_lib_type_t;

typedef enum cmake_cmd_target_run_type_e {
	CMAKE_CMD_TARGET_PRE_BUILD,
	CMAKE_CMD_TARGET_PRE_LINK,
	CMAKE_CMD_TARGET_POST_BUILD,
} cmake_cmd_target_run_type_t;

typedef enum cmake_scope_e {
	CMAKE_SCOPE_PRIVATE,
	CMAKE_SCOPE_PUBLIC,
	CMAKE_SCOPE_INTERFACE,
} cmake_scope_t;

typedef enum cmake_tartget_prop_e {
	CMAKE_TARGET_PROP_ARC_OUT_DIR,
	CMAKE_TARGET_PROP_LIB_OUT_DIR,
	CMAKE_TARGET_PROP_RUN_OUT_DIR,
	CMAKE_TARGET_PROP_ARC_OUT_DIR_DBG,
	CMAKE_TARGET_PROP_LIB_OUT_DIR_DBG,
	CMAKE_TARGET_PROP_RUN_OUT_DIR_DBG,
	CMAKE_TARGET_PROP_ARC_OUT_DIR_RLS,
	CMAKE_TARGET_PROP_LIB_OUT_DIR_RLS,
	CMAKE_TARGET_PROP_RUN_OUT_DIR_RLS,
	CMAKE_TARGET_PROP_BUILD_RPATH,
	CMAKE_TARGET_PROP_OUTPUT_NAME,
	CMAKE_TARGET_PROP_PREFIX,
} cmake_target_prop_t;

typedef struct cmake_s {
	arr_t strs;
	arr_t cmds;
	list_t args;
} cmake_t;

typedef struct cmake_cmd_arg_s {
	int flag;
	uint val;
} cmake_cmd_arg_t;

cmake_t *cmake_init(cmake_t *cmake, uint strs_cap, uint cmds_cap, uint args_cap);
void cmake_free(cmake_t *cmake);

uint cmake_cmd_add_str(cmake_t *cmake, uint cmd, str_t str);
uint cmake_cmd_add_arg(cmake_t *cmake, uint cmd, cmake_cmd_arg_t arg);

uint cmake_file(cmake_t *cmake, cmake_file_act_t act, str_t name);
uint cmake_add_exe(cmake_t *cmake, str_t name, uint sources);
uint cmake_add_lib(cmake_t *cmake, str_t name, cmake_lib_type_t type, uint sources);
uint cmake_add_custom_target(cmake_t *cmake, str_t name);
uint cmake_add_custom_cmd_target(cmake_t *cmake, uint target, cmake_cmd_target_run_type_t run_type, str_t cmd);
uint cmake_add_depends(cmake_t *cmake, uint target);
uint cmake_target_cmd(cmake_t *cmake, cmake_cmd_type_t cmd, uint target, cmake_scope_t scope);
uint cmake_set_target_props(cmake_t *cmake, uint target);
uint cmake_add_target_prop(cmake_t *cmake, uint props, cmake_target_prop_t prop, str_t val);

int cmake_print(const cmake_t *cmake, print_dst_t dst);

#define CMAKE_FILE(_act, _name, _files)       \
	(cmake_file_t)                        \
	{                                     \
		.act = _act, .files = _files, \
	}

#define CMAKE_TARGET(_type, _name, _sources)                       \
	(cmake_target_t)                                           \
	{                                                          \
		.type = _type, .name = _name, .sources = _sources, \
	}

#endif
