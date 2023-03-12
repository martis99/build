#ifndef MK_PROJ_H
#define MK_PROJ_H

#include "gen/proj.h"

int mk_proj_gen(const proj_t *proj, const hashmap_t *projects, const path_t *path, const prop_t *configs, const prop_t *langs, const prop_t *cflags, const prop_t *outdir,
		const prop_t *intdir);

#endif
