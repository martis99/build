#include "gen/mk/make.h"

#include "mem.h"
#include "test.h"

TEST(t_make_init_free)
{
	START;

	make_t make = { 0 };

	EXPECT_EQ(make_init(NULL, 0, 0, 0), NULL);
	mem_oom(1);
	EXPECT_EQ(make_init(&make, 1, 0, 0), NULL);
	EXPECT_EQ(make_init(&make, 0, 1, 0), NULL);
	EXPECT_EQ(make_init(&make, 0, 0, 1), NULL);
	mem_oom(0);
	EXPECT_EQ(make_init(&make, 1, 1, 1), &make);

	EXPECT_NE(make.acts.data, NULL);
	EXPECT_NE(make.strs.data, NULL);
	EXPECT_EQ(make.root, MAKE_END);

	make_free(&make);
	make_free(NULL);

	EXPECT_EQ(make.acts.data, NULL);
	EXPECT_EQ(make.strs.data, NULL);
	EXPECT_EQ(make.root, MAKE_END);

	END;
}

TEST(t_make_create_empty)
{
	START;

	make_t make = { 0 };
	make_init(&make, 0, 0, 0);

	EXPECT_EQ(make_create_empty(NULL), MAKE_END);
	mem_oom(1);
	EXPECT_EQ(make_create_empty(&make), MAKE_END);
	mem_oom(0);
	EXPECT_EQ(make_create_empty(&make), 0);

	make_free(&make);
	END;
}

TEST(t_make_create_var)
{
	START;

	make_t make = { 0 };
	make_init(&make, 0, 0, 0);

	EXPECT_EQ(make_create_var(NULL, STR(""), -1), MAKE_END);
	mem_oom(1);
	EXPECT_EQ(make_create_var(&make, STR(""), -1), MAKE_END);
	mem_oom(0);
	EXPECT_EQ(make_create_var(&make, STRH(""), -1), 0);
	EXPECT_EQ(make_create_var(&make, STRH(""), MAKE_VAR_INST), 1);
	EXPECT_EQ(make_create_var(&make, STRH(""), MAKE_VAR_APP), 2);

	make_free(&make);

	END;
}

TEST(t_make_create_var_ext)
{
	START;

	make_t make = { 0 };
	make_init(&make, 0, 0, 0);

	EXPECT_EQ(make_create_var_ext(NULL, STR(""), -1), MAKE_END);
	mem_oom(1);
	EXPECT_EQ(make_create_var(&make, STR(""), -1), MAKE_END);
	mem_oom(0);
	EXPECT_EQ(make_create_var_ext(&make, STRH(""), -1), 0);
	EXPECT_EQ(make_create_var_ext(&make, STRH(""), MAKE_VAR_INST), 1);
	EXPECT_EQ(make_create_var_ext(&make, STRH(""), MAKE_VAR_APP), 2);

	make_free(&make);

	END;
}

TEST(t_make_create_rule)
{
	START;

	make_t make = { 0 };
	make_init(&make, 0, 0, 0);

	EXPECT_EQ(make_create_rule(NULL, MRULE(MVAR(MAKE_END)), 0), MAKE_END);
	make_str_data_t str = (make_str_data_t){ .type = -1 };
	EXPECT_EQ(make_create_rule(&make, MRULE(str), 0), 0);
	EXPECT_EQ(make_create_rule(&make, MRULE(MVAR(MAKE_END)), 0), 1);
	mem_oom(1);
	EXPECT_EQ(make_create_rule(&make, MRULE(MSTR(STRH(""))), 0), MAKE_END);
	mem_oom(0);
	EXPECT_EQ(make_create_rule(&make, MRULE(MSTR(STRH(""))), 0), 2);

	make_free(&make);

	END;
}

TEST(t_make_create_phony)
{
	START;

	make_t make = { 0 };
	make_init(&make, 0, 0, 0);

	EXPECT_EQ(make_create_phony(NULL), MAKE_END);
	mem_oom(1);
	EXPECT_EQ(make_create_phony(&make), MAKE_END);
	mem_oom(0);
	EXPECT_EQ(make_create_phony(&make), 0);

	make_free(&make);

	END;
}

TEST(t_make_create_cmd)
{
	START;

	make_t make = { 0 };
	make_init(&make, 0, 0, 0);

	EXPECT_EQ(make_create_cmd(NULL, MCMD(STR(""))), MAKE_END);
	mem_oom(1);
	EXPECT_EQ(make_create_cmd(&make, MCMD(STR(""))), MAKE_END);
	mem_oom(0);
	EXPECT_EQ(make_create_cmd(&make, MCMD(STRH(""))), 0);

	make_free(&make);

	END;
}

