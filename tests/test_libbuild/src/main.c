#include "t_cm.h"
#include "t_mk.h"
#include "t_vc.h"
#include "t_vs.h"

#include "test.h"

#include "cutils.h"

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
	cutils_t cutils = { 0 };
	c_init(&cutils);

	t_init(80);
	tests();
	const int ret = t_finish();

	c_free(&cutils, stdout);

	return ret;
}
