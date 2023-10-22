#include "t_cm.h"
#include "t_mk.h"
#include "t_vc.h"
#include "t_vs.h"

#include "cutils.h"

#include "test.h"
#include "test_cutils.h"

int G_DBG = 0;
int G_SUC = 0;
int G_INF = 0;
int G_WRN = 0;
int G_ERR = 0;
int G_MSG = 0;

TEST(tests)
{
	SSTART;
	RUN(cm);
	RUN(mk);
	RUN(vc);
	RUN(vs);
	SEND;
}

int test_libbuild()
{
	t_run(test_cutils, 0);
	tests();

	return 0;
}