TEST(t_make_create_if)
{
	START;

	make_t make = { 0 };
	make_init(&make, 0, 0, 0);

	EXPECT_EQ(make_create_if(NULL, MVAR(MAKE_END), MVAR(MAKE_END)), MAKE_END);
	EXPECT_EQ(make_create_if(&make, MVAR(MAKE_END), MVAR(MAKE_END)), 0);
	mem_oom(1);
	EXPECT_EQ(make_create_if(&make, MSTR(STR("")), MSTR(STR(""))), MAKE_END);
	mem_oom(0);
	EXPECT_EQ(make_create_if(&make, MSTR(STRH("")), MSTR(STRH(""))), 1);

	make_free(&make);

	END;
}

TEST(t_make_create)
{
	SSTART;
	RUN(t_make_create_empty);
	RUN(t_make_create_var);
	RUN(t_make_create_var_ext);
	RUN(t_make_create_rule);
	RUN(t_make_create_phony);
	RUN(t_make_create_cmd);
	RUN(t_make_create_if);
	SEND;
}

TEST(t_make_add_act)
{
	START;

	make_t make = { 0 };
	make_init(&make, 1, 1, 1);

	EXPECT_EQ(make_add_act(NULL, MAKE_END), MAKE_END);
	EXPECT_EQ(make_add_act(&make, MAKE_END), MAKE_END);
	EXPECT_EQ(make_add_act(&make, make_create_empty(&make)), 0);
	EXPECT_EQ(make_add_act(&make, make_create_rule(&make, MRULE(MSTR(STRH(""))), 0)), 1);

	make_free(&make);

	END;
}

TEST(t_make_var_add_val)
{
	START;

	make_t make = { 0 };
	make_init(&make, 0, 0, 0);

	const make_empty_t empty = make_create_empty(&make);
	const make_rule_t var	 = make_create_var(&make, STRH(""), MAKE_VAR_INST);

	EXPECT_EQ(make_var_add_val(NULL, MAKE_END, MVAR(MAKE_END)), MAKE_END);
	EXPECT_EQ(make_var_add_val(&make, MAKE_END, MVAR(MAKE_END)), MAKE_END);
	EXPECT_EQ(make_var_add_val(&make, empty, MVAR(MAKE_END)), MAKE_END);
	EXPECT_EQ(make_var_add_val(&make, make_create_var_ext(&make, STRH(""), MAKE_VAR_INST), MVAR(MAKE_END)), MAKE_END);
	EXPECT_EQ(make_var_add_val(&make, var, MVAR(MAKE_END)), 1);
	mem_oom(1);
	EXPECT_EQ(make_var_add_val(&make, var, MSTR(STRH(""))), MAKE_END);
	mem_oom(0);
	EXPECT_EQ(make_var_add_val(&make, var, MSTR(STRH(""))), 1);

	make_free(&make);

	END;
}

TEST(t_make_rule_add_depend)
{
	START;

	make_t make = { 0 };
	make_init(&make, 0, 0, 0);

	const make_empty_t empty = make_create_empty(&make);
	const make_rule_t rule	 = make_create_rule(&make, MRULE(MSTR(STRH(""))), 0);

	EXPECT_EQ(make_rule_add_depend(NULL, MAKE_END, MRULE(MVAR(MAKE_END))), MAKE_END);
	EXPECT_EQ(make_rule_add_depend(&make, MAKE_END, MRULE(MVAR(MAKE_END))), MAKE_END);
	EXPECT_EQ(make_rule_add_depend(&make, empty, MRULE(MVAR(MAKE_END))), MAKE_END);
	EXPECT_EQ(make_rule_add_depend(&make, rule, MRULE(MVAR(MAKE_END))), 0);
	mem_oom(1);
	EXPECT_EQ(make_rule_add_depend(&make, rule, MRULE(MSTR(STRH("")))), MAKE_END);
	mem_oom(0);
	EXPECT_EQ(make_rule_add_depend(&make, rule, MRULE(MSTR(STRH("")))), 1);

	make_free(&make);

	END;
}

