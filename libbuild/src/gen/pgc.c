#include "gen/pgc.h"

#include "pgc_types.h"

// clang-format off
static const char *str_str[] = {
	[PGC_STR_NAME]	  = "NAME",
	[PGC_STR_OUTDIR]  = "OUTDIR",
	[PGC_STR_COVDIR]  = "COVDIR",
	[PGC_STR_ARGS]	  = "ARGS",
	[PGC_STR_LDFLAGS] = "LDFLAGS",
	[PGC_STR_HEADER]  = "HEADER",
	[PGC_STR_SIZE]	  = "SIZE",
	[PGC_STR_URL]	  = "URL",
	[PGC_STR_FORMAT]  = "FORMAT",
	[PGC_STR_CONFIG]  = "CONFIG",
	[PGC_STR_TARGETS] = "TARGETS",
};

static const char *build_type_str[] = {
	[PGC_BUILD_EXE]	   = "EXE",
	[PGC_BUILD_STATIC] = "STATIC",
	[PGC_BUILD_SHARED] = "SHARED",
	[PGC_BUILD_ELF]	   = "ELF",
	[PGC_BUILD_BIN]	   = "BIN",
	[PGC_BUILD_FAT12]  = "FAT12",
};

static const char *arr_str[] = {
	[PGC_ARR_HEADERS]   = "HEADERS",
	[PGC_ARR_CONFIGS]   = "CONFIGS",
	[PGC_ARR_SRCS]	    = "SRCS",
	[PGC_ARR_INCLUDES]  = "INCLUDES",
	[PGC_ARR_LIBS]	    = "LIBS",
	[PGC_ARR_DEPENDS]   = "DEPENDS",
	[PGC_ARR_FILES]	    = "FILES",
	[PGC_ARR_REQUIRES]  = "REQUIRES",
	[PGC_ARR_COPYFILES] = "COPYFILES",
};
// clang-format on

static const char *intdir_type_str[] = {
	[PGC_INTDIR_OBJECT] = "OBJECT",
	[PGC_INTDIR_STATIC] = "STATIC",
	[PGC_INTDIR_SHARED] = "SHARED",
};

static const char *src_type_str[] = {
	[PGC_SRC_NASM] = "NASM",
	[PGC_SRC_S]    = "S",
	[PGC_SRC_C]    = "C",
	[PGC_SRC_CPP]  = "CPP",
};

static const char *intdir_str_str[] = {
	[PGC_INTDIR_STR_INTDIR]	 = "INTDIR",
	[PGC_INTDIR_STR_DEFINES] = "DEFINES",
};

static const char *target_str_str[] = {
	[PGC_TARGET_STR_RUN]	  = "RUN",
	[PGC_TARGET_STR_RUN_DBG]  = "RUN_DBG",
	[PGC_TARGET_STR_ARTIFACT] = "ARTIFACT",
};

static const char *src_str_str[] = { [PGC_SRC_STR_FLAGS] = "FLAGS" };

static void pgc_str_free(void *ptr)
{
	str_t *str = ptr;
	str_free(str);
}

static int pgc_str_dprint(void *ptr, print_dst_t dst)
{
	str_t *str = ptr;

	int off = dst.off;

	if (str->data) {
		dst.off += dprintf(dst, "    %.*s\n", str->len, str->data);
	}

	return dst.off - off;
}

static void pgc_str_flag_free(void *ptr)
{
	pgc_str_flags_t *data = ptr;
	str_free(&data->str);
}

static int pgc_str_flag_dprint(void *ptr, print_dst_t dst)
{
	pgc_str_flags_t *data = ptr;

	int off = dst.off;

	if (data->str.data) {
		dst.off += dprintf(dst, "    %.*s (0x%04x)\n", data->str.len, data->str.data, data->flags);
	}

	return dst.off - off;
}

static void pgc_lib_free(void *ptr)
{
	pgc_lib_data_t *lib = ptr;
	str_free(&lib->dir);
	str_free(&lib->name);
}

