#include "pgc_gen.h"

void pgc_gen_empty(pgc_t *pgc)
{
	pgc_init(pgc);
}

void pgc_gen_args(pgc_t *pgc)
{
	pgc_init(pgc);

	pgc->str[PGC_STR_NAME]				      = STRH("test");
	pgc->str[PGC_STR_OUTDIR]			      = STRH("$(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/");
	pgc->intdir[PGC_INTDIR_STR_INTDIR][PGC_INTDIR_OBJECT] = STRH("$(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/int/");
	pgc->target[PGC_TARGET_STR_TARGET][PGC_BUILD_EXE]     = STRH("$(OUTDIR)test");
	pgc->str[PGC_STR_ARGS]				      = STRH("-D");

	pgc->builds = F_PGC_BUILD_EXE;

	pgc_add_arch(pgc, STRH("x86_64"));
	pgc_add_config(pgc, STRH("Debug"));
	pgc_add_src(pgc, STRH("src/"), F_PGC_SRC_C);
}

void pgc_gen_cwd(pgc_t *pgc)
{
	pgc_init(pgc);

	pgc->str[PGC_STR_NAME]				      = STRH("test");
	pgc->str[PGC_STR_OUTDIR]			      = STRH("$(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/");
	pgc->intdir[PGC_INTDIR_STR_INTDIR][PGC_INTDIR_OBJECT] = STRH("$(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/int/");
	pgc->target[PGC_TARGET_STR_TARGET][PGC_BUILD_EXE]     = STRH("$(OUTDIR)test");
	pgc->str[PGC_STR_CWD]				      = STRH("projects/test");

	pgc->builds = F_PGC_BUILD_EXE;

	pgc_add_arch(pgc, STRH("x86_64"));
	pgc_add_config(pgc, STRH("Debug"));
	pgc_add_src(pgc, STRH("src/"), F_PGC_SRC_C);
}

void pgc_gen_require(pgc_t *pgc)
{
	pgc_init(pgc);

	pgc_add_require(pgc, STRH("package"));
}

void pgc_gen_lib_empty(pgc_t *pgc)
{
	pgc_init(pgc);

	pgc->str[PGC_STR_NAME]	 = STRH("test");
	pgc->str[PGC_STR_OUTDIR] = STRH("$(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/");

	pgc->builds = F_PGC_BUILD_STATIC | F_PGC_BUILD_SHARED;
}

void pgc_gen_headers(pgc_t *pgc)
{
	pgc_init(pgc);

	pgc->str[PGC_STR_NAME] = STRH("test");

	pgc_add_header(pgc, STRH("src/"), F_PGC_HEADER_H);
}

void pgc_gen_includes(pgc_t *pgc)
{
	pgc_init(pgc);

	pgc->str[PGC_STR_NAME]				      = STRH("test");
	pgc->str[PGC_STR_OUTDIR]			      = STRH("$(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/");
	pgc->intdir[PGC_INTDIR_STR_INTDIR][PGC_INTDIR_OBJECT] = STRH("$(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/int/");

	pgc_add_arch(pgc, STRH("x86_64"));
	pgc_add_src(pgc, STRH("src/"), F_PGC_SRC_C);
	pgc_add_include(pgc, STRH("src/"), PGC_SCOPE_PRIVATE);
	pgc_add_include(pgc, STRH("include/"), PGC_SCOPE_PUBLIC);
}

void pgc_gen_flags(pgc_t *pgc)
{
	pgc_init(pgc);

	pgc->str[PGC_STR_NAME]				      = STRH("test");
	pgc->str[PGC_STR_OUTDIR]			      = STRH("$(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/");
	pgc->intdir[PGC_INTDIR_STR_INTDIR][PGC_INTDIR_OBJECT] = STRH("$(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/int/");

	pgc_add_arch(pgc, STRH("x86_64"));
	pgc_add_src(pgc, STRH("src/"), F_PGC_SRC_C);
	pgc_add_flag(pgc, STR("-Wall"), F_PGC_SRC_C);
	pgc_add_flag(pgc, STR("-Wextra"), F_PGC_SRC_C);
}

