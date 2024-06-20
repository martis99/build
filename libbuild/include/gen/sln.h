#ifndef SLN_H
#define SLN_H

#include "prop.h"

#include "dict.h"
#include "path.h"

typedef enum sln_prop_e {
	SLN_PROP_NAME,
	SLN_PROP_LANGS,
	SLN_PROP_DIRS,
	SLN_PROP_STARTUP,
	SLN_PROP_CONFIGS,
	SLN_PROP_ARCHS,
	SLN_PROP_CHARSET,
	SLN_PROP_FLAGS,
	SLN_PROP_OUTDIR,
	SLN_PROP_INTDIR,

	__SLN_PROP_MAX,
} sln_prop_t;

typedef struct sln_s {
	path_t path;
	pathv_t dir;
	char file[1024];
	prop_str_t data;
	prop_t props[__SLN_PROP_MAX];
	char guid[37];
	dict_t projects;
	dict_t dirs;
	arr_t build_order;
} sln_t;

int sln_read(sln_t *sln, const path_t *path);
void sln_print(sln_t *sln);
void sln_free(sln_t *sln);

#endif
