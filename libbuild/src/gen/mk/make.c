#include "gen/mk/make.h"

#include "log.h"
#include "platform.h"

#define MAX_VAR_VALUE_LEN 256

typedef struct make_var_data_s {
	str_t name;
	make_var_type_t type;
	lnode_t values;
	str_t ref;
	str_t expand;
	str_t resolve;
	int ext : 1;
	int expanded : 1;
	int init : 1;
} make_var_data_t;

typedef struct make_rule_data_s {
	make_rule_target_data_t target;
	lnode_t depends;
	make_act_t acts;
	int file : 1;
} make_rule_data_t;

typedef struct make_if_data_s {
	make_str_data_t l;
	make_str_data_t r;
	make_act_t true_acts;
	make_act_t false_acts;
	str_t l_value;
	str_t r_value;
} make_if_data_t;

typedef enum make_act_type_e {
	MAKE_ACT_EMPTY,
	MAKE_ACT_VAR,
	MAKE_ACT_RULE,
	MAKE_ACT_CMD,
	MAKE_ACT_IF,
} make_act_type_t;

typedef struct make_act_data_s {
	make_act_type_t type;
	union {
		make_var_data_t var;
		make_rule_data_t rule;
		make_cmd_data_t cmd;
		make_if_data_t mif;
	} val;
} make_act_data_t;

static inline make_act_data_t *make_act_get(const make_t *make, make_act_t act)
{
	if (make == NULL) {
		return NULL;
	}

	return list_get_data(&make->acts, act);
}

static inline make_var_data_t *make_var_get(const make_t *make, make_var_t var)
{
	make_act_data_t *act = make_act_get(make, var);
	if (act == NULL || act->type != MAKE_ACT_VAR) {
		return NULL;
	}

	return &act->val.var;
}

static inline make_rule_data_t *make_rule_get(const make_t *make, make_rule_t rule)
{
	make_act_data_t *act = make_act_get(make, rule);
	if (act == NULL || act->type != MAKE_ACT_RULE) {
		return NULL;
	}

	return &act->val.rule;
}

static inline make_if_data_t *make_if_get(const make_t *make, make_if_t mif)
{
	make_act_data_t *act = make_act_get(make, mif);
	if (act == NULL || act->type != MAKE_ACT_IF) {
		return NULL;
	}

	return &act->val.mif;
}

static make_var_t make_var_get_name(const make_t *make, str_t name)
{
	const make_act_data_t *act;
	list_foreach_all(&make->acts, act)
	{
		if (act->type == MAKE_ACT_VAR && str_eq(act->val.var.name, name)) {
			return _i;
		}
	}

	return MAKE_END;
}

make_t *make_init(make_t *make, uint strs_cap, uint acts_cap, uint targets_cap)
{
	if (make == NULL) {
		return NULL;
	}

	if (list_init(&make->strs, strs_cap, sizeof(make_str_data_t)) == NULL) {
		return NULL;
	}

	if (list_init(&make->acts, acts_cap, sizeof(make_act_data_t)) == NULL) {
		return NULL;
	}

	if (list_init(&make->targets, targets_cap, sizeof(make_rule_target_data_t)) == NULL) {
		return NULL;
	}

	make->root = MAKE_END;

	return make;
}

static void make_str_free(make_str_data_t *str)
{
	if (str->type != MAKE_STR_STR) {
		return;
	}

	str_free(&str->val.str);
}

static inline void make_var_free(make_var_data_t *var)
{
	str_free(&var->name);
	str_free(&var->ref);
	str_free(&var->expand);
	str_free(&var->resolve);
}

static inline void make_rule_free(make_rule_data_t *rule)
{
	make_str_free(&rule->target.target);
	str_free(&rule->target.action);
}

static inline void make_cmd_free(make_cmd_data_t *cmd)
{
	str_free(&cmd->arg1);
	str_free(&cmd->arg2);
}