TEST(t_make_rule_add_act)
{
	START;

	make_t make = { 0 };
	make_init(&make, 1, 1, 1);

	const make_empty_t empty = make_create_empty(&make);
	const make_rule_t rule	 = make_create_rule(&make, MRULE(MSTR(STRH(""))), 0);

	EXPECT_EQ(make_rule_add_act(NULL, MAKE_END, MAKE_END), MAKE_END);
	EXPECT_EQ(make_rule_add_act(&make, MAKE_END, MAKE_END), MAKE_END);
	EXPECT_EQ(make_rule_add_act(&make, empty, MAKE_END), MAKE_END);
	EXPECT_EQ(make_rule_add_act(&make, rule, MAKE_END), MAKE_END);
	EXPECT_EQ(make_rule_add_act(&make, rule, empty), 0);

	make_free(&make);

	END;
}

TEST(t_make_if_add_true_act)
{
	START;

	make_t make = { 0 };
	make_init(&make, 1, 1, 1);

	const make_empty_t empty = make_create_empty(&make);
	const make_if_t mif	 = make_create_if(&make, MSTR(STRH("")), MSTR(STRH("")));

	EXPECT_EQ(make_if_add_true_act(NULL, MAKE_END, MAKE_END), MAKE_END);
	EXPECT_EQ(make_if_add_true_act(&make, MAKE_END, MAKE_END), MAKE_END);
	EXPECT_EQ(make_if_add_true_act(&make, empty, MAKE_END), MAKE_END);
	EXPECT_EQ(make_if_add_true_act(&make, mif, MAKE_END), MAKE_END);
	EXPECT_EQ(make_if_add_true_act(&make, mif, empty), 0);

	make_free(&make);

	END;
}

TEST(t_make_if_add_false_act)
{
	START;

	make_t make = { 0 };
	make_init(&make, 1, 1, 1);

	const make_empty_t empty = make_create_empty(&make);
	const make_if_t mif	 = make_create_if(&make, MSTR(STRH("")), MSTR(STRH("")));

	EXPECT_EQ(make_if_add_false_act(NULL, MAKE_END, MAKE_END), MAKE_END);
	EXPECT_EQ(make_if_add_false_act(&make, MAKE_END, MAKE_END), MAKE_END);
	EXPECT_EQ(make_if_add_false_act(&make, empty, MAKE_END), MAKE_END);
	EXPECT_EQ(make_if_add_false_act(&make, mif, MAKE_END), MAKE_END);
	EXPECT_EQ(make_if_add_false_act(&make, mif, empty), 0);

	make_free(&make);

	END;
}

TEST(t_make_add)
{
	SSTART;
	RUN(t_make_add_act);
	RUN(t_make_var_add_val);
	RUN(t_make_rule_add_depend);
	RUN(t_make_rule_add_act);
	RUN(t_make_if_add_true_act);
	RUN(t_make_if_add_false_act);
	SEND;
}

TEST(t_make_rule_get_target)
{
	START;

	make_t make = { 0 };
	make_init(&make, 0, 0, 0);

	make_var_t var = make_create_var(&make, STRH("VAR"), MAKE_VAR_INST);

	make_add_act(&make, make_create_rule(&make, MRULE(MVAR(var)), 0));
	make_add_act(&make, make_create_rule(&make, MRULE(MSTR(STRH("a"))), 0));
	make_add_act(&make, make_create_rule(&make, MRULEACT(MSTR(STRH("a")), STRH("/action")), 0));

	EXPECT_EQ(make_rule_get_target(NULL, MRULE(MSTR(STR("")))), MAKE_END);
	EXPECT_EQ(make_rule_get_target(&make, MRULE(MSTR(STR("")))), MAKE_END);
	EXPECT_EQ(make_rule_get_target(&make, MRULE(MSTR(STR("a")))), 3);
	EXPECT_EQ(make_rule_get_target(&make, MRULE(MVAR(var))), 1);

	make_free(&make);

	END;
}

TEST(t_make_ext_set_val)
{
	START;

	make_t make = { 0 };
	make_init(&make, 0, 0, 0);

	make_create_var_ext(&make, STR("EXT"), MAKE_VAR_INST);

	EXPECT_EQ(make_ext_set_val(NULL, STR(""), MVAR(MAKE_END)), MAKE_END);
	EXPECT_EQ(make_ext_set_val(&make, STR(""), MVAR(MAKE_END)), MAKE_END);
	mem_oom(1);
	EXPECT_EQ(make_ext_set_val(&make, STR("EXT"), MVAR(MAKE_END)), MAKE_END);
	mem_oom(0);
	EXPECT_EQ(make_ext_set_val(&make, STR("EXT"), MVAR(MAKE_END)), 0);
	EXPECT_EQ(make_ext_set_val(&make, STR("EXT"), MSTR(STR(""))), 0);

	make_free(&make);

	END;
}

