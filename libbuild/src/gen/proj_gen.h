#ifndef PROJ_GEN_H
#define PROJ_GEN_H

#include "gen/proj.h"

typedef str_t (*resolve_fn)(str_t str, str_t *buf, const proj_t *proj);
typedef str_t (*resolve_path_fn)(str_t rel, str_t path, str_t *buf);

int proj_gen(const proj_t *proj, const dict_t *projects, const prop_t *sln_props, resolve_fn resolve, resolve_path_fn resolve_path, pgc_t *gen);

#endif