static inline void make_if_free(make_if_data_t *mif)
{
	make_str_free(&mif->l);
	str_free(&mif->l_value);

	make_str_free(&mif->r);
	str_free(&mif->r_value);
}

void make_free(make_t *make)
{
	if (make == NULL) {
		return;
	}

	make_str_data_t *str;
	list_foreach_all(&make->strs, str)
	{
		make_str_free(str);
	}
	list_free(&make->strs);

	make_act_data_t *act;
	list_foreach_all(&make->acts, act)
	{
		switch (act->type) {
		case MAKE_ACT_VAR: make_var_free(&act->val.var); break;
		case MAKE_ACT_RULE: make_rule_free(&act->val.rule); break;
		case MAKE_ACT_CMD: make_cmd_free(&act->val.cmd); break;
		case MAKE_ACT_IF: make_if_free(&act->val.mif); break;
		default: break;
		}
	}
	list_free(&make->acts);

	make_rule_target_data_t *target;
	list_foreach_all(&make->targets, target)
	{
		make_str_free(&target->target);
		str_free(&target->action);
	}
	list_free(&make->targets);

	make->root = MAKE_END;
}

make_empty_t make_create_empty(make_t *make)
{
	if (make == NULL) {
		return MAKE_END;
	}

	const make_act_t act  = list_add(&make->acts);
	make_act_data_t *data = make_act_get(make, act);

	if (data == NULL) {
		return MAKE_END;
	}

	*data = (make_act_data_t){
		.type = MAKE_ACT_EMPTY,
	};

	return act;
}

static make_var_t make_create_var_r(make_t *make, str_t name, make_var_type_t type, int ext)
{
	if (make == NULL) {
		return MAKE_END;
	}

	const make_act_t act  = list_add(&make->acts);
	make_act_data_t *data = make_act_get(make, act);

	if (data == NULL) {
		return MAKE_END;
	}

	*data = (make_act_data_t) {
		.type = MAKE_ACT_VAR,
		.val.var  = {
			.name	 = name,
			.type	 = type,
			.values	 = MAKE_END,
			.ref	 = strf("$(%.*s)", name.len, name.data),
			.expand	 = strz(MAX_VAR_VALUE_LEN),
			.resolve = strz(MAX_VAR_VALUE_LEN),
			.ext	 = ext,
			.expanded = 0,
		},
	};

	return act;
}

make_var_t make_create_var(make_t *make, str_t name, make_var_type_t type)
{
	return make_create_var_r(make, name, type, 0);
}

make_var_t make_create_var_ext(make_t *make, str_t name, make_var_type_t type)
{
	return make_create_var_r(make, name, type, 1);
}

make_rule_t make_create_rule(make_t *make, make_rule_target_data_t target, int file)
{
	if (make == NULL) {
		return MAKE_END;
	}

	const make_act_t act  = list_add(&make->acts);
	make_act_data_t *data = make_act_get(make, act);

	if (data == NULL) {
		return MAKE_END;
	}

	*data = (make_act_data_t){
		.type = MAKE_ACT_RULE,
		.val.rule = {
			.target  = target,
			.depends = MAKE_END,
			.acts    = MAKE_END,
			.file    = file,
		},
	};

	return act;
}

make_rule_t make_create_phony(make_t *make)
{
	return make_create_rule(make, MRULE(MSTR(STR(".PHONY"))), 1);
}

make_cmd_t make_create_cmd(make_t *make, make_cmd_data_t cmd)
{
	if (make == NULL) {
		return MAKE_END;
	}

	const make_act_t act  = list_add(&make->acts);
	make_act_data_t *data = make_act_get(make, act);

	if (data == NULL) {
		return MAKE_END;
	}

	*data = (make_act_data_t){
		.type	 = MAKE_ACT_CMD,
		.val.cmd = cmd,
	};

	return act;
}