TEST(t_make_expand)
{
	START;

	make_t make = { 0 };
	make_init(&make, 1, 1, 1);

	EXPECT_EQ(make_expand(NULL), 1);
	EXPECT_EQ(make_expand(&make), 0);

	make_free(&make);

	END;
}

TEST(t_make_var_get_expanded)
{
	START;

	make_t make = { 0 };
	make_init(&make, 1, 1, 1);

	EXPECT_EQ(make_var_get_expanded(NULL, STR("")).data, NULL);
	EXPECT_EQ(make_var_get_expanded(&make, STR("")).data, NULL);

	make_free(&make);

	END;
}

TEST(t_make_var_get_resolved)
{
	START;

	make_t make = { 0 };
	make_init(&make, 1, 1, 1);

	EXPECT_EQ(make_var_get_resolved(NULL, STR("")).data, NULL);
	EXPECT_EQ(make_var_get_resolved(&make, STR("")).data, NULL);

	make_free(&make);

	END;
}

TEST(t_make_print)
{
	START;

	make_t make = { 0 };
	make_init(&make, 1, 1, 1);

	char buf[8] = { 0 };
	EXPECT_EQ(make_print(NULL, PRINT_DST_BUF(buf, sizeof(buf), 0)), 0);
	EXPECT_EQ(make_print(&make, PRINT_DST_BUF(buf, sizeof(buf), 0)), 0);

	make_add_act(&make, make_create_empty(&make));

	EXPECT_EQ(make_print(&make, PRINT_DST_BUF(buf, sizeof(buf), 0)), 1);

	make_add_act(&make, make_create_var(&make, STRH(""), -1));
	EXPECT_EQ(make_print(&make, PRINT_DST_BUF(buf, sizeof(buf), 0)), 1);

	make_free(&make);

	END;
}

TEST(t_make_dbg)
{
	START;

	make_t make = { 0 };
	make_init(&make, 1, 1, 1);

	char buf[8] = { 0 };
	EXPECT_EQ(make_dbg(NULL, PRINT_DST_BUF(buf, sizeof(buf), 0)), 0);
	EXPECT_EQ(make_dbg(&make, PRINT_DST_BUF(buf, sizeof(buf), 0)), 0);

	make_add_act(&make, make_create_empty(&make));

	EXPECT_EQ(make_dbg(&make, PRINT_DST_BUF(buf, sizeof(buf), 0)), 7);

	make_free(&make);

	END;
}

TEST(t_make_expand_print_empty)
{
	START;

	make_t make = { 0 };
	make_init(&make, 1, 1, 1);

	make_add_act(&make, make_create_empty(&make));

	EXPECT_EQ(make_expand(&make), 0);

	char buf[8] = { 0 };
	EXPECT_EQ(make_print(&make, PRINT_DST_BUF(buf, sizeof(buf), 0)), 1);
	EXPECT_STR(buf, "\n");

	EXPECT_EQ(make_dbg(&make, PRINT_DST_BUF(buf, sizeof(buf), 0)), 7);

	make_free(&make);

	END;
}

TEST(t_make_expand_print_var_inst_empty)
{
	START;

	make_t make = { 0 };
	make_init(&make, 1, 1, 1);

	make_add_act(&make, make_create_var(&make, STRH("VAR"), MAKE_VAR_INST));

	EXPECT_EQ(make_expand(&make), 0);
	str_t var_exp = make_var_get_expanded(&make, STR("VAR"));
	EXPECT_STRN(var_exp.data, "", var_exp.len);

	char buf[128] = { 0 };
	EXPECT_EQ(make_print(&make, PRINT_DST_BUF(buf, sizeof(buf), 0)), 7);
	EXPECT_STR(buf, "VAR :=\n");

	EXPECT_EQ(make_dbg(&make, PRINT_DST_BUF(buf, sizeof(buf), 0)), 89);

	make_free(&make);

	END;
}

TEST(t_make_expand_print_var_inst)
{
	START;

	make_t make = { 0 };
	make_init(&make, 1, 1, 1);

	make_var_add_val(&make, make_add_act(&make, make_create_var(&make, STRH("VAR"), MAKE_VAR_INST)), MSTR(STRH("VAL")));

	EXPECT_EQ(make_expand(&make), 0);
	str_t var_exp = make_var_get_expanded(&make, STR("VAR"));
	EXPECT_STRN(var_exp.data, "VAL", var_exp.len);

	char buf[16] = { 0 };
	EXPECT_EQ(make_print(&make, PRINT_DST_BUF(buf, sizeof(buf), 0)), 11);
	EXPECT_STR(buf, "VAR := VAL\n");

	make_free(&make);

	END;
}

