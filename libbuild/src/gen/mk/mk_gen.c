#include "mk_gen.h"

#include "make.h"

typedef struct mk_gen_dep_data_s {
	str_t dir;
	mk_gen_inc_t incs;
} mk_gen_inc_data_t;

typedef struct mk_gen_src_data_s {
	str_t dir;
	mk_gen_src_t srcs;
} mk_gen_src_data_t;

mk_gen_t *mk_gen_init(mk_gen_t *gen)
{
	if (gen == NULL) {
		return NULL;
	}

	if (arr_init(&gen->incs, 1, sizeof(mk_gen_inc_t)) == NULL) {
		return NULL;
	}

	if (arr_init(&gen->srcs, 1, sizeof(mk_gen_src_t)) == NULL) {
		return NULL;
	}

	return gen;
}

void mk_gen_free(mk_gen_t *gen)
{
	if (gen == NULL) {
		return;
	}

	str_free(&gen->outdir);
	str_free(&gen->intdir);

	mk_gen_inc_data_t *inc;
	arr_foreach(&gen->incs, inc)
	{
		str_free(&inc->dir);
	}
	arr_free(&gen->incs);

	mk_gen_src_data_t *src;
	arr_foreach(&gen->srcs, src)
	{
		str_free(&src->dir);
	}
	arr_free(&gen->srcs);

	str_free(&gen->target);
	str_free(&gen->args);
}

uint mk_gen_add_inc(mk_gen_t *gen, str_t dir, mk_gen_inc_t incs)
{
	if (gen == NULL) {
		return;
	}

	uint id = arr_add(&gen->incs);

	mk_gen_inc_data_t *inc = arr_get(&gen->incs, id);
	if (inc == NULL) {
		return ARR_END;
	}

	*inc = (mk_gen_inc_data_t){
		.dir  = dir,
		.incs = incs,
	};

	return id;
}

uint mk_gen_add_src(mk_gen_t *gen, str_t dir, mk_gen_src_t srcs)
{
	if (gen == NULL) {
		return;
	}

	uint id = arr_add(&gen->srcs);

	mk_gen_src_data_t *src = arr_get(&gen->srcs, id);
	if (src == NULL) {
		return ARR_END;
	}

	*src = (mk_gen_src_data_t){
		.dir  = dir,
		.srcs = srcs,
	};

	return id;
}

int mk_gen_print(const mk_gen_t *gen, print_dst_t dst)
{
	if (gen == NULL) {
		return;
	}

	int off = dst.off;

	make_t make = { 0 };
	make_init(&make, 8, 8, 8);

	make_create_var_ext(&make, STR("SLNDIR"), MAKE_VAR_INST);
	const make_var_t platform = make_create_var_ext(&make, STR("PLATFORM"), MAKE_VAR_INST);
	const make_var_t config	  = make_create_var_ext(&make, STR("CONFIG"), MAKE_VAR_INST);

	make_var_t outdir = make_add_act(&make, make_create_var(&make, STR("OUTDIR"), MAKE_VAR_INST));
	make_var_add_val(&make, outdir, MSTR(outdir));
	const make_var_t intdir = make_add_act(&make, make_create_var(&make, STR("INTDIR"), MAKE_VAR_INST));
	make_var_add_val(&make, intdir, MSTR(intdir));

	make_var_t repdir = MAKE_END;
	if (outdir != MAKE_END && gen->flags & MK_GEN_COVERAGE) {
		repdir = make_add_act(&make, make_create_var(&make, STR("REPDIR"), MAKE_VAR_INST));
		make_var_add_val(&make, repdir, MSTR(STR("$(OUTDIR)coverage-report/")));
	}

	static const char *inc_ext[] = {
		[MK_INC_INC] = "inc",
		[MK_INC_H]   = "h",
		[MK_INC_HPP] = "hpp",
	};

	make_var_t deps = MAKE_END;
	mk_gen_inc_data_t *inc;
	arr_foreach(&gen->incs, inc)
	{
		for (mk_gen_inc_t ext = 0; ext < __MK_INC_MAX; ext++) {
			if (inc->incs & (1 << ext) == 0) {
				continue;
			}

			deps = make_add_act(&make, make_create_var(&make, STR("DEPS"), deps == MAKE_END ? MAKE_VAR_INST : MAKE_VAR_APP));
			make_var_add_val(&make, deps, MSTR(strf("$(shell find %.*s -name '*.%s')", inc->dir.len, inc->dir.data, inc_ext[ext])));
		}
	}

	make_var_t src_var[] = {
		[MK_SRC_ASM_BIN] = MAKE_END,
		[MK_SRC_ASM_OBJ] = MAKE_END,
		[MK_SRC_C]	 = MAKE_END,
		[MK_SRC_CPP]	 = MAKE_END,
	};

	static const char *src_var_name[] = {
		[MK_SRC_ASM_BIN] = "SRC_ASM_BIN",
		[MK_SRC_ASM_OBJ] = "SRC_ASM_OBJ",
		[MK_SRC_C]	 = "SRC_C",
		[MK_SRC_CPP]	 = "SRC_CPP",
	};

	static const char *src_ext[] = {
		[MK_SRC_ASM_BIN] = "asm",
		[MK_SRC_ASM_OBJ] = "asm",
		[MK_SRC_C]	 = "c",
		[MK_SRC_CPP]	 = "cpp",
	};

	for (mk_gen_src_t ext = 0; ext < __MK_SRC_MAX; ext++) {
		mk_gen_src_data_t *src;
		arr_foreach(&gen->srcs, src)
		{
			if (src->srcs & (1 << ext) == 0) {
				continue;
			}

			src_var[ext] = make_add_act(&make, make_create_var(&make, STR(src_var_name[ext]), src_var[ext] == MAKE_END ? MAKE_VAR_INST : MAKE_VAR_APP));
			make_var_add_val(&make, src_var[ext], MSTR(strf("$(shell find %.*s -name '*.%s')", src->dir.len, src->dir.data, src_ext[ext])));
		}
	}

	make_var_t obj_var[] = {
		[MK_SRC_ASM_BIN] = MAKE_END,
		[MK_SRC_ASM_OBJ] = MAKE_END,
		[MK_SRC_C]	 = MAKE_END,
		[MK_SRC_CPP]	 = MAKE_END,
	};

	static const char *obj_var_name[] = {
		[MK_SRC_ASM_BIN] = "BIN_ASM",
		[MK_SRC_ASM_OBJ] = "OBJ_ASM",
		[MK_SRC_C]	 = "OBJ_C",
		[MK_SRC_CPP]	 = "OBJ_CPP",
	};

	static const char *obj_ext[] = {
		[MK_SRC_ASM_BIN] = "bin",
		[MK_SRC_ASM_OBJ] = "o",
		[MK_SRC_C]	 = "o",
		[MK_SRC_CPP]	 = "o",
	};

	for (mk_gen_src_t ext = 0; ext < __MK_SRC_MAX; ext++) {
		if (src_var[ext] == MAKE_END) {
			continue;
		}

		obj_var[ext] = make_add_act(&make, make_create_var(&make, STR(obj_var_name[ext]), MAKE_VAR_INST));
		make_var_add_val(&make, obj_var[ext], MSTR(strf("$(patsubst %.%s, $(INTDIR)%.%s, $(%s))", src_ext[ext], obj_ext[ext], src_var_name[ext])));
	}

	make_free(&make);
}
