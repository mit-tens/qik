#include <stdio.h>
#include <libconfig.h>
#include "cfg_t.h"

int
get_cfg_t(config_t *cfg_t, const char *path)
{
    config_init(cfg_t);

    if (! config_read_file(cfg_t, path)) {
	fprintf(stderr, "%s:%d - %s\n", config_error_file(cfg_t), config_error_line(cfg_t), config_error_text(cfg_t));

	config_destroy(cfg_t);

	return 1;
    }

    return 0;
}