void pgc_gen_ldflags(pgc_t *pgc)
{
	pgc_init(pgc);

	pgc->str[PGC_STR_NAME]				      = STRH("test");
	pgc->str[PGC_STR_OUTDIR]			      = STRH("$(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/");
	pgc->intdir[PGC_INTDIR_STR_INTDIR][PGC_INTDIR_OBJECT] = STRH("$(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/int/");
	pgc->target[PGC_TARGET_STR_TARGET][PGC_BUILD_EXE]     = STRH("$(OUTDIR)test");

	pgc->builds = F_PGC_BUILD_EXE;

	pgc_add_arch(pgc, STRH("x86_64"));
	pgc_add_src(pgc, STRH("src/"), F_PGC_SRC_C);
	pgc_add_ldflag(pgc, STR("-lm"));
	pgc_add_ldflag(pgc, STR("-lpthread"));
}

void pgc_gen_libs(pgc_t *pgc)
{
	pgc_init(pgc);

	pgc->str[PGC_STR_NAME]				      = STRH("test");
	pgc->str[PGC_STR_OUTDIR]			      = STRH("$(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/");
	pgc->intdir[PGC_INTDIR_STR_INTDIR][PGC_INTDIR_OBJECT] = STRH("$(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/int/");
	pgc->target[PGC_TARGET_STR_TARGET][PGC_BUILD_EXE]     = STRH("$(OUTDIR)test");

	pgc->builds = F_PGC_BUILD_EXE;

	pgc_add_arch(pgc, STRH("x86_64"));
	pgc_add_src(pgc, STRH("src/"), F_PGC_SRC_C);
	pgc_add_lib(pgc, STRH("libs/"), str_null(), PGC_LINK_STATIC, PGC_LIB_INT);
	pgc_add_lib(pgc, STRH("libs/"), STRH("lib"), PGC_LINK_STATIC, PGC_LIB_INT);
	pgc_add_lib(pgc, STRH("libs/"), STRH("lib"), PGC_LINK_STATIC, PGC_LIB_EXT);
	pgc_add_lib(pgc, STRH("libs/"), STRH("lib"), PGC_LINK_SHARED, PGC_LIB_INT);
	pgc_add_lib(pgc, STRH("libs/"), STRH("lib"), PGC_LINK_SHARED, PGC_LIB_EXT);
}

void pgc_gen_depends(pgc_t *pgc)
{
	pgc_init(pgc);

	pgc->str[PGC_STR_NAME]				      = STRH("test");
	pgc->str[PGC_STR_OUTDIR]			      = STRH("$(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/");
	pgc->intdir[PGC_INTDIR_STR_INTDIR][PGC_INTDIR_OBJECT] = STRH("$(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/int/");
	pgc->target[PGC_TARGET_STR_TARGET][PGC_BUILD_EXE]     = STRH("$(OUTDIR)test");

	pgc->builds = F_PGC_BUILD_EXE;

	pgc_add_arch(pgc, STRH("x86_64"));
	pgc_add_src(pgc, STRH("src/"), F_PGC_SRC_C);
	pgc_add_depend(pgc, STRH("dep"));
}

void pgc_gen_coverage_exe(pgc_t *pgc)
{
	pgc_init(pgc);

	pgc->str[PGC_STR_NAME]				      = STRH("test");
	pgc->str[PGC_STR_OUTDIR]			      = STRH("$(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/");
	pgc->intdir[PGC_INTDIR_STR_INTDIR][PGC_INTDIR_OBJECT] = STRH("$(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/int/");
	pgc->target[PGC_TARGET_STR_TARGET][PGC_BUILD_EXE]     = STRH("$(OUTDIR)test");
	pgc->str[PGC_STR_COVDIR]			      = STRH("$(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/cov/");

	pgc->builds = F_PGC_BUILD_EXE;

	pgc_add_arch(pgc, STRH("x86_64"));
	pgc_add_src(pgc, STRH("src/"), F_PGC_SRC_C);
}

void pgc_gen_coverage_static(pgc_t *pgc)
{
	pgc_init(pgc);

	pgc->str[PGC_STR_NAME]				      = STRH("test");
	pgc->str[PGC_STR_OUTDIR]			      = STRH("$(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/");
	pgc->intdir[PGC_INTDIR_STR_INTDIR][PGC_INTDIR_STATIC] = STRH("$(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/int/");
	pgc->target[PGC_TARGET_STR_TARGET][PGC_BUILD_STATIC]  = STRH("$(OUTDIR)test.a");
	pgc->str[PGC_STR_COVDIR]			      = STRH("$(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/cov/");

	pgc->builds = F_PGC_BUILD_STATIC;

	pgc_add_arch(pgc, STRH("x86_64"));
	pgc_add_src(pgc, STRH("src/"), F_PGC_SRC_C);
}

