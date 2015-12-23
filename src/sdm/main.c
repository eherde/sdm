#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>

#include "options.h"

extern char *which(const char *);
extern int parse_cmdline(int, char *[], options *);
extern int start(void);
extern int stop(void);

void usage(FILE *fp, const char * const name)
{
	const char * const ufmt = "Usage: %s [OPTION]... ACTION\n"
		"ACTION:\n"
		"\tstart - start the service\n"
		"\tstop  - stop the service\n";
	fprintf(fp, ufmt, name);
}

int main(int argc, char *argv[])
{
	options opts;
	bzero(&opts, sizeof(opts));
	int rv = EXIT_FAILURE;
	char *path = NULL;
	if (parse_cmdline(argc, argv, &opts) != 0) {
		rv = 2;
		usage(stderr, argv[0]);
		goto out;
	}
	if (strstr(opts.cmd, "/") == NULL) {
		path = which(opts.cmd);
		if (!path) {
			fprintf(stderr, "%s: %s not found", argv[0], opts.cmd);
		}
	} else {
		path = strdup(opts.cmd);
	}
	if (access(path, X_OK) != 0) {
		fprintf(stderr, "%s: %s: %s", argv[0], strerror(errno), path);
		goto out;
	}
	if (opts.start) {
		start();
	} else if (opts.stop) {
		stop();
	}
	rv = 0;
out:
	return rv;
}
