#include "t_cm.h"
#include "t_mk.h"
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
	RUN(vs);
	SEND;
}

int main(int argc, char **argv)
{
	m_stats_t m_stats = { 0 };
	m_init(&m_stats);

	t_init(80);
	tests();
	const int ret = t_finish();

	m_print(stdout);

	return ret;
}