TEST(t_make_expand_print_var_inst2)
{
	START;

	make_t make = { 0 };
	make_init(&make, 1, 1, 1);

	make_var_t var = make_add_act(&make, make_create_var(&make, STRH("VAR"), MAKE_VAR_INST));
	make_var_add_val(&make, var, MSTR(STRH("VAL1")));
	make_var_add_val(&make, var, MSTR(STRH("VAL2")));

	EXPECT_EQ(make_expand(&make), 0);
	str_t var_exp = make_var_get_expanded(&make, STR("VAR"));
	EXPECT_STRN(var_exp.data, "VAL1 VAL2", var_exp.len);

	char buf[32] = { 0 };
	EXPECT_EQ(make_print(&make, PRINT_DST_BUF(buf, sizeof(buf), 0)), 17);
	EXPECT_STR(buf, "VAR := VAL1 VAL2\n");

	make_free(&make);

	END;
}

TEST(t_make_expand_print_var_app)
{
	START;

	make_t make = { 0 };
	make_init(&make, 1, 1, 1);

	make_var_add_val(&make, make_add_act(&make, make_create_var(&make, STRH("VAR"), MAKE_VAR_INST)), MSTR(STRH("VAL1")));
	make_var_add_val(&make, make_add_act(&make, make_create_var(&make, STRH("VAR"), MAKE_VAR_APP)), MSTR(STRH("VAL2")));

	EXPECT_EQ(make_expand(&make), 0);
	str_t var_exp = make_var_get_expanded(&make, STR("VAR"));
	EXPECT_STRN(var_exp.data, "VAL1 VAL2", var_exp.len);

	char buf[256] = { 0 };
	EXPECT_EQ(make_print(&make, PRINT_DST_BUF(buf, sizeof(buf), 0)), 24);
	EXPECT_STR(buf, "VAR := VAL1\n"
			"VAR += VAL2\n");

	EXPECT_EQ(make_dbg(&make, PRINT_DST_BUF(buf, sizeof(buf), 0)), 230);

	make_free(&make);

	END;
}

TEST(t_make_expand_print_var_ext_inst)
{
	START;

	make_t make = { 0 };
	make_init(&make, 1, 1, 1);

	make_var_add_val(&make, make_add_act(&make, make_create_var_ext(&make, STRH("VAR"), MAKE_VAR_INST)), MSTR(STR("VAL")));

	EXPECT_EQ(make_expand(&make), 0);
	str_t var_exp = make_var_get_expanded(&make, STR("VAR"));
	EXPECT_STRN(var_exp.data, "VAL", var_exp.len);

	char buf[8] = { 0 };
	EXPECT_EQ(make_print(&make, PRINT_DST_BUF(buf, sizeof(buf), 0)), 0);
	EXPECT_STR(buf, "");

	make_free(&make);

	END;
}

TEST(t_make_expand_print_var_ref)
{
	START;

	make_t make = { 0 };
	make_init(&make, 1, 1, 1);

	make_var_t ext1 = make_create_var_ext(&make, STRH("EXT1"), MAKE_VAR_INST);
	make_var_t ext2 = make_create_var_ext(&make, STRH("EXT2"), MAKE_VAR_INST);
	make_var_t ext3 = make_create_var_ext(&make, STRH("EXT3"), MAKE_VAR_INST);
	make_var_t ext4 = make_create_var_ext(&make, STRH("EXT4"), MAKE_VAR_INST);

	make_var_t var	   = make_add_act(&make, make_create_var(&make, STRH("VAR"), MAKE_VAR_APP));
	make_var_t var_app = make_add_act(&make, make_create_var(&make, STRH("VAR"), MAKE_VAR_APP));

	make_var_t out = make_add_act(&make, make_create_var(&make, STRH("OUT"), MAKE_VAR_INST));

	make_var_add_val(&make, var, MVAR(ext1));
	make_var_add_val(&make, var, MVAR(ext2));
	make_var_add_val(&make, var_app, MVAR(ext3));
	make_var_add_val(&make, var_app, MVAR(ext4));

	make_var_add_val(&make, out, MVAR(var));

	make_ext_set_val(&make, STR("EXT1"), MSTR(STR("VAL1")));
	make_ext_set_val(&make, STR("EXT2"), MSTR(STR("VAL2")));
	make_ext_set_val(&make, STR("EXT3"), MSTR(STR("VAL3")));
	make_ext_set_val(&make, STR("EXT4"), MSTR(STR("VAL4")));

	EXPECT_EQ(make_expand(&make), 0);
	str_t out_exp = make_var_get_expanded(&make, STR("OUT"));
	EXPECT_STRN(out_exp.data, "$(EXT1) $(EXT2) $(EXT3) $(EXT4)", out_exp.len);
	str_t out_res = make_var_get_resolved(&make, STR("OUT"));
	EXPECT_STRN(out_res.data, "VAL1 VAL2 VAL3 VAL4", out_res.len);

	char buf[64] = { 0 };
	EXPECT_EQ(make_print(&make, PRINT_DST_BUF(buf, sizeof(buf), 0)), 60);
	EXPECT_STR(buf, "VAR += $(EXT1) $(EXT2)\n"
			"VAR += $(EXT3) $(EXT4)\n"
			"OUT := $(VAR)\n");

	make_free(&make);

	END;
}

