#include <getopt.h>
#include <unistd.h>

#include "options.h"

int start_fl=0, stop_fl=0;
int parse_cmdline(int argc, char * const argv[], struct options_t *opts)
{
	static struct option options[] =
	{
		{ "start", no_argument, &start_fl, (1<<0) },
		{ "stop", no_argument, &stop_fl, (1<<1) },
		{ 0, 0, 0, 0 }
	};
	int rv = -1;
	while (1) {
		static int idx = 0;
		static int c;
		c = getopt_long(argc, argv, "", options, &idx);
		if (c == -1)
			break;
		switch(c) {
		case 0:
			continue;
		default:
			goto out;
		}
	}
	if (optind < argc) {
		opts->cmd = argv[optind++];
	}
	opts->start = start_fl;
	opts->stop = stop_fl;
	if (!opts->cmd) { /* no command given */
		goto out;
	}
	if ((opts->start | opts->stop) == 0) { /* no actions given */
		goto out;
	}
	if ((opts->start | opts->stop) != 1) { /* more than one action given */
		goto out;
	}
	rv = 0;
out:
	return rv;
}
