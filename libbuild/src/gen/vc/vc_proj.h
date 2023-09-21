#ifndef VC_PROJ_H
#define VC_PROJ_H

#include "gen/proj.h"

#include <stdio.h>

int vc_proj_gen_build(const proj_t *proj, const prop_t *sln_props, FILE *f);
int vc_proj_gen_launch(const proj_t *proj, const dict_t *projects, const prop_t *sln_props, FILE *f);

#endif