void pgc_gen_coverage_shared(pgc_t *pgc)
{
	pgc_init(pgc);

	pgc->str[PGC_STR_NAME]				      = STRH("test");
	pgc->str[PGC_STR_OUTDIR]			      = STRH("$(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/");
	pgc->intdir[PGC_INTDIR_STR_INTDIR][PGC_INTDIR_SHARED] = STRH("$(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/int/");
	pgc->target[PGC_TARGET_STR_TARGET][PGC_BUILD_SHARED]  = STRH("$(OUTDIR)test.so");
	pgc->str[PGC_STR_COVDIR]			      = STRH("$(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/cov/");

	pgc->builds = F_PGC_BUILD_SHARED;

	pgc_add_arch(pgc, STRH("x86_64"));
	pgc_add_src(pgc, STRH("src/"), F_PGC_SRC_C);
}

void pgc_gen_defines_exe(pgc_t *pgc)
{
	pgc_init(pgc);

	pgc->str[PGC_STR_NAME]				      = STRH("test");
	pgc->str[PGC_STR_OUTDIR]			      = STRH("$(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/");
	pgc->intdir[PGC_INTDIR_STR_INTDIR][PGC_INTDIR_OBJECT] = STRH("$(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/int/");
	pgc->target[PGC_TARGET_STR_TARGET][PGC_BUILD_EXE]     = STRH("$(OUTDIR)test");

	pgc->builds = F_PGC_BUILD_EXE;

	pgc_add_arch(pgc, STRH("x86_64"));
	pgc_add_src(pgc, STRH("src/"), F_PGC_SRC_C);
	pgc_add_define(pgc, STR("DEBUG"), F_PGC_INTDIR_OBJECT);
	pgc_add_define(pgc, STR("VERSION=1"), F_PGC_INTDIR_OBJECT);
}

void pgc_gen_defines_static(pgc_t *pgc)
{
	pgc_init(pgc);

	pgc->str[PGC_STR_NAME]				      = STRH("test");
	pgc->str[PGC_STR_OUTDIR]			      = STRH("$(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/");
	pgc->intdir[PGC_INTDIR_STR_INTDIR][PGC_INTDIR_STATIC] = STRH("$(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/int/");
	pgc->target[PGC_TARGET_STR_TARGET][PGC_BUILD_STATIC]  = STRH("$(OUTDIR)test.a");

	pgc->builds = F_PGC_BUILD_STATIC;

	pgc_add_arch(pgc, STRH("x86_64"));
	pgc_add_src(pgc, STRH("src/"), F_PGC_SRC_C);
	pgc_add_define(pgc, STR("DEBUG"), F_PGC_INTDIR_STATIC);
	pgc_add_define(pgc, STR("VERSION=1"), F_PGC_INTDIR_STATIC);
}

void pgc_gen_defines_shared(pgc_t *pgc)
{
	pgc_init(pgc);

	pgc->str[PGC_STR_NAME]				      = STRH("test");
	pgc->str[PGC_STR_OUTDIR]			      = STRH("$(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/");
	pgc->intdir[PGC_INTDIR_STR_INTDIR][PGC_INTDIR_SHARED] = STRH("$(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/int/");
	pgc->target[PGC_TARGET_STR_TARGET][PGC_BUILD_SHARED]  = STRH("$(OUTDIR)test.so");

	pgc->builds = F_PGC_BUILD_SHARED;

	pgc_add_arch(pgc, STRH("x86_64"));
	pgc_add_src(pgc, STRH("src/"), F_PGC_SRC_C);
	pgc_add_define(pgc, STR("DEBUG"), F_PGC_INTDIR_SHARED);
	pgc_add_define(pgc, STR("VERSION=1"), F_PGC_INTDIR_SHARED);
}

void pgc_gen_copyfiles(pgc_t *pgc)
{
	pgc_init(pgc);

	pgc->str[PGC_STR_NAME] = STRH("test");

	pgc->builds = F_PGC_BUILD_STATIC;

	pgc_add_copyfile(pgc, STRH("lib.so"));
}

