#ifndef CM_PROJ_H
#define CM_PROJ_H

#include "gen/proj.h"

int cm_proj_gen(const proj_t *proj, const hashmap_t *projects, const path_t *path, const prop_t *langs, charset_t charset, const prop_t *cflags, const prop_t *outdir,
		const prop_t *intdir);

#endif
