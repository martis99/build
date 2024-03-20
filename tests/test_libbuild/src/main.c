#include "test_libbuild.h"

#include "test.h"

#include "cutils.h"

int main(int argc, char **argv)
{
	(void)argc;
	(void)argv;

	cutils_t cutils = { 0 };
	c_init(&cutils);

	t_init(80);

	int level = log_set_level(LOG_FATAL);

	t_run(test_libbuild, 1);

	int ret = t_finish();

	log_set_level(level);

	ret |= c_free(&cutils, PRINT_DST_STD());

	return ret;
}
