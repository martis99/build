#ifndef VC_PROJ_H
#define VC_PROJ_H

#include "gen/proj.h"

#include <stdio.h>

int vc_proj_gen_build(const proj_t *proj, const prop_t *configs, FILE *f);
int vc_proj_gen_launch(const proj_t *proj, const hashmap_t *projects, const prop_t *configs, const prop_t *outdir, FILE *f);

#endif