TEST(t_make_expand_print_if_empty)
{
	START;

	make_t make = { 0 };
	make_init(&make, 1, 1, 1);

	make_add_act(&make, make_create_if(&make, MSTR(str_null()), MSTR(str_null())));

	EXPECT_EQ(make_expand(&make), 0);

	char buf[64] = { 0 };
	EXPECT_EQ(make_print(&make, PRINT_DST_BUF(buf, sizeof(buf), 0)), 15);
	EXPECT_STR(buf, "ifeq (,)\n"
			"endif\n");

	EXPECT_EQ(make_dbg(&make, PRINT_DST_BUF(buf, sizeof(buf), 0)), 58);

	make_free(&make);

	END;
}

TEST(t_make_expand_print_if_lr)
{
	START;

	make_t make = { 0 };
	make_init(&make, 1, 1, 1);

	make_add_act(&make, make_create_if(&make, MSTR(STRH("L")), MSTR(STRH("R"))));

	EXPECT_EQ(make_expand(&make), 0);

	char buf[64] = { 0 };
	EXPECT_EQ(make_print(&make, PRINT_DST_BUF(buf, sizeof(buf), 0)), 18);
	EXPECT_STR(buf, "ifeq (L, R)\n"
			"endif\n");

	EXPECT_EQ(make_dbg(&make, PRINT_DST_BUF(buf, sizeof(buf), 0)), 62);

	make_free(&make);

	END;
}

TEST(t_make_expand_print_var_if_true)
{
	START;

	make_t make = { 0 };
	make_init(&make, 1, 1, 1);

	const make_var_t cond	= make_create_var_ext(&make, STRH("COND"), MAKE_VAR_INST);
	const make_if_t if_cond = make_add_act(&make, make_create_if(&make, MVAR(cond), MSTR(STRH("A"))));
	make_if_add_true_act(&make, if_cond, make_var_add_val(&make, make_create_var(&make, STRH("VAR"), MAKE_VAR_INST), MSTR(STRH("VAL"))));

	make_ext_set_val(&make, STR("COND"), MSTR(STR("A")));
	EXPECT_EQ(make_expand(&make), 0);
	str_t var_exp_a = make_var_get_expanded(&make, STR("VAR"));
	EXPECT_STRN(var_exp_a.data, "VAL", var_exp_a.len);

	make_ext_set_val(&make, STR("COND"), MSTR(STR("B")));
	EXPECT_EQ(make_expand(&make), 0);
	str_t var_exp_b = make_var_get_expanded(&make, STR("VAR"));
	EXPECT_STRN(var_exp_b.data, NULL, var_exp_b.len);

	char buf[512] = { 0 };
	EXPECT_EQ(make_print(&make, PRINT_DST_BUF(buf, sizeof(buf), 0)), 35);
	EXPECT_STR(buf, "ifeq ($(COND), A)\n"
			"VAR := VAL\n"
			"endif\n");

	EXPECT_EQ(make_dbg(&make, PRINT_DST_BUF(buf, sizeof(buf), 0)), 283);

	make_free(&make);

	END;
}

