#define _GNU_SOURCE
#include <errno.h>
#include <dirent.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <ftw.h>

#include "options.h"

extern options opts;
extern char **g_argv;

extern char *g_pidfile;
extern char *g_root;

extern char *set_root(const char *cmd);
extern char *set_pidfile(const char *root, const char *cmd);
extern pid_t read_pidfile(const char *path);

/**
 * @brief Check if we will be able to write to the pidfile by checking
 * permissions on the containing directory.
 * @param [in] path Path to the pidfile
 * @return 0 on OK; -1 on not OK
 */
int check_pidfile(const char *path)
{
	int rv = -1;
	char *dup = NULL;
	char *end = NULL;

	dup = strdup(path);
	if (!dup)
		goto out;
	end = strrchr(dup, '/');
	if (!end)
		goto out;
	end[0] =  '\0';
	if (access(dup, W_OK) != 0)
		goto out;
	rv = 0;
out:
	free(dup);
	dup = end = NULL;
	return rv;
}

/**
 * @brief Write our process id to the pidfile
 * @param [in] path Path to the pidfile
 * @return 0 on success; -1 on failure
 */
int write_pidfile(const char *path)
{
	int rv = -1;
	FILE *fp = NULL;
	fp = fopen(path, "w");
	if (!fp)
		goto out;
	fprintf(fp, "%d", getpid());
	rv = 0;
out:
	if (fp) fclose(fp);
	fp = NULL;
	return rv;
}

#if 0
int closer(const char *path, const struct stat *sb, int typeflag,
	struct FTW *ftwbuf)
{
	char *name = strrchr(path, '/') + 1;
	int fd = -1;
	if (typeflag == FTW_D) {
		return FTW_CONTINUE;
	}
	fd = atoi(name);
	switch (fd) {
	case 0:
	case 1:
	case 2:
		break;
	default:
		close(fd);
	}
	return FTW_CONTINUE;
}
#endif

/**
 * @brief Close all file descriptors except 0, 1 and 2
 * @return 0 on success; -1 on failure
 */
int close_nonstd_fds(void)
{
	const int procpidfdmax = 32;
	int rv = -1;
	int fd = -1;
	char *cmp = NULL;
	char *sym = NULL;
	char phys[procpidfdmax];
	DIR *dir = NULL;
	struct dirent *ent = NULL;
	dir = opendir("/proc/self/fd");
	if (!dir) {
		goto out;
	}
	while ((ent = readdir(dir)) != NULL) {
		free(cmp);
		free(sym);
		/* ignore . and .. */
		if (strcmp(ent->d_name, ".") == 0)
			continue;
		if (strcmp(ent->d_name, "..") == 0)
			continue;
		/* check if the fd is the fd associated with opendir -
		 * we will close this one last */
		if (asprintf(&sym, "/proc/self/fd/%s", ent->d_name) == -1)
			goto out;
		if (asprintf(&cmp, "/proc/%d/fd", getpid()) == -1)
			goto out;
		bzero(phys, sizeof(phys));
		if (readlink(sym, phys, procpidfdmax-1) == -1)
			goto out;
		if (strcmp(cmp, phys) == 0)
			continue;
		fd = atoi(ent->d_name);
		switch (fd) {
		case 0:
		case 1:
		case 2:
			break;
		default:
			close(fd);
		}
	}
	rv = 0;
out:
	closedir(dir);
	dir = NULL;
	ent = NULL;
	free(cmp);
	free(sym);
	cmp = sym = NULL;
	return rv;
}

/**
 * @brief Reset all signal handlers to SIG_DFL
 */
void reset_sighandlers(void)
{
	int i = 0;
	struct sigaction act;
	bzero(&act, sizeof(act));
	act.sa_handler = SIG_DFL;
	for (i = 1; i < _NSIG; i++) {
		sigaction(i, &act, NULL);
	}
}

/**
 * @brief Reset the signal mask
 */
void reset_sigmask(void)
{
	sigset_t set;
	sigemptyset(&set);
	sigprocmask(SIG_SETMASK, &set, NULL);
}

/**
 * @brief Daemonize the command using double fork
 * and wait for the grandchild to execute.
 * @return 0 on success; -1 on failure
 */
int daemonize(void)
{
	int ret = 0;
	int rv = -1;
	int status;
	int fd[2];
	pid_t pid = 0;
	if (pipe2(fd, O_CLOEXEC) != 0) {
		goto out;
	}
	pid = fork();
	if (pid == 0) { /* child */
		close(fd[0]);
		setsid();
		pid = fork();
		if (pid == 0) { /* child */
			close(0);
			open("/dev/null", O_WRONLY);
			umask(0);
			ret = chdir(g_root);
			if (ret != 0) {
				write(fd[1], &ret, sizeof(int));
				close(fd[1]);
				exit(EXIT_FAILURE);
			}
			ret = write_pidfile(g_pidfile);
			if (ret != 0) {
				write(fd[1], &ret, sizeof(int));
				close(fd[1]);
				exit(EXIT_FAILURE);
			}
			ret = execvp(g_argv[optind-1], &g_argv[optind-1]);
			write(fd[1], &ret, sizeof(int));
			close(fd[1]);
			exit(EXIT_FAILURE);
		}
		/* parent */
		exit(0);
	}
	/* parent */
	close(fd[1]);
	waitpid(pid, &status, 0); /* wait for child to exit */
	if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
		goto out;
	}
	/* wait for grandchild to close pipe */
	read(fd[0], &ret, sizeof(int));
	close(fd[0]);
	if (ret == -1) {
		goto out;
	}
	pid = read_pidfile(g_pidfile);
	/* If a daemon is misconfigured, it can fail very quickly. Pause a moment,
	 * then see if the process is still running */
	usleep(100000); /* 0.1 seconds */
	rv = check_proc(pid);
out:
	return rv;
}

/**
 * @brief Start the command
 * @return 0 on success; -1 on error
 */
int start(void)
{
	int rv = -1;
	if (access(opts.path, X_OK) != 0) {
		fprintf(stderr, "%s: %s: %s\n", g_argv[0], strerror(errno), opts.path);
		goto out;
	}
	g_root = set_root(opts.path);
	g_pidfile = set_pidfile(g_root, opts.cmd);
	if (check_pidfile(g_pidfile) != 0) {
		fprintf(stderr, "%s: cannot write pidfile '%s': %s\n", g_argv[0], g_pidfile, strerror(errno));
		goto out;
	}
	close_nonstd_fds();
	reset_sighandlers();
	reset_sigmask();
	rv = daemonize();
out:
	return rv;
}