static int pgc_lib_dprint(void *ptr, print_dst_t dst)
{
	pgc_lib_data_t *lib = ptr;

	int off = dst.off;

	if (lib->dir.data || lib->name.data) {
		dst.off += dprintf(dst, "    dir: %.*s, name: %.*s\n", lib->dir.len, lib->dir.data, lib->name.len, lib->name.data);
	}

	return dst.off - off;
}

// clang-format off
struct {
	size_t size;
	void (*free)(void *ptr);
	int (*dprint)(void *ptr, print_dst_t dst);
} s_arr_c[] = {
	[PGC_ARR_HEADERS]   = { sizeof(pgc_str_flags_t), pgc_str_flag_free, pgc_str_flag_dprint },
	[PGC_ARR_CONFIGS]   = { sizeof(str_t),		 pgc_str_free,	    pgc_str_dprint },
	[PGC_ARR_SRCS]	    = { sizeof(pgc_str_flags_t), pgc_str_flag_free, pgc_str_flag_dprint },
	[PGC_ARR_INCLUDES]  = { sizeof(str_t),		 pgc_str_free,	    pgc_str_dprint },
	[PGC_ARR_LIBS]	    = { sizeof(pgc_lib_data_t),	 pgc_lib_free,	    pgc_lib_dprint },
	[PGC_ARR_DEPENDS]   = { sizeof(str_t),		 pgc_str_free,	    pgc_str_dprint },
	[PGC_ARR_FILES]	    = { sizeof(pgc_str_flags_t), pgc_str_flag_free, pgc_str_flag_dprint },
	[PGC_ARR_REQUIRES]  = { sizeof(str_t),		 pgc_str_free,	    pgc_str_dprint },
	[PGC_ARR_COPYFILES] = { sizeof(str_t),		 pgc_str_free,	    pgc_str_dprint },
};
// clang-format on

pgc_t *pgc_init(pgc_t *pgc)
{
	if (pgc == NULL) {
		return NULL;
	}

	for (pgc_arr_t i = 0; i < __PGC_ARR_MAX; i++) {
		if (arr_init(&pgc->arr[i], 1, s_arr_c[i].size) == NULL) {
			return NULL;
		}
	}

	return pgc;
}

void pgc_free(pgc_t *pgc)
{
	if (pgc == NULL) {
		return;
	}

	for (pgc_str_t i = 0; i < __PGC_STR_MAX; i++) {
		str_free(&pgc->str[i]);
	}

	for (pgc_arr_t i = 0; i < __PGC_ARR_MAX; i++) {
		void *val;
		arr_foreach(&pgc->arr[i], val)
		{
			s_arr_c[i].free(val);
		}
		arr_free(&pgc->arr[i]);
	}

	for (pgc_str_t s = 0; s < __PGC_INTDIR_STR_MAX; s++) {
		for (pgc_intdir_type_t i = 0; i < __PGC_INTDIR_TYPE_MAX; i++) {
			str_free(&pgc->intdir[s][i]);
		}
	}

	for (pgc_str_t s = 0; s < __PGC_TARGET_STR_MAX; s++) {
		for (pgc_build_type_t i = 0; i < __PGC_BUILD_TYPE_MAX; i++) {
			str_free(&pgc->target[s][i]);
		}
	}

	for (pgc_str_t s = 0; s < __PGC_SRC_STR_MAX; s++) {
		for (pgc_src_type_t i = 0; i < __PGC_SRC_TYPE_MAX; i++) {
			str_free(&pgc->src[s][i]);
		}
	}
}

static uint add_str(pgc_t *pgc, pgc_arr_t arr, str_t str)
{
	if (pgc == NULL) {
		return PGC_END;
	}

	uint id = arr_add(&pgc->arr[arr]);

	str_t *data = arr_get(&pgc->arr[arr], id);
	if (data == NULL) {
		return PGC_END;
	}

	*data = str;

	return id;
}