TEST(t_make_expand_print_var_if_false)
{
	START;

	make_t make = { 0 };
	make_init(&make, 1, 1, 1);

	const make_var_t cond	= make_create_var_ext(&make, STRH("COND"), MAKE_VAR_INST);
	const make_if_t if_cond = make_add_act(&make, make_create_if(&make, MVAR(cond), MSTR(STRH("A"))));
	make_if_add_true_act(&make, if_cond, make_var_add_val(&make, make_create_var(&make, STRH("VAR"), MAKE_VAR_INST), MSTR(STRH("VAL1"))));
	make_if_add_false_act(&make, if_cond, make_var_add_val(&make, make_create_var(&make, STRH("VAR"), MAKE_VAR_INST), MSTR(STRH("VAL2"))));

	make_ext_set_val(&make, STR("COND"), MSTR(STR("A")));
	EXPECT_EQ(make_expand(&make), 0);
	str_t var_exp_a = make_var_get_expanded(&make, STR("VAR"));
	EXPECT_STRN(var_exp_a.data, "VAL1", var_exp_a.len);

	make_ext_set_val(&make, STR("COND"), MSTR(STR("B")));
	EXPECT_EQ(make_expand(&make), 0);
	str_t var_exp_b = make_var_get_expanded(&make, STR("VAR"));
	EXPECT_STRN(var_exp_b.data, "VAL2", var_exp_b.len);

	char buf[512] = { 0 };
	EXPECT_EQ(make_print(&make, PRINT_DST_BUF(buf, sizeof(buf), 0)), 53);
	EXPECT_STR(buf, "ifeq ($(COND), A)\n"
			"VAR := VAL1\n"
			"else\n"
			"VAR := VAL2\n"
			"endif\n");

	EXPECT_EQ(make_dbg(&make, PRINT_DST_BUF(buf, sizeof(buf), 0)), 406);

	make_free(&make);

	END;
}

TEST(t_make_print_rule_empty)
{
	START;

	make_t make = { 0 };
	make_init(&make, 1, 1, 1);

	make_add_act(&make, make_create_rule(&make, MRULE(MSTR(STRH("rule"))), 1));

	char buf[64] = { 0 };
	EXPECT_EQ(make_print(&make, PRINT_DST_BUF(buf, sizeof(buf), 0)), 7);
	EXPECT_STR(buf, "rule:\n"
			"\n");

	EXPECT_EQ(make_dbg(&make, PRINT_DST_BUF(buf, sizeof(buf), 0)), 35);

	make_free(&make);

	END;
}

TEST(t_make_print_rule_empty_var)
{
	START;

	make_t make = { 0 };
	make_init(&make, 1, 1, 1);

	make_add_act(&make, make_create_rule(&make, MRULE(MVAR(MAKE_END)), 1));

	char buf[64] = { 0 };
	EXPECT_EQ(make_print(&make, PRINT_DST_BUF(buf, sizeof(buf), 0)), 3);
	EXPECT_STR(buf, ":\n"
			"\n");

	EXPECT_EQ(make_dbg(&make, PRINT_DST_BUF(buf, sizeof(buf), 0)), 31);

	make_free(&make);

	END;
}

TEST(t_make_print_rule_empty_action)
{
	START;

	make_t make = { 0 };
	make_init(&make, 1, 1, 1);

	make_add_act(&make, make_create_rule(&make, MRULEACT(MSTR(STRH("rule")), STRH("/action")), 1));

	char buf[64] = { 0 };
	EXPECT_EQ(make_print(&make, PRINT_DST_BUF(buf, sizeof(buf), 0)), 14);
	EXPECT_STR(buf, "rule/action:\n"
			"\n");

	EXPECT_EQ(make_dbg(&make, PRINT_DST_BUF(buf, sizeof(buf), 0)), 42);

	make_free(&make);

	END;
}

TEST(t_make_print_rule_depend)
{
	START;

	make_t make = { 0 };
	make_init(&make, 1, 1, 1);

	make_rule_t rule = make_add_act(&make, make_create_rule(&make, MRULE(MSTR(STRH("rule"))), 1));
	make_rule_add_depend(&make, rule, MRULE(MSTR(STRH("depend"))));

	char buf[64] = { 0 };
	EXPECT_EQ(make_print(&make, PRINT_DST_BUF(buf, sizeof(buf), 0)), 14);
	EXPECT_STR(buf, "rule: depend\n"
			"\n");

	EXPECT_EQ(make_dbg(&make, PRINT_DST_BUF(buf, sizeof(buf), 0)), 50);

	make_free(&make);

	END;
}

TEST(t_make_print_rule_depends)
{
	START;

	make_t make = { 0 };
	make_init(&make, 1, 1, 1);

	make_rule_t rule = make_add_act(&make, make_create_rule(&make, MRULE(MSTR(STRH("rule"))), 1));
	make_rule_add_depend(&make, rule, MRULE(MSTR(STRH("depend1"))));
	make_rule_add_depend(&make, rule, MRULE(MSTR(STRH("depend2"))));

	char buf[128] = { 0 };
	EXPECT_EQ(make_print(&make, PRINT_DST_BUF(buf, sizeof(buf), 0)), 23);
	EXPECT_STR(buf, "rule: depend1 depend2\n"
			"\n");

	EXPECT_EQ(make_dbg(&make, PRINT_DST_BUF(buf, sizeof(buf), 0)), 67);

	make_free(&make);

	END;
}

