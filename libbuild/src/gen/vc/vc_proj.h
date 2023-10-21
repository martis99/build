#ifndef VC_PROJ_H
#define VC_PROJ_H

#include "gen/proj.h"

#include "json.h"

#include <stdio.h>

int vc_proj_gen_build(const proj_t *proj, const prop_t *sln_props, json_t *json, json_val_t tasks);
int vc_proj_gen_launch(proj_t *proj, const dict_t *projects, const prop_t *sln_props, json_t *json, json_val_t confs);

#endif