void pgc_gen_run(pgc_t *pgc)
{
	pgc_init(pgc);

	pgc->str[PGC_STR_NAME]				      = STRH("test");
	pgc->str[PGC_STR_OUTDIR]			      = STRH("$(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/");
	pgc->intdir[PGC_INTDIR_STR_INTDIR][PGC_INTDIR_OBJECT] = STRH("$(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/int/");
	pgc->target[PGC_TARGET_STR_TARGET][PGC_BUILD_EXE]     = STRH("$(OUTDIR)test");

	pgc->builds = F_PGC_BUILD_EXE;

	pgc_add_arch(pgc, STRH("x86_64"));
	pgc_add_config(pgc, STRH("Release"));
	pgc_add_src(pgc, STRH("src/"), F_PGC_SRC_C);
	pgc_set_run(pgc, STRH("run"), F_PGC_BUILD_EXE);
}

void pgc_gen_run_debug(pgc_t *pgc)
{
	pgc_init(pgc);

	pgc->str[PGC_STR_NAME]				      = STRH("test");
	pgc->str[PGC_STR_OUTDIR]			      = STRH("$(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/");
	pgc->intdir[PGC_INTDIR_STR_INTDIR][PGC_INTDIR_OBJECT] = STRH("$(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/int/");
	pgc->target[PGC_TARGET_STR_TARGET][PGC_BUILD_EXE]     = STRH("$(OUTDIR)test");

	pgc->builds = F_PGC_BUILD_EXE;

	pgc_add_arch(pgc, STRH("x86_64"));
	pgc_add_config(pgc, STRH("Debug"));
	pgc_add_src(pgc, STRH("src/"), F_PGC_SRC_C);
	pgc_set_run_debug(pgc, STRH("run_debug"), F_PGC_BUILD_EXE);
}

void pgc_gen_run_run_debug(pgc_t *pgc)
{
	pgc_init(pgc);

	pgc->str[PGC_STR_NAME]				      = STRH("test");
	pgc->str[PGC_STR_OUTDIR]			      = STRH("$(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/");
	pgc->intdir[PGC_INTDIR_STR_INTDIR][PGC_INTDIR_OBJECT] = STRH("$(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/int/");
	pgc->target[PGC_TARGET_STR_TARGET][PGC_BUILD_EXE]     = STRH("$(OUTDIR)test");

	pgc->builds = F_PGC_BUILD_EXE;

	pgc_add_arch(pgc, STRH("x86_64"));
	pgc_add_config(pgc, STRH("Debug"));
	pgc_add_src(pgc, STRH("src/"), F_PGC_SRC_C);
	pgc_set_run(pgc, STRH("run"), F_PGC_BUILD_EXE);
	pgc_set_run_debug(pgc, STRH("run_debug"), F_PGC_BUILD_EXE);
}

void pgc_gen_artifact_exe(pgc_t *pgc)
{
	pgc_init(pgc);

	pgc->str[PGC_STR_NAME]				      = STRH("test");
	pgc->str[PGC_STR_OUTDIR]			      = STRH("$(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/");
	pgc->intdir[PGC_INTDIR_STR_INTDIR][PGC_INTDIR_OBJECT] = STRH("$(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/int/");
	pgc->target[PGC_TARGET_STR_TARGET][PGC_BUILD_EXE]     = STRH("$(OUTDIR)test");
	pgc->target[PGC_TARGET_STR_ARTIFACT][PGC_BUILD_EXE]   = STRH("test");

	pgc->builds = F_PGC_BUILD_EXE;

	pgc_add_arch(pgc, STRH("x86_64"));
	pgc_add_src(pgc, STRH("src/"), F_PGC_SRC_C);
}

void pgc_gen_artifact_lib(pgc_t *pgc)
{
	pgc_init(pgc);

	pgc->str[PGC_STR_NAME]				       = STRH("test");
	pgc->str[PGC_STR_OUTDIR]			       = STRH("$(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/");
	pgc->intdir[PGC_INTDIR_STR_INTDIR][PGC_INTDIR_STATIC]  = STRH("$(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/int/");
	pgc->target[PGC_TARGET_STR_TARGET][PGC_BUILD_STATIC]   = STRH("$(OUTDIR)test.a");
	pgc->target[PGC_TARGET_STR_ARTIFACT][PGC_BUILD_STATIC] = STRH("test");

	pgc->builds = F_PGC_BUILD_STATIC;

	pgc_add_arch(pgc, STRH("x86_64"));
	pgc_add_src(pgc, STRH("src/"), F_PGC_SRC_C);
}

