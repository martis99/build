#include "t_cm.h"
#include "t_mk.h"
#include "t_vc.h"
#include "t_vs.h"

#include "test.h"

#include "mem.h"

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

int main(int argc, char **argv)
{
	mem_stats_t mem_stats = { 0 };
	mem_init(&mem_stats);

	t_init(80);
	tests();
	const int ret = t_finish();

	mem_print(stdout);

	return ret;
}
