#include "test_libbuild.h"

#include "test.h"

#include "cutils.h"

int main(int argc, char **argv)
{
	cutils_t cutils = { 0 };
	c_init(&cutils);

	t_init(80);

	log_set_level(LOG_FATAL);

	t_run(test_libbuild, 1);

	const int ret = t_finish();

	c_free(&cutils, stdout);

	return ret;
}
