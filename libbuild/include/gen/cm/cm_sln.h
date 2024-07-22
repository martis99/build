#ifndef CM_SLN_H
#define CM_SLN_H

#include "gen/sln.h"

int cm_sln_gen(sln_t *sln, const path_t *path);
void cm_sln_free(sln_t *sln);

#endif