void pgc_gen_bin_obj(pgc_t *pgc)
{
	pgc_init(pgc);

	pgc->str[PGC_STR_NAME]				      = STRH("test");
	pgc->str[PGC_STR_OUTDIR]			      = STRH("$(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/");
	pgc->intdir[PGC_INTDIR_STR_INTDIR][PGC_INTDIR_OBJECT] = STRH("$(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/int/");
	pgc->target[PGC_TARGET_STR_TARGET][PGC_BUILD_BIN]     = STRH("$(OUTDIR)test.bin");

	pgc->builds = F_PGC_BUILD_BIN;

	pgc_add_arch(pgc, STRH("x86_64"));
	pgc_add_src(pgc, STRH("src/"), F_PGC_SRC_C);
}

void pgc_gen_bin_files(pgc_t *pgc)
{
	pgc_init(pgc);

	pgc->str[PGC_STR_NAME]				  = STRH("test");
	pgc->str[PGC_STR_OUTDIR]			  = STRH("$(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/");
	pgc->target[PGC_TARGET_STR_TARGET][PGC_BUILD_BIN] = STRH("$(OUTDIR)test.bin");

	pgc->builds = F_PGC_BUILD_BIN;

	pgc_add_file(pgc, STRH("file.bin"), PGC_FILE_BIN);
	pgc_add_file(pgc, STRH("file.elf"), PGC_FILE_ELF);

	pgc->str[PGC_STR_SIZE] = STRH("1024");
}

void pgc_gen_bin_run(pgc_t *pgc)
{
	pgc_init(pgc);

	pgc->str[PGC_STR_NAME]				      = STRH("test");
	pgc->str[PGC_STR_OUTDIR]			      = STRH("$(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/");
	pgc->intdir[PGC_INTDIR_STR_INTDIR][PGC_INTDIR_OBJECT] = STRH("$(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/int/");
	pgc->target[PGC_TARGET_STR_TARGET][PGC_BUILD_BIN]     = STRH("$(OUTDIR)test.bin");

	pgc->builds = F_PGC_BUILD_BIN;

	pgc_add_arch(pgc, STRH("x86_64"));
	pgc_add_config(pgc, STRH("Debug"));
	pgc_add_src(pgc, STRH("src/"), F_PGC_SRC_C);
	pgc_set_run(pgc, STRH("run"), F_PGC_BUILD_BIN);
}

void pgc_gen_elf(pgc_t *pgc)
{
	pgc_init(pgc);

	pgc->str[PGC_STR_NAME]				      = STRH("test");
	pgc->str[PGC_STR_OUTDIR]			      = STRH("$(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/");
	pgc->intdir[PGC_INTDIR_STR_INTDIR][PGC_INTDIR_OBJECT] = STRH("$(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/int/");
	pgc->target[PGC_TARGET_STR_TARGET][PGC_BUILD_ELF]     = STRH("$(OUTDIR)test.elf");

	pgc->builds = F_PGC_BUILD_ELF;

	pgc_add_arch(pgc, STRH("x86_64"));
	pgc_add_src(pgc, STRH("src/"), F_PGC_SRC_C);
}

void pgc_gen_fat12(pgc_t *pgc)
{
	pgc_init(pgc);

	pgc->str[PGC_STR_NAME]				    = STRH("test");
	pgc->str[PGC_STR_OUTDIR]			    = STRH("$(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/");
	pgc->target[PGC_TARGET_STR_TARGET][PGC_BUILD_FAT12] = STRH("$(OUTDIR)test.img");

	pgc->builds = F_PGC_BUILD_FAT12;

	pgc_add_file(pgc, STRH("file.bin"), PGC_FILE_BIN);
	pgc_add_file(pgc, STRH("file.elf"), PGC_FILE_ELF);

	pgc->str[PGC_STR_SIZE] = STRH("1024");
}