make_if_t make_create_if(make_t *make, make_str_data_t l, make_str_data_t r)
{
	if (make == NULL) {
		return MAKE_END;
	}

	const make_act_t act  = list_add(&make->acts);
	make_act_data_t *data = make_act_get(make, act);

	if (data == NULL) {
		return MAKE_END;
	}

	*data = (make_act_data_t){
		.type = MAKE_ACT_IF,
		.val.mif = {
			.l = l,
			.r = r,
			.true_acts = MAKE_END,
			.false_acts = MAKE_END,
			.l_value = strz(MAX_VAR_VALUE_LEN),
			.r_value = strz(MAX_VAR_VALUE_LEN),
		},
	};

	return act;
}

make_act_t make_add_act(make_t *make, make_act_t act)
{
	make_act_data_t *data = make_act_get(make, act);
	if (data == NULL) {
		return MAKE_END;
	}

	if (data->type == MAKE_ACT_RULE && !data->val.rule.file) {
		make_rule_t phony = make_rule_get_target(make, MRULE(MSTR(STR(".PHONY"))));
		if (phony == MAKE_END) {
			phony = make_add_act(make, make_create_phony(make));
		}
		make_rule_target_data_t depend = data->val.rule.target;
		if (depend.target.type == MAKE_STR_STR) {
			depend.target.val.str.ref = 1;
			depend.action.ref	  = 1;
		}
		make_rule_add_depend(make, phony, depend);
	}

	return list_set_next_node(&make->acts, make->root, act);
}

make_var_t make_var_add_val(make_t *make, make_var_t var, make_str_data_t val)
{
	if (make == NULL) {
		return MAKE_END;
	}

	make_var_data_t *data = make_var_get(make, var);

	if (data == NULL || data->ext) {
		return MAKE_END;
	}

	make_str_t str;
	list_add_next_node(&make->strs, data->values, str);
	make_str_data_t *target = list_get_data(&make->strs, str);

	if (target == NULL) {
		return MAKE_END;
	}

	*target = val;

	return var;
}

make_str_t make_rule_add_depend(make_t *make, make_rule_t rule, make_rule_target_data_t depend)
{
	if (make == NULL) {
		return MAKE_END;
	}

	make_rule_data_t *data = make_rule_get(make, rule);

	if (data == NULL) {
		return MAKE_END;
	}

	make_str_t str;
	list_add_next_node(&make->targets, data->depends, str);
	make_rule_target_data_t *target = list_get_data(&make->targets, str);

	if (target == NULL) {
		return MAKE_END;
	}

	*target = depend;

	return str;
}

make_act_t make_rule_add_act(make_t *make, make_rule_t rule, make_act_t act)
{
	if (make == NULL) {
		return MAKE_END;
	}

	make_rule_data_t *data = make_rule_get(make, rule);

	if (data == NULL || make_act_get(make, act) == NULL) {
		return MAKE_END;
	}

	return list_set_next_node(&make->acts, data->acts, act);
}

static make_act_t make_if_add_act(make_t *make, make_if_t mif, int true_acts, make_act_t act)
{
	if (make == NULL) {
		return MAKE_END;
	}

	make_if_data_t *data = make_if_get(make, mif);

	if (data == NULL || make_act_get(make, act) == NULL) {
		return MAKE_END;
	}

	make_act_t *acts = true_acts ? &data->true_acts : &data->false_acts;
	return list_set_next_node(&make->acts, *acts, act);
}

make_act_t make_if_add_true_act(make_t *make, make_if_t mif, make_act_t act)
{
	return make_if_add_act(make, mif, 1, act);
}

make_act_t make_if_add_false_act(make_t *make, make_if_t mif, make_act_t act)
{
	return make_if_add_act(make, mif, 0, act);
}