static uint add_str_flags(pgc_t *pgc, pgc_arr_t arr, str_t str, int flags)
{
	if (pgc == NULL) {
		return PGC_END;
	}

	uint id = arr_add(&pgc->arr[arr]);

	pgc_str_flags_t *data = arr_get(&pgc->arr[arr], id);
	if (data == NULL) {
		return PGC_END;
	}

	*data = (pgc_str_flags_t){
		.str   = str,
		.flags = flags,
	};

	return id;
}

//TODO: Add ability to set config name independent from config settings
uint pgc_add_config(pgc_t *pgc, str_t config)
{
	return add_str(pgc, PGC_ARR_CONFIGS, config);
}

uint pgc_add_header(pgc_t *pgc, str_t dir, int exts)
{
	return add_str_flags(pgc, PGC_ARR_HEADERS, dir, exts);
}

uint pgc_add_src(pgc_t *pgc, str_t dir, int exts)
{
	return add_str_flags(pgc, PGC_ARR_SRCS, dir, exts);
}

uint pgc_add_include(pgc_t *pgc, str_t dir)
{
	return add_str(pgc, PGC_ARR_INCLUDES, dir);
}

void pgc_add_flag(pgc_t *pgc, str_t flag, int exts)
{
	if (pgc == NULL) {
		return;
	}

	for (pgc_src_type_t s = 0; s < __PGC_SRC_TYPE_MAX; s++) {
		if ((exts & (1 << s)) == 0) {
			continue;
		}

		if (pgc->src[PGC_SRC_STR_FLAGS][s].data == NULL) {
			pgc->src[PGC_SRC_STR_FLAGS][s] = strz(16);
		}

		if (pgc->src[PGC_SRC_STR_FLAGS][s].len > 0) {
			str_cat(&pgc->src[PGC_SRC_STR_FLAGS][s], STR(" "));
		}
		str_cat(&pgc->src[PGC_SRC_STR_FLAGS][s], flag);
	}
}

void pgc_add_define(pgc_t *pgc, str_t define, int intdirs)
{
	if (pgc == NULL) {
		return;
	}

	for (pgc_intdir_type_t i = 0; i < __PGC_INTDIR_TYPE_MAX; i++) {
		if ((intdirs & (1 << i)) == 0) {
			continue;
		}

		if (pgc->intdir[PGC_INTDIR_STR_DEFINES][i].data == NULL) {
			pgc->intdir[PGC_INTDIR_STR_DEFINES][i] = strz(16);
		}

		str_cat(&pgc->intdir[PGC_INTDIR_STR_DEFINES][i], pgc->intdir[PGC_INTDIR_STR_DEFINES][i].len > 0 ? STR(" -D") : STR("-D"));
		str_cat(&pgc->intdir[PGC_INTDIR_STR_DEFINES][i], define);
	}
}

void pgc_add_ldflag(pgc_t *pgc, str_t ldflag)
{
	if (pgc == NULL) {
		return;
	}

	if (pgc->str[PGC_STR_LDFLAGS].data == NULL) {
		pgc->str[PGC_STR_LDFLAGS] = strz(16);
	}

	if (pgc->str[PGC_STR_LDFLAGS].len > 0) {
		str_cat(&pgc->str[PGC_STR_LDFLAGS], STR(" "));
	}
	str_cat(&pgc->str[PGC_STR_LDFLAGS], ldflag);
}

uint pgc_add_lib(pgc_t *pgc, str_t dir, str_t name, pgc_link_type_t link_type, pgc_lib_type_t lib_type)
{
	if (pgc == NULL) {
		return PGC_END;
	}

	uint id = arr_add(&pgc->arr[PGC_ARR_LIBS]);

	pgc_lib_data_t *data = arr_get(&pgc->arr[PGC_ARR_LIBS], id);
	if (data == NULL) {
		return PGC_END;
	}

	*data = (pgc_lib_data_t){
		.dir	   = dir,
		.name	   = name,
		.link_type = link_type,
		.lib_type  = lib_type,
	};

	return id;
}

uint pgc_add_depend(pgc_t *pgc, str_t depend)
{
	return add_str(pgc, PGC_ARR_DEPENDS, depend);
}

