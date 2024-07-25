#ifndef MK_PROJ_H
#define MK_PROJ_H

#include "gen/proj.h"

str_t *mk_proj_get_vars(const proj_t *proj, str_t *vars);

int mk_proj_gen(proj_t *proj, const dict_t *projects, const prop_t *sln_props);
void mk_proj_free(proj_t *proj);

#endif
