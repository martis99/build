#ifndef SLN_H
#define SLN_H

#include "types.h"
#include "utils_types.h"
#include "hash_map.h"
#include "utils_types.h"

typedef enum sln_prop_e {
	SLN_PROP_NAME,
	SLN_PROP_LANGS,
	SLN_PROP_DIRS,
	SLN_PROP_STARTUP,
	SLN_PROP_CONFIGS,
	SLN_PROP_PLATFORMS,
	SLN_PROP_CHARSET,
	SLN_PROP_OUTDIR,
	SLN_PROP_INTDIR,

	__SLN_PROP_MAX,
} sln_prop_t;

typedef struct sln_s {
	path_t path;
	path_t file_path;
	char file[1024];
	prop_str_t data;
	prop_t props[__SLN_PROP_MAX];
	char guid[37];
	hashmap_t projects;
	hashmap_t dirs;
} sln_t;

int sln_read(sln_t *sln, const path_t *path);
void sln_print(sln_t *sln);
int sln_gen_cmake(const sln_t *sln, const path_t *path);
int sln_gen_make(const sln_t *sln, const path_t *path);
int sln_gen_vs(const sln_t *sln, const path_t *path);
void sln_free(sln_t *sln);

#endif