#ifndef PGC_GEN_H
#define PGC_GEN_H

#include "gen/pgc.h"

void pgc_gen_empty(pgc_t *pgc);
void pgc_gen_args(pgc_t *pgc);
void pgc_gen_require(pgc_t *pgc);
void pgc_gen_lib_empty(pgc_t *pgc);
void pgc_gen_headers(pgc_t *pgc);
void pgc_gen_includes(pgc_t *pgc);
void pgc_gen_flags(pgc_t *pgc);
void pgc_gen_ldflags(pgc_t *pgc);
void pgc_gen_libs(pgc_t *pgc);
void pgc_gen_depends(pgc_t *pgc);
void pgc_gen_coverage_exe(pgc_t *pgc);
void pgc_gen_coverage_static(pgc_t *pgc);
void pgc_gen_coverage_shared(pgc_t *pgc);
void pgc_gen_defines_exe(pgc_t *pgc);
void pgc_gen_defines_static(pgc_t *pgc);
void pgc_gen_defines_shared(pgc_t *pgc);
void pgc_gen_copyfiles(pgc_t *pgc);
void pgc_gen_run(pgc_t *pgc);
void pgc_gen_run_debug(pgc_t *pgc);
void pgc_gen_run_run_debug(pgc_t *pgc);
void pgc_gen_artifact_exe(pgc_t *pgc);
void pgc_gen_artifact_lib(pgc_t *pgc);
void pgc_gen_bin_obj(pgc_t *pgc);
void pgc_gen_bin_files(pgc_t *pgc);
void pgc_gen_elf(pgc_t *pgc);
void pgc_gen_fat12(pgc_t *pgc);
void pgc_gen_fat12_header(pgc_t *pgc);
void pgc_gen_archs(pgc_t *pgc);
void pgc_gen_configs(pgc_t *pgc);
void pgc_gen_nasm_bin(pgc_t *pgc);
void pgc_gen_nasm_exe(pgc_t *pgc);
void pgc_gen_nasm_static(pgc_t *pgc);
void pgc_gen_nasm_shared(pgc_t *pgc);
void pgc_gen_asm_exe(pgc_t *pgc);
void pgc_gen_asm_static(pgc_t *pgc);
void pgc_gen_asm_shared(pgc_t *pgc);
void pgc_gen_c_exe(pgc_t *pgc);
void pgc_gen_c_static(pgc_t *pgc);
void pgc_gen_c_shared(pgc_t *pgc);
void pgc_gen_cpp_exe(pgc_t *pgc);
void pgc_gen_cpp_static(pgc_t *pgc);
void pgc_gen_cpp_shared(pgc_t *pgc);
void pgc_gen_url(pgc_t *pgc);

#endif
