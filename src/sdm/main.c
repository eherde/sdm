#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>

#include "options.h"

extern int parse_cmdline(options *, int, char *[]);
extern void release_opts(options *opts);

extern int start(void);
extern int stop(void);
extern int status(void);

extern void free_paths(void);

char **g_argv;
options opts;

/**
 * @brief Print a usage string
 * @param [in] fp File stream to print to
 * @param [in] name Name of this executable
 */
void usage(FILE *fp, const char * const name)
{
	const char * const ufmt = "Usage: %s [OPTION]... COMMAND\n"
		"OPTION:\n"
		"\t--help     display this message\n"
		"\t--start    start the service\n"
		"\t--stop     stop the service\n";
	fprintf(fp, ufmt, name);
}

int main(int argc, char *argv[])
{
	int rv = EXIT_FAILURE;
	g_argv = argv;
	bzero(&opts, sizeof(opts));
	if (parse_cmdline(&opts, argc, argv) != 0) {
		rv = 2;
		usage(stderr, argv[0]);
		goto out;
	}
	if (opts.help) {
		usage(stdout, argv[0]);
	} else if (opts.status) {
		rv = status();
	} else if (opts.start) {
		rv = start();
	} else if (opts.stop) {
		rv = stop();
	} else { /* no actions given */
		rv = 2;
		usage(stderr, argv[0]);
		goto out;
	}
out:
	release_opts(&opts);
	free_paths();
	return rv;
}