make_str_t make_ext_set_val(make_t *make, str_t name, make_str_data_t val)
{
	if (make == NULL) {
		return MAKE_END;
	}

	const make_act_t act  = make_var_get_name(make, name);
	make_act_data_t *data = make_act_get(make, act);

	if (data == NULL || !data->val.var.ext) {
		return MAKE_END;
	}

	make_str_t str;
	list_add_node(&make->strs, make_var_get(make, act)->values, str);
	make_str_data_t *target = list_get_data(&make->strs, str);

	if (target == NULL) {
		return MAKE_END;
	}

	*target = val;

	return str;
}

static int replace_refs(make_t *make, str_t *res)
{
	int found = 0;
	const make_act_data_t *act;
	list_foreach_all(&make->acts, act)
	{
		if (act->type != MAKE_ACT_VAR || !act->val.var.init || act->val.var.ext) {
			continue;
		}

		found |= str_replace(res, act->val.var.ref, act->val.var.expand);
	}
	return found;
}

static int replace_exts(make_t *make, str_t *res)
{
	int found = 0;
	const make_act_data_t *act;
	list_foreach_all(&make->acts, act)
	{
		if (act->type != MAKE_ACT_VAR || !act->val.var.init || !act->val.var.ext) {
			continue;
		}

		found |= str_replace(res, act->val.var.ref, act->val.var.resolve);
	}
	return found;
}

static str_t make_str(const make_t *make, make_str_data_t str)
{
	switch (str.type) {
	case MAKE_STR_VAR: {
		make_var_data_t *data = make_var_get(make, str.val.var);
		if (data == NULL) {
			log_warn("cutils", "make", NULL, "variable not found");
			return str_null();
		}
		return data->ref;
	}
	default: return str.val.str;
	}
}

static int make_var_expand(make_t *make, make_var_data_t *var)
{
	str_zero(&var->expand);
	str_zero(&var->resolve);
	const make_str_data_t *value;
	list_foreach(&make->strs, var->values, value)
	{
		str_cat(&var->expand, make_str(make, *value));

		if (list_get_next(&make->strs, _i) != LIST_END) {
			str_cat(&var->expand, STR(" "));
		}
	}

	make_var_data_t *init = make_var_get(make, make_var_get_name(make, var->name));

	init->init = -1;

	replace_refs(make, &var->expand);
	str_cpyd(var->expand, &var->resolve);
	replace_exts(make, &var->resolve);

	if (var != init) {
		str_cat(&init->expand, STR(" "));
		str_cat(&init->expand, var->expand);
		str_cat(&init->resolve, STR(" "));
		str_cat(&init->resolve, var->resolve);
	}

	var->expanded = -1;

	return 0;
}

static int make_acts_expand(make_t *make, make_act_t acts);

static inline int make_if_expand(make_t *make, make_if_data_t *mif)
{
	int ret = 0;

	str_zero(&mif->l_value);
	str_cat(&mif->l_value, make_str(make, mif->l));

	str_zero(&mif->r_value);
	str_cat(&mif->r_value, make_str(make, mif->r));

	replace_refs(make, &mif->l_value);
	replace_exts(make, &mif->l_value);
	replace_refs(make, &mif->r_value);
	replace_exts(make, &mif->r_value);

	if (str_eq(mif->l_value, mif->r_value)) {
		ret |= make_acts_expand(make, mif->true_acts);
	} else {
		ret |= make_acts_expand(make, mif->false_acts);
	}

	return ret;
}

static inline int make_expand_reset(make_t *make)
{
	make_act_data_t *act;
	list_foreach_all(&make->acts, act)
	{
		switch (act->type) {
		case MAKE_ACT_VAR:
			act->val.var.expanded = 0;
			act->val.var.init     = 0;
			break;
		default: break;
		}
	}

	return 0;
}

static inline int make_var_ext_expand(make_t *make)
{
	int ret = 0;
	make_act_data_t *act;
	list_foreach_all(&make->acts, act)
	{
		if (act->type == MAKE_ACT_VAR && act->val.var.ext) {
			ret |= make_var_expand(make, &act->val.var);
		}
	}

	return ret;
}

