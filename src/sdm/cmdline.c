#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "options.h"

extern char *which(const char *);

/**
 * @brief Free memory associated with command line options
 * @param [in] opts The struct containing the elements to free
 */
void release_opts(options *opts)
{
	if (!opts)
		return;
	free(opts->cmd);
	free(opts->path);
	bzero(&opts, sizeof(opts));
}

static int help_fl=0, start_fl=0, stop_fl=0;
/**
 * @brief Parse the command line
 * @param [out] opts The options structure to fill
 * @param [in] argc Argument count
 * @param [in] argv[] Argument vector
 * @return 0 on success; -1 on error
 */
int parse_cmdline(options *opts, int argc, char * const argv[])
{
	const char *cmd = NULL;
	static struct option options[] =
	{
		{ "help", no_argument, &help_fl, 1 },
		{ "start", no_argument, &start_fl, (1<<0) },
		{ "stop", no_argument, &stop_fl, (1<<1) },
		{ 0, 0, 0, 0 }
	};
	int rv = -1;
	if (argc == 1)
		goto out;
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
		cmd = argv[optind++];
		if (strstr(cmd, "/") == NULL) { /* passed cmd, need path */
			opts->cmd = strdup(cmd);
			if (!opts->cmd) {
				fprintf(stderr, "%s: malloc error\n", argv[0]);
				goto out;
			}
			opts->path = which(cmd);
			if (!opts->path) {
				fprintf(stderr, "%s: %s not found\n", argv[0], opts->cmd);
				goto out;
			}
		} else { /* passed path, need cmd */
			opts->path = strdup(cmd);
			if (!opts->path) {
				fprintf(stderr, "%s: malloc error\n", argv[0]);
				goto out;
			}
			opts->cmd = strrchr(cmd, '/');
			if (!opts->cmd) {
				fprintf(stderr, "%s: insanity\n", argv[0]);
				goto out;
			}
			opts->cmd = strdup(opts->cmd + 1);
			if (!opts->cmd) {
				fprintf(stderr, "%s: malloc error\n", argv[0]);
				goto out;
			}
		}
	}
	if (!help_fl && !opts->cmd)
		goto out;
	opts->help = help_fl;
	opts->start = start_fl;
	opts->stop = stop_fl;
	rv = 0;
out:
	return rv;
}
