#include <signal.h>
#include <sys/types.h>
#include <unistd.h>

#include "options.h"

extern options opts;
extern char *g_pidfile;
extern char *g_root;

extern char *set_pidfile(const char *root, const char *cmd);
extern char *set_root(const char *cmd);

extern pid_t read_pidfile(const char *path);
extern pid_t lookup_pid(const char *cmd, const char *path);

int stop(void)
{
	int rv = -1;
	pid_t pid = 0;
	g_root = set_root(opts.path);
	g_pidfile = set_pidfile(g_root, opts.cmd);
	pid = read_pidfile(g_pidfile);
	if (pid <= 0) { /* could not find pid in pidfile, try to find it in /proc */
		pid = lookup_pid(opts.cmd, opts.path);
		if (pid <= 0)
			goto out;
	}
	if (pid > 1) {
		rv = kill(pid, SIGTERM);
	}
	unlink(g_pidfile);
out:
	return rv;
}