static int make_acts_expand(make_t *make, make_act_t acts)
{
	int ret = 0;
	make_act_data_t *act;
	list_foreach(&make->acts, acts, act)
	{
		switch (act->type) {
		case MAKE_ACT_VAR: ret |= make_var_expand(make, &act->val.var); break;
		case MAKE_ACT_IF: ret |= make_if_expand(make, &act->val.mif); break;
		default: break;
		}
	}
	return ret;
}

static int make_rule_target_eq(const make_t *make, make_rule_target_data_t target1, make_rule_target_data_t target2)
{
	if (target1.target.type != target2.target.type) {
		return 0;
	}

	if (!str_eq(target1.action, target2.action)) {
		return 0;
	}

	switch (target1.target.type) {
	case MAKE_STR_VAR: return str_eq(make_var_get(make, target1.target.val.var)->name, make_var_get(make, target2.target.val.var)->name);
	default: return str_eq(target1.target.val.str, target2.target.val.str);
	}
}

make_rule_t make_rule_get_target(const make_t *make, make_rule_target_data_t target)
{
	if (make == NULL) {
		return MAKE_END;
	}

	const make_act_data_t *act;
	make_act_t rule = MAKE_END;

	list_foreach_all(&make->acts, act)
	{
		if (act->type == MAKE_ACT_RULE && make_rule_target_eq(make, act->val.rule.target, target)) {
			return _i;
		}
	}

	return rule;
}

int make_expand(make_t *make)
{
	if (make == NULL) {
		return 1;
	}

	int ret = 0;

	ret |= make_expand_reset(make);
	ret |= make_var_ext_expand(make);
	ret |= make_acts_expand(make, make->root);

	return ret;
}

str_t make_var_get_expanded(const make_t *make, str_t var)
{
	if (make == NULL) {
		return str_null();
	}

	make_act_data_t *act;
	list_foreach_all(&make->acts, act)
	{
		if (act->type == MAKE_ACT_VAR && act->val.var.expanded && str_eq(act->val.var.name, var)) {
			return act->val.var.expand;
		}
	}

	return str_null();
}

str_t make_var_get_resolved(const make_t *make, str_t var)
{
	if (make == NULL) {
		return str_null();
	}

	make_act_data_t *act;
	list_foreach_all(&make->acts, act)
	{
		if (act->type == MAKE_ACT_VAR && act->val.var.expanded && str_eq(act->val.var.name, var)) {
			return act->val.var.resolve;
		}
	}

	return str_null();
}

const char *make_var_type_to_str(make_var_type_t type)
{
	switch (type) {
	case MAKE_VAR_INST: return ":=";
	//case MAKE_VAR_REF: return "=";
	//case MAKE_VAR_COND: return "?=";
	case MAKE_VAR_APP: return "+=";
	default: return NULL;
	}
}

static int make_str_print(const make_t *make, make_str_data_t str, print_dst_t dst)
{
	str_t val = make_str(make, str);
	if (val.data == NULL) {
		return 0;
	}

	return dprintf(dst, "%.*s", val.len, val.data);
}

static int make_rule_target_print(const make_t *make, const make_rule_target_data_t *target, print_dst_t dst)
{
	int off = dst.off;

	dst.off += make_str_print(make, target->target, dst);
	if (target->action.data != NULL) {
		dst.off += dprintf(dst, "%.*s", target->action.len, target->action.data);
	}

	return dst.off - off;
}

