#include <stdio.h>
#include <sys/types.h>

#include "options.h"

#define STATUS_OK 0
#define STATUS_DEAD_PIDFILE 1
#define STATUS_DEAD_LOCKFILE 2
#define STATUS_DEAD 3
#define STATUS_UNKNOWN 4

extern options opts;

extern char *g_pidfile;
extern char *g_root;

extern char *set_root(const char *cmd);
extern char *set_pidfile(const char *root, const char *cmd);

extern int check_proc(pid_t pid, const char *cmd, const char *path);
extern pid_t read_pidfile(const char *path);
extern pid_t lookup_pid(const char *cmd, const char *path);

int status(void)
{
	int rv = STATUS_UNKNOWN;
	pid_t pid = 0;
	g_root = set_root(opts.path);
	g_pidfile = set_pidfile(g_root, opts.cmd);
	pid = read_pidfile(g_pidfile);
	if (pid != -1) {
		if (check_proc(pid, opts.cmd, opts.path) == 0) {
			rv = STATUS_OK;
			printf("%s (pid %d) is running...\n", opts.cmd, pid);
		} else {
			rv = STATUS_DEAD_PIDFILE;
			printf("%s is stopped\n", opts.cmd);
		}
	} else {
		pid = lookup_pid(opts.cmd, opts.path);
		if (pid != -1) {
			rv = STATUS_OK;
			printf("%s (pid %d) is running...\n", opts.cmd, pid);
		} else {
			rv = STATUS_DEAD;
			printf("%s is stopped\n", opts.cmd);
		}
	}
	return rv;
}