TEST(t_make_print_rule_acts)
{
	START;

	make_t make = { 0 };
	make_init(&make, 1, 1, 1);

	make_rule_t rule = make_add_act(&make, make_create_rule(&make, MRULE(MSTR(STRH("rule"))), 1));
	make_rule_add_act(&make, rule, make_create_cmd(&make, MCMD(STRH("cmd1"))));
	make_rule_add_act(&make, rule, make_create_cmd(&make, MCMD(STRH("cmd2"))));

	make_rule_t if_rule = make_rule_add_act(&make, rule, make_create_if(&make, MSTR(STRH("L")), MSTR(STRH("R"))));
	make_if_add_true_act(&make, if_rule, make_create_cmd(&make, MCMD(STRH("cmd3"))));
	make_if_add_false_act(&make, if_rule, make_create_cmd(&make, MCMD(STRH("cmd4"))));

	char buf[512] = { 0 };
	EXPECT_EQ(make_print(&make, PRINT_DST_BUF(buf, sizeof(buf), 0)), 54);
	EXPECT_STR(buf, "rule:\n"
			"\tcmd1\n"
			"\tcmd2\n"
			"ifeq (L, R)\n"
			"\tcmd3\n"
			"else\n"
			"\tcmd4\n"
			"endif\n"
			"\n");

	EXPECT_EQ(make_dbg(&make, PRINT_DST_BUF(buf, sizeof(buf), 0)), 263);

	make_free(&make);

	END;
}

TEST(t_make_print_cmd)
{
	START;

	make_t make = { 0 };
	make_init(&make, 1, 1, 1);

	make_rule_t rule = make_add_act(&make, make_create_rule(&make, MRULE(MSTR(STRH("rule"))), 1));
	make_rule_add_act(&make, rule, make_create_cmd(&make, MCMD(STRH("cmd"))));
	make_rule_add_act(&make, rule, make_create_cmd(&make, MCMDCHILD(STRH("dir"), str_null())));
	make_rule_add_act(&make, rule, make_create_cmd(&make, MCMDCHILD(STRH("dir"), STRH("action"))));
	make_rule_add_act(&make, rule, make_create_cmd(&make, MCMDERR(STRH("msg"))));

	char buf[256] = { 0 };
	EXPECT_EQ(make_print(&make, PRINT_DST_BUF(buf, sizeof(buf), 0)), 67);
	EXPECT_STR(buf, "rule:\n"
			"\tcmd\n"
			"\t@$(MAKE) -C dir\n"
			"\t@$(MAKE) -C dir action\n"
			"\t$(error msg)\n"
			"\n");

	EXPECT_EQ(make_dbg(&make, PRINT_DST_BUF(buf, sizeof(buf), 0)), 205);

	make_free(&make);

	END;
}

TEST(t_make_expand_print)
{
	SSTART;
	RUN(t_make_ext_set_val);
	RUN(t_make_expand);
	RUN(t_make_var_get_expanded);
	RUN(t_make_var_get_resolved);
	RUN(t_make_print);
	RUN(t_make_dbg);
	RUN(t_make_expand_print_empty);
	RUN(t_make_expand_print_var_inst_empty);
	RUN(t_make_expand_print_var_inst);
	RUN(t_make_expand_print_var_inst2);
	RUN(t_make_expand_print_var_app);
	RUN(t_make_expand_print_var_ext_inst);
	RUN(t_make_expand_print_var_ref);
	RUN(t_make_expand_print_if_empty);
	RUN(t_make_expand_print_if_lr);
	RUN(t_make_expand_print_var_if_true);
	RUN(t_make_expand_print_var_if_false);
	RUN(t_make_print_rule_empty);
	RUN(t_make_print_rule_empty_var);
	RUN(t_make_print_rule_empty_action);
	RUN(t_make_print_rule_depend);
	RUN(t_make_print_rule_depends);
	RUN(t_make_print_rule_acts);
	RUN(t_make_print_cmd);
	SEND;
}

STEST(t_make)
{
	SSTART;

	RUN(t_make_init_free);
	RUN(t_make_create);
	RUN(t_make_add);
	RUN(t_make_rule_get_target);
	RUN(t_make_expand_print);

	SEND;
}