static inline int make_var_print(const make_t *make, const make_var_data_t *var, print_dst_t dst)
{
	int off = dst.off;

	if (var->ext) {
		return 0;
	}

	const char *var_type_str = make_var_type_to_str(var->type);
	if (var_type_str == NULL) {
		return 0;
	}

	dst.off += dprintf(dst, "%.*s %s", var->name.len, var->name.data, var_type_str);

	const make_str_data_t *value;
	list_foreach(&make->strs, var->values, value)
	{
		dst.off += dprintf(dst, " ");
		dst.off += make_str_print(make, *value, dst);
	}

	dst.off += dprintf(dst, "\n");

	return dst.off - off;
}

static int make_acts_print(const make_t *make, const make_act_t acts, print_dst_t dst, int rule);

static inline int make_rule_print(const make_t *make, const make_rule_data_t *rule, print_dst_t dst)
{
	int off = dst.off;
	dst.off += make_rule_target_print(make, &rule->target, dst);
	dst.off += dprintf(dst, ":");

	const make_rule_target_data_t *depend;
	list_foreach(&make->targets, rule->depends, depend)
	{
		dst.off += dprintf(dst, " ");
		dst.off += make_rule_target_print(make, depend, dst);
	}

	dst.off += dprintf(dst, "\n");
	dst.off += make_acts_print(make, rule->acts, dst, 1);
	dst.off += dprintf(dst, "\n");

	return dst.off - off;
}

static inline int make_cmd_print(const make_t *make, const make_cmd_data_t *cmd, print_dst_t dst, int rule)
{
	(void)make;

	switch (cmd->type) {
	case MAKE_CMD_CHILD:
		if (cmd->arg2.data == NULL) {
			return dprintf(dst, "\t@$(MAKE) -C %.*s\n", cmd->arg1.len, cmd->arg1.data);
		} else {
			return dprintf(dst, "\t@$(MAKE) -C %.*s %.*s\n", cmd->arg1.len, cmd->arg1.data, cmd->arg2.len, cmd->arg2.data);
		}
	case MAKE_CMD_ERR: return dprintf(dst, "%s$(error %.*s)\n", rule ? "\t" : "", cmd->arg1.len, cmd->arg1.data);
	default: return dprintf(dst, "%s%.*s\n", rule ? "\t" : "", cmd->arg1.len, cmd->arg1.data);
	}
}

static inline int make_if_print(const make_t *make, const make_if_data_t *mif, print_dst_t dst, int rule)
{
	int off = dst.off;

	dst.off += dprintf(dst, "ifeq (");
	dst.off += make_str_print(make, mif->l, dst);

	str_t r = make_str(make, mif->r);
	if (r.data == NULL) {
		dst.off += dprintf(dst, ",");
	} else {
		dst.off += dprintf(dst, ", ");
		dst.off += make_str_print(make, mif->r, dst);
	}

	dst.off += dprintf(dst, ")\n");

	dst.off += make_acts_print(make, mif->true_acts, dst, rule);

	if (mif->false_acts != MAKE_END) {
		dst.off += dprintf(dst, "else\n");
		dst.off += make_acts_print(make, mif->false_acts, dst, rule);
	}

	dst.off += dprintf(dst, "endif\n");

	return dst.off - off;
}

static int make_acts_print(const make_t *make, make_act_t acts, print_dst_t dst, int rule)
{
	int off = dst.off;
	const make_act_data_t *act;
	list_foreach(&make->acts, acts, act)
	{
		switch (act->type) {
		case MAKE_ACT_EMPTY: dst.off += dprintf(dst, "\n"); break;
		case MAKE_ACT_VAR: dst.off += make_var_print(make, &act->val.var, dst); break;
		case MAKE_ACT_RULE: dst.off += make_rule_print(make, &act->val.rule, dst); break;
		case MAKE_ACT_CMD: dst.off += make_cmd_print(make, &act->val.cmd, dst, rule); break;
		case MAKE_ACT_IF: dst.off += make_if_print(make, &act->val.mif, dst, rule); break;
		}
	}

	return dst.off - off;
}

int make_print(const make_t *make, print_dst_t dst)
{
	if (make == NULL) {
		return 0;
	}

	return make_acts_print(make, make->root, dst, 0);
}

