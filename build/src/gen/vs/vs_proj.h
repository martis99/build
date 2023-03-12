#ifndef VS_PROJ_H
#define VS_PROJ_H

#include "gen/proj.h"

int vs_proj_gen(proj_t *proj, const hashmap_t *projects, const path_t *path, const prop_t *configs, const array_t *platforms, const prop_t *langs, const prop_t *charset,
		const prop_t *cflags, const prop_t *outdir, const prop_t *intdir);

#endif
