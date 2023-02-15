#ifndef VAR_H
#define VAR_H

typedef struct var_pol_s {
	const char *names[64];
	const char *tos[64];
} var_pol_t;

typedef enum var_e {
	VAR_SLN_DIR,
	VAR_SLN_NAME,
	VAR_PROJ_DIR,
	VAR_PROJ_NAME,
	VAR_CONFIG,
	VAR_PLATFORM,

	__VAR_MAX,
} var_t;

#endif
