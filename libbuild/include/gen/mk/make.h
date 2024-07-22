#ifndef MAKE_H
#define MAKE_H

#include "arr.h"
#include "list.h"
#include "print.h"
#include "str.h"

#define MAKE_END LIST_END

typedef lnode_t make_str_t;

typedef lnode_t make_act_t;
typedef make_act_t make_empty_t;
typedef make_act_t make_var_t;
typedef make_act_t make_rule_t;
typedef make_act_t make_rule_target_t;
typedef make_act_t make_cmd_t;
typedef make_act_t make_if_t;

typedef enum make_var_type_e {
	MAKE_VAR_INST, //:=
	MAKE_VAR_REF, //=
	MAKE_VAR_COND, //?=
	MAKE_VAR_APP, //+=
} make_var_type_t;

typedef enum make_str_type_e {
	MAKE_STR_STR,
	MAKE_STR_VAR,
} make_str_type_t;

typedef struct make_str_data_s {
	make_str_type_t type;
	union {
		str_t str;
		make_var_t var;
	} val;
} make_str_data_t;

typedef struct make_rule_target_data_s {
	make_str_data_t target;
	str_t action;
} make_rule_target_data_t;

typedef enum make_cmd_type_e {
	MAKE_CMD_NORMAL,
	MAKE_CMD_CHILD,
	MAKE_CMD_ERR,
} make_cmd_type_t;

typedef struct make_cmd_data_s {
	make_cmd_type_t type;
	str_t arg1;
	str_t arg2;
} make_cmd_data_t;

typedef struct make_s {
	list_t strs;
	list_t acts;
	list_t targets;
	make_act_t root;
} make_t;

make_t *make_init(make_t *make, uint strs_cap, uint acts_cap, uint targets_cap);
void make_free(make_t *make);

make_empty_t make_create_empty(make_t *make);
make_var_t make_create_var(make_t *make, str_t name, make_var_type_t type);
make_var_t make_create_var_ext(make_t *make, str_t name, make_var_type_t type);
make_rule_t make_create_rule(make_t *make, make_rule_target_data_t target, int file);
make_rule_t make_create_phony(make_t *make);
make_cmd_t make_create_cmd(make_t *make, make_cmd_data_t cmd);
make_if_t make_create_if(make_t *make, make_str_data_t l, make_str_data_t r);

make_act_t make_add_act(make_t *make, make_act_t act);
make_var_t make_var_add_val(make_t *make, make_var_t var, make_str_data_t val);
make_str_t make_rule_add_depend(make_t *make, make_rule_t rule, make_rule_target_data_t depend);
make_act_t make_rule_add_act(make_t *make, make_rule_t rule, make_act_t act);
make_act_t make_if_add_true_act(make_t *make, make_if_t mif, make_act_t act);
make_act_t make_if_add_false_act(make_t *make, make_if_t mif, make_act_t act);

make_str_t make_ext_set_val(make_t *make, str_t name, make_str_data_t val);

make_rule_t make_rule_get_target(const make_t *make, make_rule_target_data_t target);

int make_expand(make_t *make);
str_t make_var_get_expanded(const make_t *make, str_t var);
str_t make_var_get_resolved(const make_t *make, str_t var);

int make_print(const make_t *make, print_dst_t dst);

int make_dbg(const make_t *make, print_dst_t dst);

#define MSTR(_str)                                    \
	(make_str_data_t)                             \
	{                                             \
		.type = MAKE_STR_STR, .val.str = _str \
	}

#define MVAR(_var)                                     \
	(make_str_data_t)                              \
	{                                              \
		.type = MAKE_STR_VAR, .val.var = _var, \
	}

#define MRULE(_target)                              \
	(make_rule_target_data_t)                   \
	{                                           \
		.target = _target, .action = { 0 }, \
	}

#define MRULEACT(_target, _action)                    \
	(make_rule_target_data_t)                     \
	{                                             \
		.target = _target, .action = _action, \
	}

#define MCMD(_cmd)                                     \
	(make_cmd_data_t)                              \
	{                                              \
		.type = MAKE_CMD_NORMAL, .arg1 = _cmd, \
	}

#define MCMDCHILD(_dir, _target)                                       \
	(make_cmd_data_t)                                              \
	{                                                              \
		.type = MAKE_CMD_CHILD, .arg1 = _dir, .arg2 = _target, \
	}

#define MCMDERR(_msg)                               \
	(make_cmd_data_t)                           \
	{                                           \
		.type = MAKE_CMD_ERR, .arg1 = _msg, \
	}

#endif
