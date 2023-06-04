#ifndef VAR_H
#define VAR_H

typedef enum var_e {
	VAR_SLN_DIR,
	VAR_SLN_NAME,
	VAR_PROJ_DIR,
	VAR_PROJ_NAME,
	VAR_CONFIG,
	VAR_PLATFORM,

	__VAR_MAX,
} var_t;

typedef struct var_pol_s {
	const char *names[__VAR_MAX];
	const char *tos[__VAR_MAX];
} var_pol_t;

#endif