void pgc_set_run(pgc_t *pgc, str_t run, int build)
{
	if (pgc == NULL) {
		return;
	}

	for (pgc_build_type_t b = 0; b < __PGC_BUILD_TYPE_MAX; b++) {
		if ((build & (1 << b)) == 0) {
			continue;
		}

		pgc->target[PGC_TARGET_STR_RUN][b] = run;
	}
}

void pgc_set_run_debug(pgc_t *pgc, str_t run, int build)
{
	if (pgc == NULL) {
		return;
	}

	for (pgc_build_type_t b = 0; b < __PGC_BUILD_TYPE_MAX; b++) {
		if ((build & (1 << b)) == 0) {
			continue;
		}

		pgc->target[PGC_TARGET_STR_RUN_DBG][b] = run;
	}
}

uint pgc_add_file(pgc_t *pgc, str_t path, int ext)
{
	return add_str_flags(pgc, PGC_ARR_FILES, path, ext);
}

uint pgc_add_require(pgc_t *pgc, str_t require)
{
	return add_str(pgc, PGC_ARR_REQUIRES, require);
}

uint pgc_add_copyfile(pgc_t *pgc, str_t path)
{
	return add_str(pgc, PGC_ARR_COPYFILES, path);
}

int pgc_print(const pgc_t *pgc, print_dst_t dst)
{
	if (pgc == NULL) {
		return 0;
	}

	int off = dst.off;

	for (pgc_str_t i = 0; i < __PGC_STR_MAX; i++) {
		if (!pgc->str[i].data || pgc->str[i].len == 0) {
			continue;
		}
		dst.off += dprintf(dst, "%s: %.*s\n", str_str[i], pgc->str[i].len, pgc->str[i].data);
	}

	for (pgc_arr_t i = 0; i < __PGC_ARR_MAX; i++) {
		if (pgc->arr[i].cnt == 0) {
			continue;
		}

		dst.off += dprintf(dst, "%s\n", arr_str[i]);
		void *val;
		arr_foreach(&pgc->arr[i], val)
		{
			dst.off += s_arr_c[i].dprint(val, dst);
		}
	}

	for (pgc_str_t s = 0; s < __PGC_INTDIR_STR_MAX; s++) {
		int first = 1;
		for (pgc_intdir_type_t i = 0; i < __PGC_INTDIR_TYPE_MAX; i++) {
			if (!pgc->intdir[s][i].data || pgc->intdir[s][i].len == 0) {
				continue;
			}
			if (first) {
				dst.off += dprintf(dst, "%s\n", intdir_str_str[s]);
				first = 0;
			}
			dst.off += dprintf(dst, "    %s: %.*s\n", intdir_type_str[i], pgc->intdir[s][i].len, pgc->intdir[s][i].data);
		}
	}

	for (pgc_str_t s = 0; s < __PGC_TARGET_STR_MAX; s++) {
		int first = 1;
		for (pgc_build_type_t i = 0; i < __PGC_BUILD_TYPE_MAX; i++) {
			if (!pgc->target[s][i].data || pgc->target[s][i].len == 0) {
				continue;
			}
			if (first) {
				dst.off += dprintf(dst, "%s\n", target_str_str[s]);
				first = 0;
			}
			dst.off += dprintf(dst, "    %s: %.*s\n", build_type_str[i], pgc->target[s][i].len, pgc->target[s][i].data);
		}
	}

	for (pgc_str_t s = 0; s < __PGC_SRC_STR_MAX; s++) {
		int first = 1;
		for (pgc_src_type_t i = 0; i < __PGC_SRC_TYPE_MAX; i++) {
			if (!pgc->src[s][i].data || pgc->src[s][i].len == 0) {
				continue;
			}
			if (first) {
				dst.off += dprintf(dst, "%s\n", src_str_str[s]);
				first = 0;
			}
			dst.off += dprintf(dst, "    %s: %.*s\n", src_type_str[i], pgc->src[s][i].len, pgc->src[s][i].data);
		}
	}

	return dst.off - off;
}