void pgc_gen_fat12_header(pgc_t *pgc)
{
	pgc_init(pgc);

	pgc->str[PGC_STR_NAME]				    = STRH("test");
	pgc->str[PGC_STR_OUTDIR]			    = STRH("$(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/");
	pgc->target[PGC_TARGET_STR_TARGET][PGC_BUILD_FAT12] = STRH("$(OUTDIR)test.img");
	pgc->str[PGC_STR_HEADER]			    = STRH("file.bin");
	pgc->str[PGC_STR_SIZE]				    = STRH("1024");

	pgc->builds = F_PGC_BUILD_FAT12;

	pgc_add_file(pgc, STRH("file.elf"), PGC_FILE_ELF);
}

void pgc_gen_archs(pgc_t *pgc)
{
	pgc_init(pgc);

	pgc->str[PGC_STR_NAME]				      = STRH("test");
	pgc->str[PGC_STR_OUTDIR]			      = STRH("$(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/");
	pgc->intdir[PGC_INTDIR_STR_INTDIR][PGC_INTDIR_OBJECT] = STRH("$(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/int/");
	pgc->target[PGC_TARGET_STR_TARGET][PGC_BUILD_EXE]     = STRH("$(OUTDIR)test");

	pgc->builds = F_PGC_BUILD_EXE;

	pgc_add_arch(pgc, STRH("x86_64"));
	pgc_add_arch(pgc, STRH("i386"));
	pgc_add_config(pgc, STRH("Debug"));
	pgc_add_src(pgc, STRH("src/"), F_PGC_SRC_C);
}

void pgc_gen_configs(pgc_t *pgc)
{
	pgc_init(pgc);

	pgc->str[PGC_STR_NAME]				      = STRH("test");
	pgc->str[PGC_STR_OUTDIR]			      = STRH("$(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/");
	pgc->intdir[PGC_INTDIR_STR_INTDIR][PGC_INTDIR_OBJECT] = STRH("$(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/int/");
	pgc->target[PGC_TARGET_STR_TARGET][PGC_BUILD_EXE]     = STRH("$(OUTDIR)test");

	pgc->builds = F_PGC_BUILD_EXE;

	pgc_add_arch(pgc, STRH("x86_64"));
	pgc_add_config(pgc, STRH("Debug"));
	pgc_add_config(pgc, STRH("Release"));
	pgc_add_src(pgc, STRH("src/"), F_PGC_SRC_C);
}

void pgc_gen_nasm_bin(pgc_t *pgc)
{
	pgc_init(pgc);

	pgc->str[PGC_STR_NAME]				      = STRH("test");
	pgc->str[PGC_STR_OUTDIR]			      = STRH("$(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/");
	pgc->intdir[PGC_INTDIR_STR_INTDIR][PGC_INTDIR_OBJECT] = STRH("$(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/int/");
	pgc->target[PGC_TARGET_STR_TARGET][PGC_BUILD_BIN]     = STRH("$(OUTDIR)test.bin");

	pgc->builds = F_PGC_BUILD_BIN;

	pgc_add_arch(pgc, STRH("x86_64"));
	pgc_add_config(pgc, STRH("Debug"));
	pgc_add_src(pgc, STRH("src/"), F_PGC_SRC_NASM);
}

void pgc_gen_nasm_exe(pgc_t *pgc)
{
	pgc_init(pgc);

	pgc->str[PGC_STR_NAME]				      = STRH("test");
	pgc->str[PGC_STR_OUTDIR]			      = STRH("$(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/");
	pgc->intdir[PGC_INTDIR_STR_INTDIR][PGC_INTDIR_OBJECT] = STRH("$(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/int/");
	pgc->target[PGC_TARGET_STR_TARGET][PGC_BUILD_EXE]     = STRH("$(OUTDIR)test");

	pgc->builds = F_PGC_BUILD_EXE;

	pgc_add_arch(pgc, STRH("x86_64"));
	pgc_add_config(pgc, STRH("Debug"));
	pgc_add_src(pgc, STRH("src/"), F_PGC_SRC_NASM);
}

void pgc_gen_nasm_static(pgc_t *pgc)
{
	pgc_init(pgc);

	pgc->str[PGC_STR_NAME]				      = STRH("test");
	pgc->str[PGC_STR_OUTDIR]			      = STRH("$(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/");
	pgc->intdir[PGC_INTDIR_STR_INTDIR][PGC_INTDIR_STATIC] = STRH("$(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/int/");
	pgc->target[PGC_TARGET_STR_TARGET][PGC_BUILD_STATIC]  = STRH("$(OUTDIR)test.a");

	pgc->builds = F_PGC_BUILD_STATIC;

	pgc_add_arch(pgc, STRH("x86_64"));
	pgc_add_src(pgc, STRH("src/"), F_PGC_SRC_NASM);
}

