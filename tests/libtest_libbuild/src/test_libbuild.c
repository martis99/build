#include "cutils.h"

#include "test.h"
#include "test_cutils.h"

int G_DBG = 0;
int G_SUC = 0;
int G_INF = 0;
int G_WRN = 0;
int G_ERR = 0;
int G_MSG = 0;

STEST(t_cmake);
STEST(t_make);
STEST(t_pgc);
STEST(t_pgc_gen_cm);
STEST(t_pgc_gen_mk);
STEST(t_pgc_gen_vc_tasks);
STEST(t_pgc_gen_vc_launch);
STEST(t_proj_gen_pgc);
STEST(t_cm);
STEST(t_mk);
STEST(t_vc);
STEST(t_vs);

TEST(tests)
{
	SSTART;
	RUN(t_cmake);
	RUN(t_make);
	RUN(t_pgc);
	RUN(t_pgc_gen_cm);
	RUN(t_pgc_gen_mk);
	RUN(t_pgc_gen_vc_tasks);
	RUN(t_pgc_gen_vc_launch);
	RUN(t_proj_gen_pgc);
	//RUN(t_cm);
	//RUN(t_mk);
	//RUN(t_vc);
	//RUN(t_vs);
	SEND;
}

int test_libbuild()
{
	t_run(test_cutils, 0);
	tests();

	return 0;
}
