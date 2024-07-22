#ifndef CM_PROJ_H
#define CM_PROJ_H

#include "gen/proj.h"

int cm_proj_gen(proj_t *proj, const dict_t *projects, const prop_t *sln_props);
void cm_proj_free(proj_t *proj);

#endif