void pgc_gen_nasm_shared(pgc_t *pgc)
{
	pgc_init(pgc);

	pgc->str[PGC_STR_NAME]				      = STRH("test");
	pgc->str[PGC_STR_OUTDIR]			      = STRH("$(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/");
	pgc->intdir[PGC_INTDIR_STR_INTDIR][PGC_INTDIR_SHARED] = STRH("$(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/int/");
	pgc->target[PGC_TARGET_STR_TARGET][PGC_BUILD_SHARED]  = STRH("$(OUTDIR)test.so");

	pgc->builds = F_PGC_BUILD_SHARED;

	pgc_add_arch(pgc, STRH("x86_64"));
	pgc_add_src(pgc, STRH("src/"), F_PGC_SRC_NASM);
}

void pgc_gen_asm_exe(pgc_t *pgc)
{
	pgc_init(pgc);

	pgc->str[PGC_STR_NAME]				      = STRH("test");
	pgc->str[PGC_STR_OUTDIR]			      = STRH("$(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/");
	pgc->intdir[PGC_INTDIR_STR_INTDIR][PGC_INTDIR_OBJECT] = STRH("$(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/int/");
	pgc->target[PGC_TARGET_STR_TARGET][PGC_BUILD_EXE]     = STRH("$(OUTDIR)test");

	pgc->builds = F_PGC_BUILD_EXE;

	pgc_add_arch(pgc, STRH("x86_64"));
	pgc_add_src(pgc, STRH("src/"), F_PGC_SRC_S);
}

void pgc_gen_asm_static(pgc_t *pgc)
{
	pgc_init(pgc);

	pgc->str[PGC_STR_NAME]				      = STRH("test");
	pgc->str[PGC_STR_OUTDIR]			      = STRH("$(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/");
	pgc->intdir[PGC_INTDIR_STR_INTDIR][PGC_INTDIR_STATIC] = STRH("$(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/int/");
	pgc->target[PGC_TARGET_STR_TARGET][PGC_BUILD_STATIC]  = STRH("$(OUTDIR)test.a");

	pgc->builds = F_PGC_BUILD_STATIC;

	pgc_add_arch(pgc, STRH("x86_64"));
	pgc_add_src(pgc, STRH("src/"), F_PGC_SRC_S);
}

void pgc_gen_asm_shared(pgc_t *pgc)
{
	pgc_init(pgc);

	pgc->str[PGC_STR_NAME]				      = STRH("test");
	pgc->str[PGC_STR_OUTDIR]			      = STRH("$(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/");
	pgc->intdir[PGC_INTDIR_STR_INTDIR][PGC_INTDIR_SHARED] = STRH("$(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/int/");
	pgc->target[PGC_TARGET_STR_TARGET][PGC_BUILD_SHARED]  = STRH("$(OUTDIR)test.so");

	pgc->builds = F_PGC_BUILD_SHARED;

	pgc_add_arch(pgc, STRH("x86_64"));
	pgc_add_src(pgc, STRH("src/"), F_PGC_SRC_S);
}

void pgc_gen_c_exe(pgc_t *pgc)
{
	pgc_init(pgc);

	pgc->str[PGC_STR_NAME]				      = STRH("test");
	pgc->str[PGC_STR_OUTDIR]			      = STRH("$(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/");
	pgc->intdir[PGC_INTDIR_STR_INTDIR][PGC_INTDIR_OBJECT] = STRH("$(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/int/");
	pgc->target[PGC_TARGET_STR_TARGET][PGC_BUILD_EXE]     = STRH("$(OUTDIR)test");

	pgc->builds = F_PGC_BUILD_EXE;

	pgc_add_arch(pgc, STRH("x86_64"));
	pgc_add_src(pgc, STRH("src/"), F_PGC_SRC_C);
}