static inline int make_empty_dbg(const make_t *make, print_dst_t dst)
{
	(void)make;
	return dprintf(dst, "EMPTY\n\n");
}

static inline int make_var_dbg(const make_t *make, const make_var_data_t *var, print_dst_t dst)
{
	int off = dst.off;
	dst.off += dprintf(dst,
			   "VAR\n"
			   "    NAME    : %.*s %s\n"
			   "    VALUES  :\n",
			   var->name.len, var->name.data, var->ext ? "(ext)" : "");
	const make_str_data_t *value;
	list_foreach(&make->strs, var->values, value)
	{
		dst.off += dprintf(dst, "        ");
		dst.off += make_str_print(make, *value, dst);
		dst.off += dprintf(dst, "\n");
	}
	dst.off += dprintf(dst,
			   "    REF     : %.*s\n"
			   "    EXPANDED: %.*s\n"
			   "    RESOLVED: %.*s\n"
			   "\n",
			   var->ref.len, var->ref.data, var->expand.len, var->expand.data, var->resolve.len, var->resolve.data);

	return dst.off - off;
}

static inline int make_rule_dbg(const make_t *make, const make_rule_data_t *rule, print_dst_t dst)
{
	int off = dst.off;
	dst.off += dprintf(dst, "RULE\n"
				"    TARGET: ");
	dst.off += make_rule_target_print(make, &rule->target, dst);

	dst.off += dprintf(dst, "\n    DEPENDS:\n");
	const make_rule_target_data_t *depend;
	list_foreach(&make->targets, rule->depends, depend)
	{
		dst.off += dprintf(dst, "        ");
		dst.off += make_rule_target_print(make, depend, dst);
		dst.off += dprintf(dst, "\n");
	}

	return dst.off - off;
}

static inline int make_cmd_dbg(const make_t *make, const make_cmd_data_t *cmd, print_dst_t dst)
{
	(void)make;
	int off = dst.off;
	dst.off += dprintf(dst,
			   "CMD\n"
			   "    ARG1: %.*s\n"
			   "    ARG2: %.*s\n"
			   "    TYPE: %d\n",
			   cmd->arg1.len, cmd->arg1.data, cmd->arg2.len, cmd->arg2.data, cmd->type);

	return dst.off - off;
}

static inline int make_if_dbg(const make_t *make, const make_if_data_t *mif, print_dst_t dst)
{
	int off = dst.off;
	dst.off += dprintf(dst, "IF\n"
				"    L: ");
	dst.off += make_str_print(make, mif->l, dst);
	dst.off += dprintf(dst,
			   "\n"
			   "        VALUE   : %.*s\n"
			   "    R: ",
			   mif->l_value.len, mif->l_value.data);

	dst.off += make_str_print(make, mif->r, dst);
	dst.off += dprintf(dst,
			   "\n"
			   "        VALUE   : %.*s\n"
			   "\n",
			   mif->r_value.len, mif->r_value.data);

	return dst.off - off;
}

int make_dbg(const make_t *make, print_dst_t dst)
{
	if (make == NULL) {
		return 0;
	}

	int off = dst.off;
	const make_act_data_t *act;
	list_foreach_all(&make->acts, act)
	{
		switch (act->type) {
		case MAKE_ACT_EMPTY: dst.off += make_empty_dbg(make, dst); break;
		case MAKE_ACT_VAR: dst.off += make_var_dbg(make, &act->val.var, dst); break;
		case MAKE_ACT_RULE: dst.off += make_rule_dbg(make, &act->val.rule, dst); break;
		case MAKE_ACT_CMD: dst.off += make_cmd_dbg(make, &act->val.cmd, dst); break;
		case MAKE_ACT_IF: dst.off += make_if_dbg(make, &act->val.mif, dst); break;
		}
	}

	return dst.off - off;
}