void pgc_gen_c_static(pgc_t *pgc)
{
	pgc_init(pgc);

	pgc->str[PGC_STR_NAME]				      = STRH("test");
	pgc->str[PGC_STR_OUTDIR]			      = STRH("$(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/");
	pgc->intdir[PGC_INTDIR_STR_INTDIR][PGC_INTDIR_STATIC] = STRH("$(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/int/");
	pgc->target[PGC_TARGET_STR_TARGET][PGC_BUILD_STATIC]  = STRH("$(OUTDIR)test.a");

	pgc->builds = F_PGC_BUILD_STATIC;

	pgc_add_arch(pgc, STRH("x86_64"));
	pgc_add_config(pgc, STRH("Debug"));
	pgc_add_src(pgc, STRH("src/"), F_PGC_SRC_C);
}

void pgc_gen_c_shared(pgc_t *pgc)
{
	pgc_init(pgc);

	pgc->str[PGC_STR_NAME]				      = STRH("test");
	pgc->str[PGC_STR_OUTDIR]			      = STRH("$(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/");
	pgc->intdir[PGC_INTDIR_STR_INTDIR][PGC_INTDIR_SHARED] = STRH("$(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/int/");
	pgc->target[PGC_TARGET_STR_TARGET][PGC_BUILD_SHARED]  = STRH("$(OUTDIR)test.so");

	pgc->builds = F_PGC_BUILD_SHARED;

	pgc_add_arch(pgc, STRH("x86_64"));
	pgc_add_config(pgc, STRH("Debug"));
	pgc_add_src(pgc, STRH("src/"), F_PGC_SRC_C);
}

void pgc_gen_cpp_exe(pgc_t *pgc)
{
	pgc_init(pgc);

	pgc->str[PGC_STR_NAME]				      = STRH("test");
	pgc->str[PGC_STR_OUTDIR]			      = STRH("$(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/");
	pgc->intdir[PGC_INTDIR_STR_INTDIR][PGC_INTDIR_OBJECT] = STRH("$(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/int/");
	pgc->target[PGC_TARGET_STR_TARGET][PGC_BUILD_EXE]     = STRH("$(OUTDIR)test");

	pgc->builds = F_PGC_BUILD_EXE;

	pgc_add_arch(pgc, STRH("x86_64"));
	pgc_add_src(pgc, STRH("src/"), F_PGC_SRC_CPP);
}

void pgc_gen_cpp_static(pgc_t *pgc)
{
	pgc_init(pgc);

	pgc->str[PGC_STR_NAME]				      = STRH("test");
	pgc->str[PGC_STR_OUTDIR]			      = STRH("$(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/");
	pgc->intdir[PGC_INTDIR_STR_INTDIR][PGC_INTDIR_STATIC] = STRH("$(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/int/");
	pgc->target[PGC_TARGET_STR_TARGET][PGC_BUILD_STATIC]  = STRH("$(OUTDIR)test.a");

	pgc->builds = F_PGC_BUILD_STATIC;

	pgc_add_arch(pgc, STRH("x86_64"));
	pgc_add_src(pgc, STRH("src/"), F_PGC_SRC_CPP);
}

void pgc_gen_cpp_shared(pgc_t *pgc)
{
	pgc_init(pgc);

	pgc->str[PGC_STR_NAME]				      = STRH("test");
	pgc->str[PGC_STR_OUTDIR]			      = STRH("$(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/");
	pgc->intdir[PGC_INTDIR_STR_INTDIR][PGC_INTDIR_SHARED] = STRH("$(SLNDIR)bin/$(CONFIG)-$(ARCH)/test/int/");
	pgc->target[PGC_TARGET_STR_TARGET][PGC_BUILD_SHARED]  = STRH("$(OUTDIR)test.so");

	pgc->builds = F_PGC_BUILD_SHARED;

	pgc_add_arch(pgc, STRH("x86_64"));
	pgc_add_src(pgc, STRH("src/"), F_PGC_SRC_CPP);
}

void pgc_gen_url(pgc_t *pgc)
{
	pgc_init(pgc);

	pgc->str[PGC_STR_NAME]	  = STRH("gcc-13.1.0");
	pgc->str[PGC_STR_URL]	  = STRH("http://ftp.gnu.org/gnu/gcc/gcc-13.1.0/");
	pgc->str[PGC_STR_FORMAT]  = STRH("tar.gz");
	pgc->str[PGC_STR_CONFIG]  = STRH("--disable-nls");
	pgc->str[PGC_STR_TARGETS] = STRH("all-gcc");
	pgc->str[PGC_STR_OUTDIR]  = STRH("$(SLNDIR)bin/$(ARCH)/test/");

	pgc_add_require(pgc, STRH("g++"));
}
